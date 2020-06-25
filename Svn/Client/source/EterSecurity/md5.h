#pragma once



#include "sha.h"
#include <memory>
#include <string>

class DataBuffer;

class MD5_Impl : private SHA
{
public:
	MD5_Impl();

	std::string get_hash(bool uppercase = false) const;
	void get_hash(uint8_t out_hash[16]) const;
	void reset();
	void add(const void *data, int32_t size);
	void calculate();
	void set_hmac(const void *key_data, int32_t key_size);

private:
	void process_chunk();

	const static int32_t block_size = 64;

	inline uint32_t leftrotate_uint32(uint32_t value, int32_t shift) const
	{
		return (value << shift) | (value >> (32 - shift));
	}

	uint32_t h0, h1, h2, h3;
	uint8_t chunk[block_size];
	int32_t chunk_filled;
	uint32_t length_message;
	bool calculated;

	bool hmac_enabled;
	uint8_t hmac_key_chunk[block_size];
};

class MD5
{
public:
	MD5();

	static const int32_t hash_size = 16;

	std::string get_hash(bool uppercase = false) const;

	void get_hash(uint8_t out_hash[hash_size]) const;

	void reset();

	void set_hmac(const void *key_data, int32_t key_size);

	void add(const void *data, int32_t size);

	void add(const DataBuffer &data);

	void calculate();

private:
	std::shared_ptr<MD5_Impl> impl;
};
