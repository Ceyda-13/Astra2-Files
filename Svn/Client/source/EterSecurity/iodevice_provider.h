

#pragma once

#include "iodevice.h"

class IODeviceProvider
{
public:
	virtual ~IODeviceProvider() { return; }

	
	virtual int32_t get_size() const { return -1; }

	
	virtual int32_t get_position() const { return -1; }

	virtual int32_t send(const void *data, int32_t len, bool send_all = true) = 0;

	virtual int32_t receive(void *data, int32_t len, bool receive_all = true) = 0;

	virtual int32_t peek(void *data, int32_t len) = 0;

	virtual IODeviceProvider *duplicate() = 0;

	virtual bool seek(int32_t , IODevice::SeekMode ) { return false; }
};

