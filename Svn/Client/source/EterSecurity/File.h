

#pragma once

#include "iodevice.h"
#include "DataBuffer.hpp"

class File : public IODevice
{
public:
	static std::string read_text(const std::string &filename);

	static DataBuffer read_bytes(const std::string &filename);

	static void write_text(const std::string &filename, const std::string &text, bool write_bom = false);

	static void write_bytes(const std::string &filename, const DataBuffer &bytes);

	
	enum AccessFlags
	{
		access_read = 1,

		access_write = 2,

		access_read_write = access_read | access_write
	};

	enum ShareFlags
	{
		share_read = 1,

		share_write = 2,

		share_delete = 4,

		share_all = share_read + share_write + share_delete
	};

	enum OpenMode
	{
		open_always,

		open_existing,

		open_existing_truncate,

		create_always,

		create_new
	};

	enum Flags
	{
		flag_write_through = 1,
		flag_no_buffering = 2,
		flag_random_access = 4,
		flag_sequential_scan = 8
	};

	File();

	File(
		const std::string &filename);

	File(
		const std::string &filename,
		OpenMode mode,
		uint32_t access,
		uint32_t share = share_all,
		uint32_t flags = 0);

	~File();

	bool open(
		const std::string &filename);

	bool open(
		const std::string &filename,
		OpenMode mode,
		uint32_t access,
		uint32_t share = share_all,
		uint32_t flags = 0);

	void close();

private:
};

