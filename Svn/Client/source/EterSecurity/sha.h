

#pragma once

#include <cstdint>
#include <memory>

class ubyte128
{
public:
	ubyte128() {}
	ubyte128(const uint64_t &value) : high(0), low(value) {}
	ubyte128(const ubyte128 &value) : high(value.high), low(value.low) {}

	ubyte128 operator+(const uint64_t & value)
	{
		ubyte128 sum;
		sum.high = high;
		sum.low = low + value;
		if (sum.low < low)
			++sum.high;
		return sum;
	}

	ubyte128 operator+(const ubyte128 & rhs)
	{
		ubyte128 sum;
		sum.high = high + rhs.high;
		sum.low = low + rhs.low;
		if (sum.low < low)
			++sum.high;
		return sum;
	}
	ubyte128 operator-(const ubyte128 & rhs)
	{
		ubyte128 difference;
		difference.high = high - rhs.high;
		difference.low = low - rhs.low;
		if (difference.low > low)
			--difference.high;
		return difference;
	}

	uint64_t high;
	uint64_t low;
};

enum cl_sha_type
{
	cl_sha_1,
	cl_sha_224,
	cl_sha_256,
	cl_sha_384,
	cl_sha_512,
	cl_sha_512_224,
	cl_sha_512_256
};

class SHA
{
public:
	inline uint32_t leftrotate_uint32(uint32_t value, int32_t shift) const
	{
		return (value << shift) | (value >> (32 - shift));
	}

	inline uint32_t rightrotate_uint32(uint32_t value, int32_t shift) const
	{
		return (value >> shift) | (value << (32 - shift));
	}

	inline uint64_t leftrotate_uint64(uint64_t value, int32_t shift) const
	{
		return (value << shift) | (value >> (64 - shift));
	}

	inline uint64_t rightrotate_uint64(uint64_t value, int32_t shift) const
	{
		return (value >> shift) | (value << (64 - shift));
	}

	void to_hex_be(char *buffer, uint32_t value, bool uppercase) const;
	void to_hex_le(char *buffer, uint32_t value, bool uppercase) const;
	void to_hex64_be(char *buffer, uint64_t value, bool uppercase) const;
};

