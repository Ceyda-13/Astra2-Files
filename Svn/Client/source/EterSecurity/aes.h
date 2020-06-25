#pragma once



#include "DataBuffer.hpp"
#include <cstdint>

class AES_Impl
{
public:
	AES_Impl();

	static void create_tables();

	static const int32_t aes128_key_length_nk = 4;
	static const int32_t aes128_block_size_nb = 4;
	static const int32_t aes128_num_rounds_nr = 10;
	static const int32_t aes192_key_length_nk = 6;
	static const int32_t aes192_block_size_nb = 4;
	static const int32_t aes192_num_rounds_nr = 12;
	static const int32_t aes256_key_length_nk = 8;
	static const int32_t aes256_block_size_nb = 4;
	static const int32_t aes256_num_rounds_nr = 14;

	static const int32_t aes128_key_length_bytes = aes128_key_length_nk * 4;
	static const int32_t aes128_nb_mult_nr_plus1 = aes128_block_size_nb * (aes128_num_rounds_nr + 1);
	static const int32_t aes128_block_size_bytes = aes128_block_size_nb * 4;
	static const int32_t aes192_key_length_bytes = aes192_key_length_nk * 4;
	static const int32_t aes192_nb_mult_nr_plus1 = aes192_block_size_nb * (aes192_num_rounds_nr + 1);
	static const int32_t aes192_block_size_bytes = aes192_block_size_nb * 4;
	static const int32_t aes256_key_length_bytes = aes256_key_length_nk * 4;
	static const int32_t aes256_nb_mult_nr_plus1 = aes256_block_size_nb * (aes256_num_rounds_nr + 1);
	static const int32_t aes256_block_size_bytes = aes256_block_size_nb * 4;

	static uint32_t sbox_substitution_values[256];
	static uint32_t sbox_inverse_substitution_values[256];
	static uint32_t rcon_values[10];

	static uint32_t table_e0[256];
	static uint32_t table_e1[256];
	static uint32_t table_e2[256];
	static uint32_t table_e3[256];
	static uint32_t table_d0[256];
	static uint32_t table_d1[256];
	static uint32_t table_d2[256];
	static uint32_t table_d3[256];

	void extract_encrypt_key128(const uint8_t key[aes128_key_length_bytes], uint32_t key_expanded[aes128_nb_mult_nr_plus1]);
	void extract_encrypt_key192(const uint8_t key[aes192_key_length_bytes], uint32_t key_expanded[aes192_nb_mult_nr_plus1]);
	void extract_encrypt_key256(const uint8_t key[aes256_key_length_bytes], uint32_t key_expanded[aes256_nb_mult_nr_plus1]);
	void extract_decrypt_key(uint32_t *key_expanded, int32_t num_rounds);
	void store_block(uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3, DataBuffer &databuffer);

	inline uint32_t get_word(const uint8_t *data) const
	{
		return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]));
	}

	inline void put_word(uint32_t source_value, uint8_t *dest_data) const
	{
		dest_data[0] = (uint8_t)(source_value >> 24);
		dest_data[1] = (uint8_t)(source_value >> 16);
		dest_data[2] = (uint8_t)(source_value >> 8);
		dest_data[3] = (uint8_t)(source_value);
	}

	static inline int32_t x_time(int32_t value)
	{
		return ((value << 1) ^ ((value & 0x80) ? 0x1B : 0x00));
	}

	static inline int32_t mul_value(int32_t x, int32_t y, const int32_t *power, const int32_t *logarithm)
	{
		if (x && y)
		{
			return power[(logarithm[x] + logarithm[y]) % 255];
		}
		else
		{
			return 0;
		}
	}

private:
	static bool is_tables_created;
};
