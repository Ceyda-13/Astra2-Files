

#include "stdafx.h"
#include "Secret.h"
#ifdef _WIN32
#include <Windows.h>
#endif

#include <memory>

#ifndef _WIN32
#include <cstring>
#endif

Secret_Impl::Secret_Impl() :
	key(nullptr), key_length(0)
{
}

Secret_Impl::~Secret_Impl()
{
	free();
}

void Secret_Impl::create(uint32_t new_key_length)
{
	free();
	if (new_key_length)
	{
		key = new uint8_t[new_key_length];
		key_length = new_key_length;
	}
}

void Secret_Impl::free()
{
	if (key)
	{
#ifdef WIN32
		SecureZeroMemory(key, key_length);
#else
		volatile uint8_t *p = key;
		while (key_length--)
			*p++ = 0;
#endif

		delete[] key;
		key = nullptr;
		key_length = 0;
	}
}


Secret::Secret()
	: impl(std::make_shared<Secret_Impl>())
{
}

Secret::Secret(uint32_t new_key_length)
	: impl(std::make_shared<Secret_Impl>())
{
	impl->create(new_key_length);
}

uint32_t Secret::get_size() const
{
	return impl->get_size();
}

uint8_t *Secret::get_data() const
{
	return impl->get_data();
}

