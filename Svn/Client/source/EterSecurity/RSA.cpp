

#include "stdafx.h"
#include "RSA.h"
#include "Secret.h"
#include "Random.h"
#include "big_int.h"
#include "DataBuffer.hpp"
#include "Exception.h"

#include <exception>

#ifndef WIN32
#include <cstring>
#endif

RSA_Impl::RSA_Impl()
{
}

void RSA_Impl::create(Random &random, int32_t key_size_in_bits, int32_t public_exponent_value)
{

	const int32_t public_exponent_size = 4;
	uint8_t public_exponent[public_exponent_size];

	int32_t public_exponent_written = 0;
	for (int32_t cnt = 0; cnt < public_exponent_size; cnt++)
	{
		if (public_exponent_written || (public_exponent_value & 0xff000000 >> (cnt * 8)))
		{
			public_exponent[public_exponent_written] = (uint8_t)((public_exponent_value >> (3 - cnt) * 8));
			public_exponent_written++;
		}
	}

	uint32_t prime_len = key_size_in_bits / 16;

	if ((prime_len * 16) != key_size_in_bits)
		throw Exception("Key size must be a multiple of 16 bits");

	BigInt p, q, e, d;

	e.read_unsigned_octets(public_exponent, public_exponent_written);

	int32_t kiter = 0;

	const int32_t max_key_gen_attempts = 10;

	bool created_keypair = false;
	do
	{

		generate_prime(random, p, prime_len);
		generate_prime(random, q, prime_len);

		if (p.cmp(&q) < 0)
			p.exch(&q);

		if (build_from_primes(&p, &q, &e, &d, key_size_in_bits))
		{
			created_keypair = true;
			break;
		}

		kiter++;

	} while (kiter < max_key_gen_attempts);

	if (!created_keypair)
		throw Exception("Cannot create the key pair after lots of attempts");

}

bool RSA_Impl::build_from_primes(BigInt *p, BigInt *q, BigInt *e, BigInt *d, uint32_t key_size_in_bits)
{

	BigInt n, phi;
	BigInt psub1, qsub1, tmp;

	n = (*p) * (*q);

	if (n.significant_bits() != key_size_in_bits)
	{
		return false;
	}

	psub1 = (*p) - 1;
	qsub1 = (*q) - 1;

	phi = psub1 * qsub1;

	bool inverse_found = e->invmod(&phi, d);

	if (!inverse_found)
		return false;

	d->mod(&psub1, &tmp);
	rsa_private_key.exponent1 = tmp;

	d->mod(&qsub1, &tmp);
	rsa_private_key.exponent2 = tmp;

	q->invmod(p, &tmp);
	rsa_private_key.coefficient = tmp;

	rsa_private_key.modulus = n;
	rsa_private_key.private_exponent = *d;
	rsa_private_key.public_exponent = *e;
	rsa_private_key.prime1 = *p;
	rsa_private_key.prime2 = *q;

	return true;
}

void RSA_Impl::generate_prime(Random &random, BigInt &prime, int32_t prime_len)
{
	int32_t piter;

	Secret random_key(prime_len);
	uint8_t *random_bytes_ptr = random_key.get_data();
	const int32_t max_prime_gen_attempts = 10;

	for (piter = 0; piter < max_prime_gen_attempts; piter++)
	{
		random.get_random_bytes(random_bytes_ptr, prime_len);

		random_bytes_ptr[0] |= 0xC0;
		random_bytes_ptr[prime_len - 1] |= 0x01;
		prime.read_unsigned_octets(random_bytes_ptr, prime_len);

		if (prime.make_prime(prime_len * 8))
			return;
	}
	throw Exception("Cannot create a prime number");
}

void RSA_Impl::rsaep(BigInt *msg, const BigInt *e, const BigInt *modulus, BigInt *cipher)
{
	if ((msg->cmp_z() < 0) || (msg->cmp(modulus) >= 0))
	{
		throw Exception("Message is out of range of modulus");
	}

	msg->exptmod(e, modulus, cipher);

}

void RSA_Impl::rsadp(BigInt *cipher, const BigInt *d, const BigInt *modulus, BigInt *msg)
{
	if ((cipher->cmp_z() < 0) || (cipher->cmp(modulus) >= 0))
	{
		throw Exception("ciphertext is out of range of modulus");
	}

	cipher->exptmod(d, modulus, msg);
}

void RSA_Impl::pkcs1v15_encode(int32_t block_type, Random &random, const char *msg, int32_t mlen, char *emsg, int32_t emlen)
{
	if (mlen > emlen - 11)
		throw Exception("input length is out of range of output length");

	*(emsg++) = 0;
	*(emsg++) = block_type;

	int32_t length = emlen - mlen - 3;
	switch (block_type)
	{
	case 1:
		memset(emsg, 0xff, length);
		break;
	case 2:
		random.get_random_bytes_nzero((uint8_t *)emsg, length);
		break;
	default:
		throw Exception("PKCS1 v1.5 block type not supported");
	}
	emsg += length;
	*(emsg++) = 0;

	memcpy(emsg, msg, mlen);
}

Secret RSA_Impl::pkcs1v15_decode(const char *emsg, int32_t emlen)
{
	int32_t    ix, outlen;

	if (emlen < 11)
		throw Exception("message is less than minimum length");

	if (emsg[0] != 0)
		throw Exception("packet format error");

	int32_t block_type = emsg[1];
	if (!((block_type == 1) || (block_type == 2)))
	{
		throw Exception("packet format error");
	}

	for (ix = 2 + 8; ix < emlen; ix++)
		if (emsg[ix] == 0)
			break;

	if (ix == emlen)
		throw Exception("zero sperator error");

	outlen = emlen - (ix + 1);
	Secret buffer(outlen);

	if (outlen > 0)
		memmove(buffer.get_data(), emsg + (ix + 1), outlen);

	return buffer;
}

DataBuffer RSA_Impl::pkcs1v15_encrypt(int32_t block_type, Random &random, const char *msg, int32_t mlen, const BigInt *e, const BigInt *modulus)
{
	int32_t k = modulus->unsigned_octet_size();
	Secret key(k);

	char *buf = (char *)key.get_data();

	pkcs1v15_encode(block_type, random, msg, mlen, buf, k);

	BigInt mrep;

	mrep.read_unsigned_octets(key.get_data(), key.get_size());

	rsaep(&mrep, e, modulus, &mrep);

	DataBuffer buffer(mrep.unsigned_octet_size());
	mrep.to_unsigned_octets((uint8_t *)buffer.get_data(), buffer.get_size());
	return buffer;
}

Secret RSA_Impl::pkcs1v15_decrypt(const char *msg, int32_t mlen, const BigInt *d, const BigInt *modulus)
{
	int32_t     k;

	k = modulus->unsigned_octet_size();
	if (mlen != k)
		throw Exception("Invalid message length");

	BigInt  mrep;
	mrep.read_unsigned_octets((const uint8_t *)msg, mlen);

	rsadp(&mrep, d, modulus, &mrep);

	Secret key_buffer(k);
	mrep.to_unsigned_octets(key_buffer.get_data(), k);
	return pkcs1v15_decode((char *)key_buffer.get_data(), k);
}

DataBuffer RSA_Impl::encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size)
{
	BigInt exponent;
	exponent.read_unsigned_octets((const uint8_t *)in_public_exponent, in_public_exponent_size);

	BigInt modulus;
	modulus.read_unsigned_octets((const uint8_t *)in_modulus, in_modulus_size);

	return pkcs1v15_encrypt(block_type, random, (const char *)in_data, in_data_size, &exponent, &modulus);
}

Secret RSA_Impl::decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size)
{
	BigInt exponent;
	exponent.read_unsigned_octets((const uint8_t *)in_private_exponent.get_data(), in_private_exponent.get_size());

	BigInt modulus;
	modulus.read_unsigned_octets((const uint8_t *)in_modulus, in_modulus_size);

	return pkcs1v15_decrypt((const char *)in_data, in_data_size, &exponent, &modulus);
}

void RSA_Impl::create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits, int32_t public_exponent_value)
{
	create(random, key_size_in_bits, public_exponent_value);
	out_public_exponent = DataBuffer(rsa_private_key.public_exponent.unsigned_octet_size());
	rsa_private_key.public_exponent.to_unsigned_octets((uint8_t *)out_public_exponent.get_data(), out_public_exponent.get_size());

	out_private_exponent = Secret(rsa_private_key.private_exponent.unsigned_octet_size());
	rsa_private_key.private_exponent.to_unsigned_octets(out_private_exponent.get_data(), out_private_exponent.get_size());

	out_modulus = DataBuffer(rsa_private_key.modulus.unsigned_octet_size());
	rsa_private_key.modulus.to_unsigned_octets((uint8_t *)out_modulus.get_data(), out_modulus.get_size());
}

void RSA::create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits, int32_t public_exponent_value)
{
	RSA_Impl rsa_impl;
	rsa_impl.create_keypair(random, out_private_exponent, out_public_exponent, out_modulus, key_size_in_bits, public_exponent_value);
}

DataBuffer RSA::encrypt(int32_t block_type, Random &random, const DataBuffer &in_public_exponent, const DataBuffer &in_modulus, const Secret &in_data)
{
	return RSA_Impl::encrypt(block_type, random, in_public_exponent.get_data(), in_public_exponent.get_size(), in_modulus.get_data(), in_modulus.get_size(), in_data.get_data(), in_data.get_size());
}

Secret RSA::decrypt(const Secret &in_private_exponent, const DataBuffer &in_modulus, const DataBuffer &in_data)
{
	return RSA_Impl::decrypt(in_private_exponent, in_modulus.get_data(), in_modulus.get_size(), in_data.get_data(), in_data.get_size());
}

DataBuffer RSA::encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size)
{
	return RSA_Impl::encrypt(block_type, random, in_public_exponent, in_public_exponent_size, in_modulus, in_modulus_size, in_data, in_data_size);
}

Secret RSA::decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size)
{
	return RSA_Impl::decrypt(in_private_exponent, in_modulus, in_modulus_size, in_data, in_data_size);
}

