


#pragma once

#include <cstdint>
#include <memory>
#include <vector>

class BigInt_Impl
{
public:
	BigInt_Impl(uint32_t prec = default_allocated_precision);
	BigInt_Impl(const BigInt_Impl &other);
	~BigInt_Impl();

	void read_unsigned_octets(const uint8_t *input_str, uint32_t input_length);
	void zero();
	bool make_prime(uint32_t num_bits);
	int32_t cmp_z() const;
	void set_bit(uint32_t bit_number, uint32_t value);
	int32_t significant_bits() const;
	void sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve);
	uint32_t mod_d(uint32_t d) const;
	void div_d(uint32_t d, BigInt_Impl *q, uint32_t *r) const;
	void copy(BigInt_Impl *to) const;
	void add_d(uint32_t d, BigInt_Impl *out_b) const;
	bool fermat(uint32_t w) const;
	bool pprime(int32_t nt) const;
	void set(uint32_t d);
	void set(int32_t d);
	void set(uint64_t d);
	void set(int64_t d);
	void get(uint32_t &d);
	void get(int32_t &d);
	void get(uint64_t &d);
	void get(int64_t &d);
	void exptmod(const BigInt_Impl *b, const BigInt_Impl *m, BigInt_Impl *c) const;
	void mod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void div(const BigInt_Impl *b, BigInt_Impl *q, BigInt_Impl *r) const;
	void add(const BigInt_Impl *b, BigInt_Impl *c) const;
	void sub(const BigInt_Impl *b, BigInt_Impl *c) const;
	int32_t cmp(const BigInt_Impl *b) const;
	int32_t cmp_d(uint32_t d) const;
	void sub_d(uint32_t d, BigInt_Impl *b) const;
	void neg(BigInt_Impl *b) const;
	uint32_t trailing_zeros() const;
	void div_2d(uint32_t d, BigInt_Impl *q, BigInt_Impl *r) const;
	void sqrmod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void sqr(BigInt_Impl *b) const;
	void random();
	void exch(BigInt_Impl *mp2);
	void mul(const BigInt_Impl *b, BigInt_Impl *c) const;
	void mul_d(uint32_t d, BigInt_Impl *c) const;
	bool invmod(const BigInt_Impl *m, BigInt_Impl *c) const;
	void xgcd(const BigInt_Impl *b, BigInt_Impl *g, BigInt_Impl *x, BigInt_Impl *y) const;
	void abs(BigInt_Impl *b) const;
	int32_t iseven() const;
	int32_t isodd() const;
	void div_2(BigInt_Impl *c) const;
	void to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const;
	uint32_t unsigned_octet_size() const;

	static const int32_t default_allocated_precision;

private:
	static const int32_t num_bits_in_digit = (8 * sizeof(uint32_t));
	static const int32_t num_bits_in_word = (8 * sizeof(uint64_t));
	static const uint64_t digit_radix = 1ULL << (8 * sizeof(uint32_t));
	static const uint32_t digit_half_radix = 1U << (8 * sizeof(uint32_t) - 1);
	static const uint64_t word_maximim_value = ~0;

	static const int32_t prime_tab_size = 6542;
	static std::vector<uint32_t> prime_tab;

	static void build_primes();

	static inline uint32_t internal_carryout(uint64_t w)
	{
		return w >> num_bits_in_digit;
	}

	static inline uint32_t internal_accum(uint64_t w)
	{
		return (uint32_t)w;
	}

	void internal_init_size(uint32_t prec);
	void internal_free();
	void internal_lshd(uint32_t p);
	void internal_pad(uint32_t min);
	void internal_grow(uint32_t min);
	void internal_clamp();
	int32_t internal_ispow2d(uint32_t d) const;
	void internal_div_2d(uint32_t d);
	void internal_rshd(uint32_t p);
	int32_t  internal_ispow2() const;
	void internal_mod_2d(uint32_t d);
	void internal_mul_2d(uint32_t d);

	void internal_exch(BigInt_Impl *b);

	int32_t internal_cmp_d(uint32_t d) const;

	void internal_div_d(uint32_t d, uint32_t *r);

	void internal_add_d(uint32_t d);
	void internal_sub_d(uint32_t d);
	uint32_t internal_norm(BigInt_Impl *b);
	void internal_sub(const BigInt_Impl *b);

	int32_t internal_cmp(const BigInt_Impl *b) const;
	void internal_div(BigInt_Impl *b);
	void internal_mul_d(uint32_t d);
	void internal_add(const BigInt_Impl *b);
	void internal_mul(const BigInt_Impl *b);
	void internal_div_2();
	void internal_mul_2();

	void internal_reduce(const BigInt_Impl *m, BigInt_Impl *mu);
	void internal_sqr();

	bool digits_negative;
	uint32_t digits_alloc;
	uint32_t digits_used;
	uint32_t *digits;
	BigInt_Impl &operator=(const BigInt_Impl& other) = delete;
};

class BigInt
{
public:
	BigInt();

	explicit BigInt(uint32_t value);

	explicit BigInt(int32_t value);

	explicit BigInt(uint64_t value);

	explicit BigInt(int64_t value);

	BigInt(const BigInt &other);

	~BigInt();

	BigInt &operator=(const BigInt& other);

	void read_unsigned_octets(const uint8_t *input_str, uint32_t input_length);

	void zero();

	bool make_prime(uint32_t num_bits);

	int32_t cmp_z() const;

	void set_bit(uint32_t bit_number, uint32_t value);

	int32_t significant_bits() const;

	void sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve);

	uint32_t mod_d(uint32_t d) const;

	void div_d(uint32_t d, BigInt *q, uint32_t *r) const;

	bool fermat(uint32_t w) const;

	bool pprime(int32_t nt) const;

	void set(int32_t d);
	void set(uint32_t d);
	void set(uint64_t d);
	void set(int64_t d);

	void get(uint32_t &d);
	void get(uint64_t &d);
	void get(int64_t &d);
	void get(int32_t &d);

	void exptmod(const BigInt *b, const BigInt *m, BigInt *c) const;

	void mod(const BigInt *m, BigInt *c) const;

	void div(const BigInt &b, BigInt *q, BigInt *r) const;
	void div(uint32_t d, BigInt *q, BigInt *r) const;

	BigInt operator + (const BigInt& b);
	BigInt operator + (uint32_t d);

	BigInt operator += (const BigInt& b);
	BigInt operator += (uint32_t d);

	BigInt operator - (const BigInt& b);
	BigInt operator - (uint32_t d);

	BigInt operator -= (const BigInt& b);
	BigInt operator -= (uint32_t d);

	BigInt operator * (const BigInt& b);
	BigInt operator * (uint32_t d);

	BigInt operator *= (const BigInt& b);
	BigInt operator *= (uint32_t d);

	BigInt operator / (const BigInt& b);
	BigInt operator / (uint32_t d);

	BigInt operator /= (const BigInt& b);
	BigInt operator /= (uint32_t d);

	BigInt operator % (const BigInt& b);
	BigInt operator % (uint32_t d);

	BigInt operator %= (const BigInt& b);
	BigInt operator %= (uint32_t d);

	int32_t cmp(const BigInt *b) const;

	int32_t cmp_d(uint32_t d) const;

	void neg(BigInt *b) const;

	uint32_t trailing_zeros() const;

	void sqrmod(const BigInt *m, BigInt *c) const;
	void sqr(BigInt *b) const;

	void random();

	void exch(BigInt *mp2);

	bool invmod(const BigInt *m, BigInt *c) const;

	void xgcd(const BigInt *b, BigInt *g, BigInt *x, BigInt *y) const;

	void abs(BigInt *b) const;

	bool is_even() const;

	bool is_odd() const;

	void div_2(BigInt *c) const;

	void to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const;

	uint32_t unsigned_octet_size() const;

private:
	std::unique_ptr<BigInt_Impl> impl;
};

