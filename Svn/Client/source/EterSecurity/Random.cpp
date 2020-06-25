

#include "stdafx.h"
#include "Random.h"
#include "Exception.h"
#include "File.h"

#ifndef WIN32
#include <cstring>
#endif

#include <exception>
#include <memory>

Random_Impl::Random_Impl(int32_t cache_size) : random_pool_size(cache_size), random_pool_free(0), random_pool(nullptr), random_bool_bits_free(0)
#ifdef WIN32
, hProvider(0)
#endif
{
	random_pool = new uint8_t[random_pool_size];
}

Random_Impl::~Random_Impl()
{
	if (random_pool)
	{
		memset(random_pool, 0, random_pool_size);
		delete[] random_pool;
	}
#ifdef WIN32
	if (hProvider)
		::CryptReleaseContext(hProvider, 0);
#endif
}

void Random_Impl::get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes)
{
	while (num_bytes > 0)
	{
		int32_t bytes_to_copy = num_bytes;
		if (bytes_to_copy > random_pool_free)
			bytes_to_copy = random_pool_free;

		if (bytes_to_copy)
		{
			memcpy(out_dest_ptr, random_pool + (random_pool_size - random_pool_free), bytes_to_copy);
			out_dest_ptr += bytes_to_copy;
			random_pool_free -= bytes_to_copy;
			num_bytes -= bytes_to_copy;
		}
		else
		{
			fill_random_pool();
		}
	}
}

void Random_Impl::get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes)
{
	while (num_bytes > 0)
	{
		if (!random_pool_free)
			fill_random_pool();

		uint8_t random_byte = random_pool[random_pool_size - random_pool_free];
		random_pool_free--;
		if (random_byte)
		{
			*(out_dest_ptr++) = random_byte;
			num_bytes--;
		}
	}
}

void Random_Impl::fill_random_pool()
{

#ifdef WIN32
	if (!hProvider)
	{
		if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
		{
			throw Exception("Cannot acquire crypt context");
		}
	}
	if (!::CryptGenRandom(hProvider, random_pool_size, random_pool))
	{
		throw Exception("Cannot generate random numbers");
	}

#else
	File file("/dev/urandom");
	file.read(random_pool, random_pool_size);
	file.close();
#endif

	random_pool_free = random_pool_size;
}

bool Random_Impl::get_random_bool()
{
	if (!random_bool_bits_free)
	{
		get_random_bytes(&random_bool, 1);
		random_bool_bits_free = 8;
	}
	random_bool_bits_free--;

	return ((random_bool >> random_bool_bits_free) & 1);
}

Random::Random(int32_t cache_size) : impl(std::make_shared<Random_Impl>(cache_size))
{
}

void Random::get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes)
{
	impl->get_random_bytes(out_dest_ptr, num_bytes);
}

void Random::get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes)
{
	impl->get_random_bytes_nzero(out_dest_ptr, num_bytes);
}

bool Random::get_random_bool()
{
	return impl->get_random_bool();
}