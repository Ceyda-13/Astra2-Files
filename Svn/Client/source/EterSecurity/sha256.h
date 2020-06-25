

#pragma once

#include "sha.h"
#include <string>

class DataBuffer;

class SHA256_Impl : private SHA
{
public:
	SHA256_Impl(cl_sha_type new_sha_type);

	std::string get_hash(bool uppercase = false) const;
	void get_hash(uint8_t *out_hash) const;

	void reset();
	void set_hmac(const void *key_data, int32_t key_size);
	void add(const void *data, int32_t size);
	void calculate();

private:
	inline uint32_t sigma_rr2_rr13_rr22(uint32_t value) const
	{
		return (rightrotate_uint32(value, 2) ^ rightrotate_uint32(value, 13) ^ rightrotate_uint32(value, 22));
	}

	inline uint32_t sigma_rr6_rr11_rr25(uint32_t value) const
	{
		return (rightrotate_uint32(value, 6) ^ rightrotate_uint32(value, 11) ^ rightrotate_uint32(value, 25));
	}

	inline uint32_t sigma_rr7_rr18_sr3(uint32_t value) const
	{
		return (rightrotate_uint32(value, 7) ^ rightrotate_uint32(value, 18) ^ (value >> 3));
	}

	inline uint32_t sigma_rr17_rr19_sr10(uint32_t value) const
	{
		return (rightrotate_uint32(value, 17) ^ rightrotate_uint32(value, 19) ^ (value >> 10));
	}

	inline uint32_t sha_ch(uint32_t x, uint32_t y, uint32_t z) const
	{
		return (((x)& ((y) ^ (z))) ^ (z));
	}

	inline uint32_t sha_maj(uint32_t x, uint32_t y, uint32_t z) const
	{
		return  (((x)& ((y) | (z))) | ((y)& (z)));
	}

	void process_chunk();

	uint32_t h0, h1, h2, h3, h4, h5, h6, h7;
	const static int32_t block_size = 64;
	uint8_t chunk[block_size];
	int32_t chunk_filled;
	uint64_t length_message;
	bool calculated;

	cl_sha_type sha_type;
	bool hmac_enabled;
	uint8_t hmac_key_chunk[block_size];
};

class SHA256
{
public:
	SHA256();

	static const int32_t hash_size = 32;

	std::string get_hash(bool uppercase = false) const;

	void get_hash(uint8_t out_hash[hash_size]) const;

	void reset();

	void set_hmac(const void *key_data, int32_t key_size);

	void add(const void *data, int32_t size);

	void add(const DataBuffer &data);

	void calculate();

private:
	std::shared_ptr<SHA256_Impl> impl;
};

