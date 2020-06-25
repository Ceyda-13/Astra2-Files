

#pragma once

#include <memory>

class Secret_Impl
{
public:
	Secret_Impl();
	~Secret_Impl();

	uint32_t get_size() const { return key_length; }
	uint8_t *get_data() const { return key; }

	void create(uint32_t new_key_length);

private:
	void free();

	uint8_t *key;
	uint32_t key_length;
};

class Secret
{
public:
	Secret();

	Secret(uint32_t new_key_length);

	uint32_t get_size() const;

	uint8_t *get_data() const;

private:
	std::shared_ptr<Secret_Impl> impl;
};
