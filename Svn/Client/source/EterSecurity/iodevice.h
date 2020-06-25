

#pragma once

#include <memory>
#include <string>

class IODeviceProvider;

class IODevice_Impl
{
public:
	IODevice_Impl()
	{
	}
	~IODevice_Impl()
	{
#pragma warning(push)
#pragma warning(disable: 4150)

		if (provider)
			delete provider;

#pragma warning(pop)
	}

	bool little_endian_mode = true;
	IODeviceProvider *provider = nullptr;
};

class IODevice
{
public:
	enum SeekMode
	{
		seek_set,

		seek_cur,

		seek_end
	};

	IODevice();

	IODevice(IODeviceProvider *provider);

	~IODevice();

	bool is_null() const { return !impl; }

	void throw_if_null() const;

	
	int32_t get_size() const;

	
	int32_t get_position() const;

	
	bool is_little_endian() const;

	const IODeviceProvider *get_provider() const;

	IODeviceProvider *get_provider();

	
	int32_t send(const void *data, int32_t len, bool send_all = true);

	int32_t receive(void *data, int32_t len, bool receive_all = true);

	int32_t peek(void *data, int32_t len);

	bool seek(int32_t position, SeekMode mode = seek_set);

	int32_t read(void *data, int32_t len, bool receive_all = true);

	int32_t write(const void *data, int32_t len, bool send_all = true);

	void set_system_mode();

	void set_big_endian_mode();

	void set_little_endian_mode();

	void write_int64(int64_t data);

	void write_uint64(uint64_t data);

	void write_int32(int32_t data);

	void write_uint32(uint32_t data);

	void write_int16(int16_t data);

	void write_uint16(uint16_t data);

	void write_int8(int8_t data);

	void write_uint8(uint8_t data);

	void write_float(float data);

	void write_string_a(const std::string &str);

	void write_string_nul(const std::string &str);

	void write_string_text(const std::string &str);

	
	int64_t read_int64();

	
	uint64_t read_uint64();

	
	int32_t read_int32();

	
	uint32_t read_uint32();

	
	int16_t read_int16();

	
	uint16_t read_uint16();

	
	int8_t read_int8();

	
	uint8_t read_uint8();

	
	float read_float();

	
	std::string read_string_a();

	
	std::string read_string_nul();

	std::string read_string_text(const char *skip_initial_chars, const char *read_until_chars, bool allow_eof = true);

	IODevice duplicate();

protected:
	std::shared_ptr<IODevice_Impl> impl;
};

