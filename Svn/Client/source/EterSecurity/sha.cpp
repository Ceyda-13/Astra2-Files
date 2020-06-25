

#include "stdafx.h"
#include "sha.h"

void SHA::to_hex_le(char *buffer, uint32_t value, bool uppercase) const
{
	uint32_t values[4];
	values[0] = (value & 0x000000ff);
	values[1] = ((value & 0x0000ff00) >> 8);
	values[2] = ((value & 0x00ff0000) >> 16);
	values[3] = ((value & 0xff000000) >> 24);

	uint32_t low = '0';
	uint32_t high = uppercase ? 'A' : 'a';
	for (int32_t i = 0; i < 4; i++)
	{
		uint32_t a = ((values[i] & 0xf0) >> 4);
		uint32_t b = (values[i] & 0x0f);
		buffer[i * 2 + 0] = (a < 10) ? (low + a) : (high + a - 10);
		buffer[i * 2 + 1] = (b < 10) ? (low + b) : (high + b - 10);
	}
}

void SHA::to_hex_be(char *buffer, uint32_t value, bool uppercase) const
{
	uint32_t values[4];
	values[0] = ((value & 0xff000000) >> 24);
	values[1] = ((value & 0x00ff0000) >> 16);
	values[2] = ((value & 0x0000ff00) >> 8);
	values[3] = (value & 0x000000ff);

	uint32_t low = '0';
	uint32_t high = uppercase ? 'A' : 'a';
	for (int32_t i = 0; i < 4; i++)
	{
		uint32_t a = ((values[i] & 0xf0) >> 4);
		uint32_t b = (values[i] & 0x0f);
		buffer[i * 2 + 0] = (a < 10) ? (low + a) : (high + a - 10);
		buffer[i * 2 + 1] = (b < 10) ? (low + b) : (high + b - 10);
	}
}

void SHA::to_hex64_be(char *buffer, uint64_t value, bool uppercase) const
{
	uint32_t values[8];
	values[0] = ((value & 0xff00000000000000ll) >> 56);
	values[1] = ((value & 0x00ff000000000000ll) >> 48);
	values[2] = ((value & 0x0000ff0000000000ll) >> 40);
	values[3] = ((value & 0x000000ff00000000ll) >> 32);
	values[4] = ((value & 0x00000000ff000000ll) >> 24);
	values[5] = ((value & 0x0000000000ff0000ll) >> 16);
	values[6] = ((value & 0x000000000000ff00ll) >> 8);
	values[7] = (value & 0x00000000000000ffll);

	uint32_t low = '0';
	uint32_t high = uppercase ? 'A' : 'a';
	for (int32_t i = 0; i < 8; i++)
	{
		uint32_t a = ((values[i] & 0xf0) >> 4);
		uint32_t b = (values[i] & 0x0f);
		buffer[i * 2 + 0] = (a < 10) ? (low + a) : (high + a - 10);
		buffer[i * 2 + 1] = (b < 10) ? (low + b) : (high + b - 10);
	}
}

