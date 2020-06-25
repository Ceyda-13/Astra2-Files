

#include "stdafx.h"
#include "File.h"
#include "Exception.h"
#include "iodevice.h"
#include "iodevice_provider_file.h"

std::string File::read_text(const std::string &filename)
{
	File file(filename);
	uint32_t file_size = file.get_size();
	std::vector<char> text;
	text.resize(file_size + 1);
	text[file_size] = 0;
	if (file_size)
		file.read(&text[0], file_size);
	file.close();
	if (file_size)
		return std::string(&text[0]);
	else
		return std::string();
}

DataBuffer File::read_bytes(const std::string &filename)
{
	File file(filename);
	DataBuffer buffer(file.get_size());
	file.read(buffer.get_data(), buffer.get_size());
	file.close();
	return buffer;
}

void File::write_text(const std::string &filename, const std::string &text, bool write_bom)
{
	File file(filename, create_always, access_write);
	if (write_bom)
	{
		uint8_t bom[3] = { 0xef, 0xbb, 0xbf };
		file.write(bom, 3);
	}
	file.write(text.data(), text.length());
	file.close();
}

void File::write_bytes(const std::string &filename, const DataBuffer &bytes)
{
	File file(filename, create_always, access_write);
	file.write(bytes.get_data(), bytes.get_size());
	file.close();
}

File::File()
	: IODevice(new IODeviceProvider_File())
{
}

File::File(
	const std::string &filename)
	: IODevice(new IODeviceProvider_File(filename, open_existing, access_read, share_all, 0))
{
}

File::File(
	const std::string &filename,
	OpenMode open_mode,
	uint32_t access,
	uint32_t share,
	uint32_t flags)
	: IODevice(new IODeviceProvider_File(filename, open_mode, access, share, flags))
{
}
File::~File()
{
}

bool File::open(
	const std::string &filename)
{
	IODeviceProvider_File *provider = dynamic_cast<IODeviceProvider_File*>(impl->provider);
	return provider->open(filename, open_existing, access_read, share_all, 0);
}

bool File::open(
	const std::string &filename,
	OpenMode open_mode,
	uint32_t access,
	uint32_t share,
	uint32_t flags)
{
	IODeviceProvider_File *provider = dynamic_cast<IODeviceProvider_File*>(impl->provider);
	return provider->open(filename, open_mode, access, share, flags);
}

void File::close()
{
	IODeviceProvider_File *provider = dynamic_cast<IODeviceProvider_File*>(impl->provider);
	provider->close();
}

