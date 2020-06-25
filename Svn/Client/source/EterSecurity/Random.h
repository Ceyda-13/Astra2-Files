

#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <wincrypt.h>
#endif

#include <memory>

class DataBuffer;

class Random_Impl
{
public:
	Random_Impl(int32_t cache_size);
	~Random_Impl();

	void get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes);
	void get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes);
	bool get_random_bool();

private:
	void fill_random_pool();

	int32_t random_pool_size;
	int32_t random_pool_free;
	uint8_t *random_pool;

#ifdef WIN32
	HCRYPTPROV hProvider;
#endif

	uint8_t random_bool;
	int32_t random_bool_bits_free;
};

class Random
{
public:
	Random(int32_t cache_size = 1024);

	void get_random_bytes(uint8_t *out_dest_ptr, int32_t num_bytes);

	void get_random_bytes_nzero(uint8_t *out_dest_ptr, int32_t num_bytes);

	bool get_random_bool();

private:
	std::shared_ptr<Random_Impl> impl;
};