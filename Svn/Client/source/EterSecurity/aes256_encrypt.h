#pragma once



#include "aes.h"
#include "DataBuffer.hpp"

#include <memory>

class AES256_Encrypt_Impl : public AES_Impl
{
public:
	AES256_Encrypt_Impl();

	DataBuffer get_data() const;

	void reset();

	void purge_data();

	void set_iv(const uint8_t iv[16]);

	void set_key(const uint8_t key[32]);

	void set_padding(bool value, bool use_pkcs7, uint32_t num_additional_padded_blocks);

	void add(const void *data, int32_t size);

	void add(const DataBuffer &data);

	void calculate();

private:
	void process_chunk();

	uint32_t key_expanded[aes256_nb_mult_nr_plus1];

	uint8_t chunk[aes256_block_size_bytes];
	uint32_t initialisation_vector_1;
	uint32_t initialisation_vector_2;
	uint32_t initialisation_vector_3;
	uint32_t initialisation_vector_4;

	int32_t chunk_filled;

	bool initialisation_vector_set;
	bool cipher_key_set;
	bool calculated;
	bool padding_enabled;
	bool padding_pkcs7;
	uint32_t padding_num_additional_padded_blocks;

	DataBuffer databuffer;
};

class AES256_Encrypt
{
public:
	AES256_Encrypt();

	DataBuffer get_data() const;

	static const int32_t iv_size = 16;
	static const int32_t key_size = 32;
	static const int32_t block_size = 16;

	void reset();

	void set_iv(const uint8_t iv[iv_size]);

	void set_key(const uint8_t key[key_size]);

	void set_padding(bool value = true, bool use_pkcs7 = true, uint32_t num_additional_padded_blocks = 0);

	void add(const void *data, int32_t size);

	void add(const DataBuffer &data);

	void calculate();

private:
	std::shared_ptr<AES256_Encrypt_Impl> impl;
};