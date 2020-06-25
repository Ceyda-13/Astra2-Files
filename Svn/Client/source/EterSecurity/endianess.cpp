

#include "stdafx.h"
#include "cl_endian.h"

void Endian::swap(void *data, int32_t type_size, int32_t total_times)
{
	if (type_size == 1) return;

	uint8_t *d = (uint8_t *)data;

	for (int32_t j = 0; j < total_times; j++)
	{
		for (int32_t i = 0; i < type_size / 2; i++)
		{
			uint8_t a = d[i];
			d[i] = d[type_size - 1 - i];
			d[type_size - 1 - i] = a;
		}

		d += type_size;
	}
}

bool Endian::is_system_big()
{
	const int32_t i = 1;
	return !(*(char *)(&i));
}

bool Endian::is_system_64bit()
{
	return (sizeof(int32_t*) == 8);
}

