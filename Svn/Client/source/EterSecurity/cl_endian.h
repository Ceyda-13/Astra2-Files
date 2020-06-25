

#pragma once

class Endian
{
public:
	static void swap(void *data, int32_t type_size, int32_t total_times = 1);

	static void swap_if_big(void *data, int32_t type_size, int32_t total_times = 1) { if (is_system_big()) swap(data, type_size, total_times); }
	static void swap_if_little(void *data, int32_t type_size, int32_t total_times = 1) { if (!is_system_big()) swap(data, type_size, total_times); }

	
	static bool is_system_big();

	
	static bool is_system_64bit();
};
