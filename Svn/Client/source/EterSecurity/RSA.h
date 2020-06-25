

#pragma once

#include "big_int.h"
#include "DataBuffer.hpp"
#include "Secret.h"
#include "Random.h"

typedef struct
{
	BigInt modulus;
	BigInt public_exponent;
	BigInt private_exponent;
	BigInt prime1;
	BigInt prime2;
	BigInt exponent1;
	BigInt exponent2;
	BigInt coefficient;
}  RSAPrivateKey;

class RSA_Impl
{
public:
	RSA_Impl();

	static DataBuffer encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);
	static Secret decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);

	void create(Random &random, int32_t key_size_in_bits, int32_t public_exponent_value);

	void create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits, int32_t public_exponent_value);

private:
	void generate_prime(Random &random, BigInt &prime, int32_t prime_len);
	bool build_from_primes(BigInt *p, BigInt *q, BigInt *e, BigInt *d, uint32_t key_size_in_bits);

	static void rsaep(BigInt *msg, const BigInt *e, const BigInt *modulus, BigInt *cipher);
	static void rsadp(BigInt *cipher, const BigInt *d, const BigInt *modulus, BigInt *msg);

	static void pkcs1v15_encode(int32_t block_type, Random &random, const char *msg, int32_t mlen, char *emsg, int32_t emlen);

	static Secret pkcs1v15_decode(const char *emsg, int32_t emlen);

	static DataBuffer pkcs1v15_encrypt(int32_t block_type, Random &random, const char *msg, int32_t mlen, const BigInt *e, const BigInt *modulus);

	static Secret pkcs1v15_decrypt(const char *msg, int32_t mlen, const BigInt *d, const BigInt *modulus);

	RSAPrivateKey rsa_private_key;
};

class RSA
{
public:
	static void create_keypair(Random &random, Secret &out_private_exponent, DataBuffer &out_public_exponent, DataBuffer &out_modulus, int32_t key_size_in_bits = 1024, int32_t public_exponent_value = 65537);

	static DataBuffer encrypt(int32_t block_type, Random &random, const DataBuffer &in_public_exponent, const DataBuffer &in_modulus, const Secret &in_data);

	static DataBuffer encrypt(int32_t block_type, Random &random, const void *in_public_exponent, uint32_t in_public_exponent_size, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);

	static Secret decrypt(const Secret &in_private_exponent, const DataBuffer &in_modulus, const DataBuffer &in_data);

	static Secret decrypt(const Secret &in_private_exponent, const void *in_modulus, uint32_t in_modulus_size, const void *in_data, uint32_t in_data_size);
};

