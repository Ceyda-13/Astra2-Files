

#include "stdafx.h"
#include "big_int.h"
#include "Exception.h"

#include <cstdlib>
#include <vector>
#include <exception>

const int32_t BigInt_Impl::default_allocated_precision = 64;

std::vector<uint32_t> BigInt_Impl::prime_tab;

BigInt_Impl::BigInt_Impl(uint32_t prec) : digits_negative(false), digits_alloc(0), digits_used(0), digits(nullptr)
{
	if (prec)
		internal_init_size(prec);
}

BigInt_Impl::BigInt_Impl(const BigInt_Impl &other) : digits_negative(false), digits_alloc(0), digits_used(0), digits(nullptr)
{
	other.copy(this);
}

BigInt_Impl::~BigInt_Impl()
{
	internal_free();
}

void BigInt_Impl::internal_init_size(uint32_t prec)
{
	internal_free();
	digits = new uint32_t[prec];
	if (prec)
		memset(digits, 0, prec * sizeof(uint32_t));
	digits_alloc = prec;
	digits_used = 1;
	digits_negative = false;
}

void BigInt_Impl::internal_free()
{
	if (digits)
	{
		delete[] digits;
		digits = nullptr;

	}
	digits_negative = false;
	digits_alloc = 0;
	digits_used = 0;
}

void BigInt_Impl::zero()
{
	memset(digits, 0, digits_alloc * sizeof(uint32_t));
	digits_used = 1;
	digits_negative = false;
}

int32_t BigInt_Impl::cmp_z() const
{
	if (digits_negative)
		return -1;

	if (digits_used == 1 && digits[0] == 0)
		return 0;

	return 1;
}

void BigInt_Impl::internal_lshd(uint32_t p)
{

	uint32_t  pos;
	uint32_t *dp;
	uint32_t ix;

	if (p == 0)
		return;

	internal_pad(digits_used + p);

	pos = digits_used - 1;
	dp = digits;

	for (ix = pos - p; ix >= 0; ix--)
		dp[ix + p] = dp[ix];

	for (ix = 0; ix < p; ix++)
		dp[ix] = 0;
}

void BigInt_Impl::internal_pad(uint32_t min)
{
	if (min > digits_used)
	{
		if (min > digits_alloc)
		{
			internal_grow(min);
		}
		digits_used = min;
	}
}

void BigInt_Impl::internal_grow(uint32_t min)
{
	if (min > digits_alloc)
	{

		min = ((min + (default_allocated_precision - 1)) / default_allocated_precision) * default_allocated_precision;

		auto tmp = new uint32_t[min];

		memcpy(tmp, digits, sizeof(uint32_t) * digits_used);
		memset(tmp + digits_used, 0, (min - digits_used) * sizeof(uint32_t));
		memset(digits, 0, digits_alloc * sizeof(uint32_t));
		delete[] digits;
		digits = tmp;
		digits_alloc = min;
	}
}

void BigInt_Impl::read_unsigned_octets(const uint8_t *input_str, uint32_t input_length)
{
	zero();

	uint32_t d;
	uint32_t len = input_length;
	const uint8_t *str = input_str;

	int32_t count = len % sizeof(uint32_t);
	if (count)
	{
		for (d = 0; count-- > 0; --len)
		{
			d = (d << 8) | *str++;
		}

		digits[0] = d;
	}

	for (; len > 0; len -= sizeof(uint32_t))
	{
		for (d = 0, count = sizeof(uint32_t); count > 0; --count)
		{
			d = (d << 8) | *str++;
		}
		if (cmp_z() == 0)
		{
			if (!d)
				continue;
		}
		else
		{
			internal_lshd(1);
		}
		digits[0] = d;
	}
}

void BigInt_Impl::internal_clamp()
{
	uint32_t used = digits_used;
	while (used > 1 && digits[used - 1] == 0)
		--used;
	digits_used = used;
}

void BigInt_Impl::set_bit(uint32_t bit_number, uint32_t value)
{
	uint32_t ix = bit_number / num_bits_in_digit;
	if (ix + 1 > digits_used)
	{
		internal_pad(ix + 1);
	}

	bit_number = bit_number % num_bits_in_digit;
	uint32_t mask = 1 << bit_number;

	if (value)
	{
		digits[ix] |= mask;
	}
	else
	{
		digits[ix] &= ~mask;
	}
	internal_clamp();
}

int32_t BigInt_Impl::significant_bits() const
{

	int32_t bits = 0;
	int32_t ix;

	for (ix = digits_used; ix > 0;)
	{
		uint32_t d;
		d = digits[--ix];
		if (d)
		{
			while (d)
			{
				++bits;
				d >>= 1;
			}
			break;
		}
	}
	bits += ix * num_bits_in_digit;
	if (!bits)
		bits = 1;
	return bits;
}

int32_t BigInt_Impl::internal_cmp_d(uint32_t d) const
{
	uint32_t  ua = digits_used;
	uint32_t *ap = digits;

	if (ua > 1)
		return 1;

	if (*ap < d)
		return -1;

	if (*ap > d)
		return 1;

	return 0;
}

int32_t BigInt_Impl::internal_ispow2d(uint32_t d) const
{
	int32_t   pow = 0;

	while ((d & 1) == 0)
	{
		++pow;
		d >>= 1;
	}

	if (d == 1)
		return pow;

	return -1;

}

void BigInt_Impl::copy(BigInt_Impl *to) const
{
	if (to != this)
	{
		to->internal_free();
		to->digits_negative = digits_negative;
		to->digits_alloc = digits_alloc;
		to->digits_used = digits_used;
		to->digits = nullptr;
		if (digits)
		{
			to->digits = new uint32_t[digits_alloc];
			memcpy(to->digits, digits, digits_alloc * sizeof(uint32_t));
		}
	}
}

void BigInt_Impl::internal_rshd(uint32_t p)
{

	uint32_t  ix;
	uint32_t *dp;

	if (p == 0)
		return;

	if (p >= digits_used)
	{
		memset(digits, 0, digits_alloc * sizeof(uint32_t));
		digits_used = 1;
		digits_negative = false;
		return;
	}

	dp = digits;
	for (ix = p; ix < digits_used; ix++)
		dp[ix - p] = dp[ix];

	ix -= p;
	while (ix < digits_used)
		dp[ix++] = 0;

	internal_clamp();

}

void BigInt_Impl::internal_div_2d(uint32_t d)
{

	uint32_t save, next, mask, *dp = digits;

	internal_rshd(d / num_bits_in_digit);
	d %= num_bits_in_digit;

	mask = (1 << d) - 1;

	save = 0;
	for (int32_t ix = digits_used - 1; ix >= 0; ix--)
	{
		next = dp[ix] & mask;
		dp[ix] = (dp[ix] >> d) | (save << (num_bits_in_digit - d));
		save = next;
	}

	internal_clamp();
}

void BigInt_Impl::internal_div_d(uint32_t d, uint32_t *r)
{

	uint64_t w = 0, t;
	BigInt_Impl quot_impl(digits_used);

	uint32_t *dp = digits, *qp;

	int32_t ix;

	if (d == 0)
		throw Exception("Divide by zero");

	quot_impl.digits_used = digits_used;
	qp = quot_impl.digits;

	for (ix = digits_used - 1; ix >= 0; ix--)
	{
		w = (w << num_bits_in_digit) | dp[ix];

		if (w >= d)
		{
			t = w / d;
			w = w % d;
		}
		else
		{
			t = 0;
		}
		qp[ix] = t;
	}

	if (r)
		*r = w;

	quot_impl.internal_clamp();
	quot_impl.internal_exch(this);

}

void BigInt_Impl::internal_exch(BigInt_Impl *b)
{
	bool b_digits_negative = b->digits_negative;
	uint32_t b_digits_alloc = b->digits_alloc;
	uint32_t b_digits_used = b->digits_used;
	uint32_t *b_digits = b->digits;

	b->digits_negative = digits_negative;
	b->digits_alloc = digits_alloc;
	b->digits_used = digits_used;
	b->digits = digits;

	digits_negative = b_digits_negative;
	digits_alloc = b_digits_alloc;
	digits_used = b_digits_used;
	digits = b_digits;
}

void BigInt_Impl::div_d(uint32_t d, BigInt_Impl *q, uint32_t *r) const
{
	uint32_t rem;
	int32_t      pow;

	if (d == 0)
		throw Exception("Divide by zero error");

	if ((pow = internal_ispow2d(d)) >= 0)
	{
		uint32_t  mask;

		mask = (1 << pow) - 1;
		rem = digits[0] & mask;

		if (q)
		{
			copy(q);
			q->internal_div_2d(pow);
		}

		if (r)
			*r = rem;

		return;
	}

	if (q)
	{
		copy(q);
		q->internal_div_d(d, &rem);

		if (q->internal_cmp_d(0) == 0)
			q->digits_negative = false;

	}
	else
	{
		BigInt_Impl  qp(*this);
		qp.internal_div_d(d, &rem);
		if (qp.internal_cmp_d(0) == 0)
			qp.digits_negative = false;

	}

	if (r)
		*r = rem;

}

uint32_t BigInt_Impl::mod_d(uint32_t d) const
{
	uint32_t rem;

	if (internal_cmp_d(d) > 0)
	{
		div_d(d, nullptr, &rem);
	}
	else
	{
		if (digits_negative)
		{
			rem = d - digits[0];
		}
		else
		{
			rem = digits[0];
		}
	}
	return rem;

}

void BigInt_Impl::sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve)
{

	uint32_t offset;

	uint32_t sieve_size = sieve.size();

	uint8_t *sieve_ptr = &sieve[0];
	memset(sieve_ptr, 0, sieve_size);

	for (uint32_t ix = 0; ix < num_primes; ix++)
	{
		uint32_t prime = primes[ix];
		uint32_t rem = mod_d(prime);

		if (rem == 0)
		{
			offset = 0;
		}
		else
		{
			offset = prime - (rem / 2);
		}
		for (uint32_t i = offset; i < sieve_size; i += prime)
		{
			sieve_ptr[i] = 1;
		}
	}
}

void BigInt_Impl::internal_add_d(uint32_t d)
{

	uint64_t w;
	uint32_t k;
	uint32_t ix = 1, used = digits_used;
	uint32_t *dp = digits;

	w = dp[0] + d;
	dp[0] = internal_accum(w);
	k = internal_carryout(w);

	while (ix < used && k)
	{
		w = (uint64_t)dp[ix] + k;
		dp[ix] = internal_accum(w);
		k = internal_carryout(w);
		++ix;
	}

	if (k != 0)
	{
		internal_pad(digits_used + 1);
		digits[ix] = k;
	}

}

void BigInt_Impl::internal_sub_d(uint32_t d)
{

	uint64_t   w, b = 0;
	uint32_t   ix = 1, used = digits_used;
	uint32_t *dp = digits;

	w = (digit_radix + dp[0]) - d;
	b = internal_carryout(w) ? 0 : 1;
	dp[0] = internal_accum(w);

	while (b && ix < used)
	{
		w = (digit_radix + dp[ix]) - b;
		b = internal_carryout(w) ? 0 : 1;
		dp[ix] = internal_accum(w);
		++ix;
	}

	internal_clamp();

	if (b)
		throw Exception("Borrow out detected, internal error");

}

void BigInt_Impl::add_d(uint32_t d, BigInt_Impl *out_b) const
{
	copy(out_b);

	if (!digits_negative)
	{
		out_b->internal_add_d(d);
	}
	else if (out_b->internal_cmp_d(d) >= 0)
	{
		out_b->internal_sub_d(d);
	}
	else
	{
		out_b->digits_negative = false;
		out_b->digits[0] = d - out_b->digits[0];
	}
}

void BigInt_Impl::set(uint32_t d)
{
	zero();
	digits[0] = d;
}

void BigInt_Impl::set(uint64_t d)
{
	zero();
	uint32_t d_low = d;
	uint32_t d_high = d >> 32;

	if (d_high)
	{
		internal_pad(2);
		digits[1] = d_high;
	}

	digits[0] = d_low;
}

void BigInt_Impl::set(int32_t d)
{
	zero();
	if (d < 0)
	{
		d = -d;
		digits_negative = true;
	}
	digits[0] = d;
}

void BigInt_Impl::set(int64_t d)
{
	zero();

	if (d < 0)
	{
		d = -d;
		digits_negative = true;
	}

	uint32_t d_low = d;
	uint32_t d_high = d >> 32;

	if (d_high)
	{
		internal_pad(2);
		digits[1] = d_high;
	}

	digits[0] = d_low;

}

int32_t BigInt_Impl::internal_cmp(const BigInt_Impl *b) const
{
	uint32_t   ua = digits_used, ub = b->digits_used;

	if (ua > ub)
		return 1;
	else if (ua < ub)
		return -1;
	else
	{
		int32_t ix = ua - 1;
		const uint32_t *ap = digits + ix, *bp = b->digits + ix;

		while (ix >= 0)
		{
			if (*ap > *bp)
				return 1;
			else if (*ap < *bp)
				return -1;

			--ap; --bp; --ix;
		}

		return 0;
	}
}

int32_t  BigInt_Impl::internal_ispow2() const
{

	uint32_t d;
	const uint32_t *dp;
	uint32_t  uv = digits_used;
	int32_t      extra = 0, ix;

	d = digits[uv - 1];
	while (d && ((d & 1) == 0))
	{
		d >>= 1;
		++extra;
	}

	if (d == 1)
	{
		ix = uv - 2;
		dp = digits + ix;

		while (ix >= 0)
		{
			if (*dp)
				return -1;
			--dp; --ix;
		}

		return ((uv - 1) * num_bits_in_digit) + extra;
	}

	return -1;
}

void BigInt_Impl::internal_mod_2d(uint32_t d)
{

	uint32_t  ndig = (d / num_bits_in_digit), nbit = (d % num_bits_in_digit);
	uint32_t  ix;
	uint32_t dmask, *dp = digits;

	if (ndig >= digits_used)
		return;

	dmask = (1 << nbit) - 1;
	dp[ndig] &= dmask;

	for (ix = ndig + 1; ix < digits_used; ix++)
		dp[ix] = 0;

	internal_clamp();

}

void BigInt_Impl::internal_mul_2d(uint32_t d)
{

	uint32_t save, next, mask, *dp;
	uint32_t  used;
	uint32_t ix;

	internal_lshd(d / num_bits_in_digit);

	dp = digits; used = digits_used;
	d %= num_bits_in_digit;

	mask = (1 << d) - 1;

	if ((dp[used - 1] >> (num_bits_in_digit - d)) & mask)
	{
		internal_grow(used + 1);
		dp = digits;
	}

	save = 0;
	for (ix = 0; ix < used; ix++)
	{
		next = (dp[ix] >> (num_bits_in_digit - d)) & mask;
		dp[ix] = (dp[ix] << d) | save;
		save = next;
	}

	if (save)
	{
		dp[used] = save;
		digits_used += 1;
	}

	internal_clamp();
}

uint32_t BigInt_Impl::internal_norm(BigInt_Impl *b)
{

	
	uint32_t  t, d = 0;

	t = b->digits[b->digits_used - 1];
	while (t < digit_half_radix)
	{
		t <<= 1;
		++d;
	}

	if (d != 0)
	{
		internal_mul_2d(d);
		b->internal_mul_2d(d);
	}

	return d;

}

void BigInt_Impl::internal_mul_d(uint32_t d)
{
	uint32_t k = 0;
	uint64_t w = 0;
	uint32_t ix, max;
	uint32_t *dp = digits;

	max = digits_used;
	w = (uint64_t)dp[max - 1] * d;
	if (internal_carryout(w) != 0)
	{
		internal_pad(max + 1);
		dp = digits;
	}

	for (ix = 0; ix < max; ix++)
	{
		w = ((uint64_t)dp[ix] * (uint64_t)d) + k;
		dp[ix] = internal_accum(w);
		k = internal_carryout(w);
	}

	if (k)
	{
		dp[max] = k;
		digits_used = max + 1;
	}

	internal_clamp();
}

void BigInt_Impl::internal_sub(const BigInt_Impl *b)
{
	uint64_t   w = 0;
	uint32_t *pa;
	const uint32_t *pb;
	uint32_t   ix, used = b->digits_used;

	pa = digits;
	pb = b->digits;

	for (ix = 0; ix < used; ++ix)
	{
		w = (digit_radix + *pa) - w - *pb++;
		*pa++ = internal_accum(w);
		w = internal_carryout(w) ? 0 : 1;
	}

	used = digits_used;
	while (ix < used)
	{
		w = digit_radix + *pa - w;
		*pa++ = internal_accum(w);
		w = internal_carryout(w) ? 0 : 1;
		++ix;
	}

	internal_clamp();

	if (w)
		throw Exception("Internal violation error");

}

void BigInt_Impl::internal_div(BigInt_Impl *b)
{

	uint64_t  q;
	uint32_t d;
	int32_t      ix;

	if (b->cmp_z() == 0)
		throw Exception("Divide by zero");

	if ((ix = b->internal_ispow2()) >= 0)
	{
		copy(b);
		internal_div_2d(ix);
		b->internal_mod_2d(ix);
		return;
	}

	BigInt_Impl quot_impl(digits_used);
	BigInt_Impl rem_impl(digits_used);
	BigInt_Impl t_impl(digits_used);

	d = internal_norm(b);

	ix = digits_used - 1;

	while (ix >= 0)
	{
		while (rem_impl.internal_cmp(b) < 0 && ix >= 0)
		{
			rem_impl.internal_lshd(1);
			quot_impl.internal_lshd(1);
			rem_impl.digits[0] = digits[ix];
			rem_impl.internal_clamp();
			--ix;
		}

		if (rem_impl.internal_cmp(b) < 0)
			break;

		q = rem_impl.digits[rem_impl.digits_used - 1];
		if (q <= b->digits[b->digits_used - 1] && rem_impl.digits_used > 1)
			q = (q << num_bits_in_digit) | rem_impl.digits[rem_impl.digits_used - 2];

		q /= (uint64_t)b->digits[b->digits_used - 1];

		if (q >= digit_radix)
			q = digit_radix - 1;

		b->copy(&t_impl);
		t_impl.internal_mul_d(q);

		while (t_impl.internal_cmp(&rem_impl) > 0)
		{
			--q;
			t_impl.internal_sub(b);
		}

		rem_impl.internal_sub(&t_impl);

		quot_impl.digits[0] = q;
	}

	if (d != 0)
		rem_impl.internal_div_2d(d);

	quot_impl.internal_clamp();
	rem_impl.internal_clamp();

	quot_impl.internal_exch(this);

	rem_impl.internal_exch(b);
}

void BigInt_Impl::div_2d(uint32_t d, BigInt_Impl *q, BigInt_Impl *r) const
{
	if (q)
	{
		copy(q);
		q->internal_div_2d(d);
	}

	if (r)
	{
		copy(r);
		r->internal_mod_2d(d);
	}

}

void BigInt_Impl::div(const BigInt_Impl *b, BigInt_Impl *q, BigInt_Impl *r) const
{
	int32_t      cmp;

	if (b->cmp_z() == 0)
		throw Exception("Divide by zero");

	if ((cmp = internal_cmp(b)) < 0)
	{
		if (r)
		{
			copy(r);
		}

		if (q)
			q->zero();

		return;

	}
	else if (cmp == 0)
	{

		if (q)
		{
			int32_t qneg = (digits_negative != b->digits_negative);

			q->set(1);
			if (qneg)
				q->digits_negative = true;
		}

		if (r)
			r->zero();

		return;
	}

	BigInt_Impl qtmp(*this);
	BigInt_Impl	rtmp(*b);

	qtmp.internal_div(&rtmp);

	rtmp.digits_negative = digits_negative;
	if (digits_negative == b->digits_negative)
		qtmp.digits_negative = false;
	else
		qtmp.digits_negative = true;
	if (qtmp.internal_cmp_d(0) == 0)
		qtmp.digits_negative = false;
	if (rtmp.internal_cmp_d(0) == 0)
		rtmp.digits_negative = false;

	if (q)
		qtmp.internal_exch(q);

	if (r)
		rtmp.internal_exch(r);

}

void BigInt_Impl::add(const BigInt_Impl *b, BigInt_Impl *c) const
{
	int32_t cmp;

	if (digits_negative == b->digits_negative)
	{

		if (c == b)
		{
			c->internal_add(this);
		}
		else
		{
			if (c != this)
				copy(c);

			c->internal_add(b);
		}

	}
	else if ((cmp = internal_cmp(b)) > 0)
	{

		if (c == b)
		{
			BigInt_Impl tmp(*this);

			tmp.internal_sub(b);

			tmp.internal_exch(c);
		}
		else
		{
			if (c != this)
				copy(c);
			c->internal_sub(b);
		}

	}
	else if (cmp == 0)
	{
		c->zero();
		return;
	}
	else
	{
		if (c == this)
		{
			BigInt_Impl tmp(*b);
			tmp.internal_sub(this);

			tmp.internal_exch(c);
		}
		else
		{
			if (c != b)
				b->copy(c);

			c->internal_sub(this);
		}
	}

	if (c->digits_used == 1 && c->digits[0] == 0)
		c->digits_negative = false;
}

void BigInt_Impl::internal_add(const BigInt_Impl *b)
{
	uint32_t   w = 0;
	uint32_t *pa;
	const uint32_t *pb;
	uint32_t   ix, used = b->digits_used;

	if (used > digits_used)
		internal_pad(used);

	pa = digits;
	pb = b->digits;
	for (ix = 0; ix < used; ++ix)
	{
		w += (uint64_t)*pa + (uint64_t)*pb++;
		*pa++ = internal_accum(w);
		w = internal_carryout(w);
	}

	used = digits_used;
	while (w && ix < used)
	{
		w += (uint64_t)*pa;
		*pa++ = internal_accum(w);
		w = internal_carryout(w);
		++ix;
	}

	if (w)
	{
		internal_pad(used + 1);
		digits[ix] = w;
	}
}

void BigInt_Impl::mod(const BigInt_Impl *m, BigInt_Impl *c) const
{
	int32_t  mag;

	if (m->digits_negative)
		throw Exception("Cannot mod() using negative values");

	if ((mag = internal_cmp(m)) > 0)
	{
		div(m, nullptr, c);

		if (c->digits_negative)
		{
			c->add(m, c);
		}
	}
	else if (mag < 0)
	{
		copy(c);

		if (cmp_z() < 0)
		{
			c->add(m, c);
		}
	}
	else
	{
		c->zero();
	}
}

void BigInt_Impl::internal_mul(const BigInt_Impl *b)
{
	uint32_t   k = 0;
	uint64_t   w = 0;
	uint32_t   ix, jx, ua = digits_used, ub = b->digits_used;
	uint32_t *pa;
	const uint32_t *pb;
	uint32_t *pt, *pbt;

	BigInt_Impl tmp_impl(ua + ub);

	tmp_impl.digits_used = ua + ub;

	pbt = tmp_impl.digits;

	pb = b->digits;
	for (ix = 0; ix < ub; ++ix, ++pb)
	{
		if (*pb == 0)
			continue;

		pa = digits;
		for (jx = 0; jx < ua; ++jx, ++pa)
		{
			pt = pbt + ix + jx;
			w = (uint64_t)*pb * (uint64_t)*pa + k + (uint64_t)*pt;
			*pt = internal_accum(w);
			k = internal_carryout(w);
		}

		pbt[ix + jx] = k;
		k = 0;
	}

	tmp_impl.internal_clamp();
	tmp_impl.internal_exch(this);

}



void BigInt_Impl::sub(const BigInt_Impl *b, BigInt_Impl *c) const
{
	int32_t     cmp;

	if (digits_negative != b->digits_negative)
	{
		if (c == this)
		{
			c->internal_add(b);
		}
		else
		{
			if (c != b)
				b->copy(c);
			c->internal_add(this);
			c->digits_negative = digits_negative;
		}

	}
	else if ((cmp = internal_cmp(b)) > 0)
	{
		if (c == b)
		{
			BigInt_Impl tmp(*this);
			tmp.internal_sub(b);
			tmp.internal_exch(c);
		}
		else
		{
			if (c != this)
				copy(c);

			c->internal_sub(b);
		}

	}
	else if (cmp == 0)
	{
		c->zero();
		return;

	}
	else
	{
		if (c == this)
		{
			BigInt_Impl tmp(*b);
			tmp.internal_sub(this);
			tmp.internal_exch(c);
		}
		else
		{
			if (c != b)
				b->copy(c);

			c->internal_sub(this);
		}
		c->digits_negative = !b->digits_negative;
	}

	if (c->digits_used == 1 && c->digits[0] == 0)
		c->digits_negative = false;
}

int32_t BigInt_Impl::cmp(const BigInt_Impl *b) const
{

	if (digits_negative == b->digits_negative)
	{
		int32_t  mag;

		if ((mag = internal_cmp(b)) == 0)
			return 0;

		if (digits_negative == false)
			return mag;
		else
			return -mag;

	}
	else if (digits_negative == false)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void BigInt_Impl::internal_reduce(const BigInt_Impl *m, BigInt_Impl *mu)
{

	uint32_t  um = m->digits_used;

	BigInt_Impl   q(*this);

	q.internal_rshd(um - 1);
	q.internal_mul(mu);
	q.internal_rshd(um + 1);
	internal_mod_2d(num_bits_in_digit * (um + 1));

	q.internal_mul(m);

	q.internal_mod_2d(num_bits_in_digit * (um + 1));

	sub(&q, this);

	if (cmp_z() < 0)
	{
		q.set(1);
		q.internal_lshd(um + 1);
		add(&q, this);
	}

	while (cmp(m) >= 0)
	{
		internal_sub(m);
	}
}

void BigInt_Impl::internal_sqr()
{

	uint64_t  w, k = 0;
	uint32_t  ix, jx, kx, used = digits_used;
	uint32_t *pa1, *pa2, *pt, *pbt;

	BigInt_Impl tmp_impl(2 * used);

	tmp_impl.digits_used = 2 * used;

	pbt = tmp_impl.digits;

	pa1 = digits;
	for (ix = 0; ix < used; ++ix, ++pa1)
	{
		if (*pa1 == 0)
			continue;

		w = (uint64_t)tmp_impl.digits[ix + ix] + ((uint64_t)*pa1 * (uint64_t)*pa1);

		pbt[ix + ix] = internal_accum(w);
		k = internal_carryout(w);

		for (jx = ix + 1, pa2 = digits + jx; jx < used; ++jx, ++pa2)
		{
			uint64_t  u = 0, v;

			pt = pbt + ix + jx;

			w = (uint64_t)*pa1 * (uint64_t)*pa2;

			u = (w >> (num_bits_in_word - 1)) & 1;

			w *= 2;

			v = (uint64_t)*pt + k;

			u |= ((word_maximim_value - v) < w);

			w += v;

			*pt = internal_accum(w);

			k = internal_carryout(w) | ((uint64_t)u << num_bits_in_digit);

		}

		k = (uint64_t)tmp_impl.digits[ix + jx] + k;
		pbt[ix + jx] = internal_accum(k);
		k = internal_carryout(k);

		kx = 1;
		while (k)
		{
			k = (uint64_t)pbt[ix + jx + kx] + 1;
			pbt[ix + jx + kx] = internal_accum(k);
			k = internal_carryout(k);
			++kx;
		}
	}

	tmp_impl.internal_clamp();
	tmp_impl.internal_exch(this);
}

void BigInt_Impl::exptmod(const BigInt_Impl *b, const BigInt_Impl *m, BigInt_Impl *c) const
{
	BigInt_Impl s, mu;
	uint32_t d;
	const uint32_t *db = b->digits;

	uint32_t  ub = b->digits_used;
	uint32_t dig, bit;

	if (b->cmp_z() < 0 || m->cmp_z() <= 0)
		throw Exception("Divide by zero");

	BigInt_Impl x(*this);

	x.mod(m, &x);

	s.set(1);

	mu.internal_add_d(1);
	mu.internal_lshd(2 * m->digits_used);
	mu.div(m, &mu, nullptr);

	for (dig = 0; dig < (ub - 1); dig++)
	{
		d = *db++;

		for (bit = 0; bit < num_bits_in_digit; bit++)
		{
			if (d & 1)
			{
				s.internal_mul(&x);
				s.internal_reduce(m, &mu);
			}

			d >>= 1;
			x.internal_sqr();
			x.internal_reduce(m, &mu);
		}
	}

	d = *db;

	while (d)
	{
		if (d & 1)
		{
			s.internal_mul(&x);
			s.internal_reduce(m, &mu);
		}

		d >>= 1;

		x.internal_sqr();
		x.internal_reduce(m, &mu);
	}

	s.internal_exch(c);
}

bool BigInt_Impl::fermat(uint32_t w) const
{
	BigInt_Impl  base, test;

	base.set(w);

	base.exptmod(this, this, &test);

	if (base.cmp(&test) == 0)
		return true;
	return false;
}

int32_t BigInt_Impl::cmp_d(uint32_t d) const
{
	if (digits_negative == true)
		return -1;

	return internal_cmp_d(d);
}

void BigInt_Impl::neg(BigInt_Impl *b) const
{
	copy(b);

	if (b->internal_cmp_d(0) == 0)
		b->digits_negative = false;
	else
		b->digits_negative = (b->digits_negative == true) ? false : true;
}

void BigInt_Impl::sub_d(uint32_t d, BigInt_Impl *b) const
{
	copy(b);

	if (b->digits_negative == true)
	{
		b->internal_add_d(d);
	}
	else if (b->internal_cmp_d(d) >= 0)
	{
		b->internal_sub_d(d);
	}
	else
	{
		b->neg(b);
		b->digits[0] = d - b->digits[0];
		b->digits_negative = true;
	}

	if (b->internal_cmp_d(0) == 0)
		b->digits_negative = false;
}

uint32_t BigInt_Impl::trailing_zeros() const
{
	uint32_t d = 0;
	uint32_t  n = 0;
	uint32_t ix;

	if (!cmp_z())
		return 0;

	for (ix = 0; ix < digits_used; ++ix)
	{
		d = digits[ix];
		if (d)
			break;

		n += num_bits_in_digit;
	}

	if (!d)
		return 0;
	if (sizeof(uint32_t) > 4)
	{
		if (!(d & 0xffffffffU))
		{
			d >>= 32;
			n += 32;
		}
	}

	if (sizeof(uint32_t) > 2)
	{
		if (!(d & 0xffffU))
		{
			d >>= 16;
			n += 16;
		}
	}

	if (!(d & 0xffU))
	{
		d >>= 8;
		n += 8;
	}
	if (!(d & 0xfU))
	{
		d >>= 4;
		n += 4;
	}
	if (!(d & 0x3U))
	{
		d >>= 2;
		n += 2;
	}
	if (!(d & 0x1U))
	{
		d >>= 1;
		n += 1;
	}

	return n;
}

void BigInt_Impl::sqr(BigInt_Impl *b) const
{
	copy(b);
	b->internal_sqr();
	b->digits_negative = false;
}

void BigInt_Impl::sqrmod(const BigInt_Impl *m, BigInt_Impl *c) const
{
	sqr(c);
	c->mod(m, c);
}

void BigInt_Impl::random()
{
	uint32_t  next = 0;
	uint32_t       ix, jx;

	for (ix = 0; ix < digits_used; ix++)
	{
		for (jx = 0; jx < sizeof(uint32_t); jx++)
		{
			next = (next << 8) | (rand() & 0xff);
		}
		digits[ix] = next;
	}
}

bool BigInt_Impl::pprime(int32_t nt) const
{
	BigInt_Impl m, z;
	int32_t iter;
	uint32_t jx;
	uint32_t b;

	BigInt_Impl amo(*this);
	BigInt_Impl x(*this);

	sub_d(1, &amo);

	b = amo.trailing_zeros();

	if (!b)
	{
		return false;
	}

	amo.div_2d(b, &m, nullptr);

	bool result = true;

	for (iter = 0; iter < nt; iter++)
	{
		x.internal_pad(digits_used);
		x.random();
		x.mod(this, &x);
		if (x.cmp_d(1) <= 0)
		{
			iter--;
			continue;
		}

		x.exptmod(&m, this, &z);

		if (z.cmp_d(1) == 0 || z.cmp(&amo) == 0)
		{
			result = true;
			continue;
		}

		result = false;
		for (jx = 1; jx < b; jx++)
		{
			z.sqrmod(this, &z);

			if (z.cmp_d(1) == 0)
			{
				break;
			}
			if (z.cmp(&amo) == 0)
			{
				result = true;
				break;
			}
		}

		if (result == false)
			break;
	}
	return result;
}

void BigInt_Impl::mul(const BigInt_Impl *b, BigInt_Impl *c) const
{
	bool  is_negative;

	is_negative = (digits_negative == b->digits_negative) ? false : true;

	if (c == b)
	{
		c->internal_mul(this);
	}
	else
	{
		copy(c);
		c->internal_mul(b);
	}

	if (is_negative == false || c->internal_cmp_d(0) == 0)
		c->digits_negative = false;
	else
		c->digits_negative = is_negative;

}

void BigInt_Impl::mul_d(uint32_t d, BigInt_Impl *c) const
{
	copy(c);

	bool  is_negative = digits_negative;

	c->internal_mul_d(d);

	if (is_negative == false || c->internal_cmp_d(0) == 0)
		c->digits_negative = false;
	else
		c->digits_negative = is_negative;
}

bool BigInt_Impl::invmod(const BigInt_Impl *m, BigInt_Impl *c) const
{
	bool  is_negative;

	if (cmp_z() == 0 || m->cmp_z() == 0)
		throw Exception("Divide by zero");

	is_negative = digits_negative;

	BigInt_Impl  g, x;

	xgcd(m, &g, &x, nullptr);

	if (g.cmp_d(1) != 0)
	{
		return false;
	}

	x.mod(m, c);
	c->digits_negative = is_negative;
	return true;

}

void BigInt_Impl::abs(BigInt_Impl *b) const
{
	copy(b);
	b->digits_negative = false;
}

int32_t BigInt_Impl::iseven() const
{
	return !isodd();
}

int32_t BigInt_Impl::isodd() const
{
	return (digits[0] & 1);
}

void BigInt_Impl::div_2(BigInt_Impl *c) const
{
	copy(c);
	c->internal_div_2();
}

void BigInt_Impl::internal_div_2()
{
	internal_div_2d(1);
}

void BigInt_Impl::internal_mul_2()
{
	uint32_t      ix;
	uint32_t kin = 0, kout, *dp = digits;

	for (ix = 0; ix < digits_used; ix++)
	{
		kout = (dp[ix] >> (num_bits_in_digit - 1)) & 1;
		dp[ix] = (dp[ix] << 1) | kin;

		kin = kout;
	}

	if (kin)
	{
		if (ix >= digits_alloc)
		{
			internal_grow(digits_alloc + 1);
			dp = digits;
		}

		dp[ix] = kin;
		digits_used += 1;
	}

}

void BigInt_Impl::xgcd(const BigInt_Impl *b, BigInt_Impl *g, BigInt_Impl *x, BigInt_Impl *y) const
{
	BigInt_Impl   gx, A, B, C, D;

	if (b->cmp_z() == 0)
		throw Exception("Divide by zero");

	BigInt_Impl xc(*this);
	xc.abs(&xc);

	BigInt_Impl yc(*b);
	yc.abs(&yc);

	gx.set(1);

	while (xc.iseven() && yc.iseven())
	{
		xc.internal_div_2();
		yc.internal_div_2();
		gx.internal_mul_2();
	}
	BigInt_Impl u(xc);
	BigInt_Impl v(yc);

	A.set(1);
	D.set(1);

	for (;;)
	{
		while (u.iseven())
		{
			u.internal_div_2();

			if (A.iseven() && B.iseven())
			{
				A.internal_div_2();
				B.internal_div_2();
			}
			else
			{
				A.add(&yc, &A);
				A.internal_div_2();
				B.sub(&xc, &B);
				B.internal_div_2();
			}
		}

		while (v.iseven())
		{
			v.internal_div_2();

			if (C.iseven() && D.iseven())
			{
				C.internal_div_2();
				D.internal_div_2();
			}
			else
			{
				C.add(&yc, &C);
				C.internal_div_2();
				D.sub(&xc, &D);
				D.internal_div_2();
			}
		}

		if (u.cmp(&v) >= 0)
		{
			u.sub(&v, &u);
			A.sub(&C, &A);
			B.sub(&D, &B);
		}
		else
		{
			v.sub(&u, &v);
			C.sub(&A, &C);
			D.sub(&B, &D);
		}

		if (u.cmp_z() == 0)
		{
			if (x)
				C.copy(x);

			if (y)
				D.copy(y);

			if (g)
				gx.mul(&v, g);

			break;
		}
	}

}

void BigInt_Impl::exch(BigInt_Impl *mp2)
{
	internal_exch(mp2);
}

bool BigInt_Impl::make_prime(uint32_t num_bits)
{

	const int32_t sieve_size = 32 * 1024;
	std::vector<uint8_t> work_sieve;
	work_sieve.resize(sieve_size);

	BigInt_Impl trial_impl(default_allocated_precision);

	uint32_t num_tests;

	if (num_bits >= 1300) {
		num_tests = 2;
	}
	else if (num_bits >= 850) {
		num_tests = 3;
	}
	else if (num_bits >= 650) {
		num_tests = 4;
	}
	else if (num_bits >= 550) {
		num_tests = 5;
	}
	else if (num_bits >= 450) {
		num_tests = 6;
	}
	else if (num_bits >= 400) {
		num_tests = 7;
	}
	else if (num_bits >= 350) {
		num_tests = 8;
	}
	else if (num_bits >= 300) {
		num_tests = 9;
	}
	else if (num_bits >= 250) {
		num_tests = 12;
	}
	else if (num_bits >= 200) {
		num_tests = 15;
	}
	else if (num_bits >= 150) {
		num_tests = 18;
	}
	else if (num_bits >= 100) {
		num_tests = 27;
	}
	else
		num_tests = 50;

	set_bit(num_bits - 1, 1);
	set_bit(0, 1);

	for (uint32_t i = significant_bits() - 1; i >= num_bits; --i)
	{
		set_bit(i, 0);
	}

	BigInt_Impl::build_primes();
	sieve(&BigInt_Impl::prime_tab[1], BigInt_Impl::prime_tab_size - 1, work_sieve);

	uint8_t *sieve_ptr = &work_sieve[0];

	bool found_prime = false;
	uint32_t i;
	for (i = 0; i < sieve_size; ++i)
	{
		if (sieve_ptr[i])
			continue;

		add_d(2 * i, &trial_impl);

		if (!trial_impl.fermat(2))
			continue;
		if (!trial_impl.pprime(num_tests))
			continue;
		found_prime = true;
		break;
	}
	if (!found_prime)
		return false;

	internal_exch(&trial_impl);

	memset(&work_sieve[0], 0, work_sieve.size());

	return true;
}

uint32_t BigInt_Impl::unsigned_octet_size() const
{
	uint32_t  bytes;
	int32_t  ix;
	uint32_t  d = 0;

	if (digits_negative)
		throw Exception("Digits negative");

	bytes = (digits_used * sizeof(uint32_t));

	for (ix = digits_used - 1; ix >= 0; ix--)
	{
		d = digits[ix];
		if (d)
			break;
		bytes -= sizeof(d);
	}
	if (!bytes)
		return 1;

	for (ix = sizeof(uint32_t) - 1; ix >= 0; ix--)
	{
		uint8_t x = (uint8_t)(d >> (ix * 8));
		if (x)
			break;
		--bytes;
	}
	return bytes;
}

void BigInt_Impl::to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const
{
	int32_t  ix, pos = 0;
	uint32_t  bytes;

	bytes = unsigned_octet_size();
	if (bytes > output_length)
		throw Exception("output size mismatch");

	if (bytes < output_length)
	{
		uint32_t padding = output_length - bytes;
		memset(output_str, 0, padding);
		output_str += padding;
	}

	for (ix = digits_used - 1; ix >= 0; ix--)
	{
		uint32_t  d = digits[ix];
		int32_t       jx;

		for (jx = sizeof(uint32_t) - 1; jx >= 0; jx--)
		{
			uint8_t x = (uint8_t)(d >> (jx * 8));
			if (!pos && !x)
				continue;
			*(output_str++) = x;
			pos++;
		}
	}
	if (!pos)
		*output_str = 0;
}

void BigInt_Impl::build_primes()
{
	if (!prime_tab.empty())
		return;

	prime_tab.resize(prime_tab_size);

	const int32_t sieve_max = 65536;
	const int32_t sieve_size = sieve_max / 2;
	const int32_t half_sieve_max = sieve_max / 2;

	std::vector<uint8_t> sieve;
	sieve.resize(sieve_size);
	uint8_t *sieve_ptr = &sieve[0];
	memset(sieve_ptr, 0, sieve_size * sizeof(uint8_t));

	for (int32_t prime_number = 3; prime_number < half_sieve_max; prime_number += 2)
	{
		if (sieve_ptr[prime_number / 2])
			continue;

		for (int32_t inner_count = prime_number * 3; inner_count < sieve_max; inner_count += (prime_number * 2))
		{
			sieve_ptr[inner_count / 2] = 1;
		}
	}

	int32_t prime_offset = 1;
	prime_tab[0] = 2;
	for (int32_t cnt = 1; cnt < sieve_size; cnt++)
	{
		if (!sieve_ptr[cnt])
		{
			if (prime_offset >= prime_tab_size)
				throw Exception("Build primes unexpected result");

			BigInt_Impl::prime_tab[prime_offset] = (cnt * 2) + 1;
			prime_offset++;
		}
	}
	if (prime_offset != prime_tab_size)
		throw Exception("Build primes unexpected result");
}

void BigInt_Impl::get(uint32_t &d)
{
	if (digits_used == 0)
	{
		d = 0;
		return;
	}
	if (digits_used > 1)
		throw Exception("Number greater than 32bit");

	if (digits_negative)
		throw Exception("Number is negative");

	d = digits[0];
}

void BigInt_Impl::get(int32_t &d)
{
	if (digits_used == 0)
	{
		d = 0;
		return;
	}
	if (digits_used > 1)
		throw Exception("Number greater than 32bit");

	d = digits[0];
	if (d & (1 << (num_bits_in_digit - 1)))
		throw Exception("Number is too large");

	if (digits_negative)
		d = -d;

}

void BigInt_Impl::get(uint64_t &d)
{
	if (digits_used == 0)
	{
		d = 0;
		return;
	}
	if (digits_used > 2)
		throw Exception("Number greater than 64bit");

	if (digits_negative)
		throw Exception("Number is negative");

	d = digits[0];
	if (digits_used == 2)
	{
		uint64_t d2 = digits[1];
		d |= d2 << 32;
	}
}

void BigInt_Impl::get(int64_t &d)
{
	if (digits_used == 0)
	{
		d = 0;
		return;
	}
	if (digits_used > 2)
		throw Exception("Number greater than 64bit");

	d = digits[0];
	if (digits_used == 2)
	{
		int64_t d2 = digits[1];
		if (d2 & (1 << (num_bits_in_digit - 1)))
			throw Exception("Number is too large");
		d |= d2 << 32LL;
	}
	if (digits_negative)
		d = -d;
}

BigInt::BigInt() : impl(std::make_unique<BigInt_Impl>(BigInt_Impl::default_allocated_precision))
{
}

BigInt::BigInt(uint32_t value) : impl(std::make_unique<BigInt_Impl>(BigInt_Impl::default_allocated_precision))
{
	set(value);
}

BigInt::BigInt(int32_t value) : impl(std::make_unique<BigInt_Impl>(BigInt_Impl::default_allocated_precision))
{
	set(value);
}

BigInt::BigInt(uint64_t value) : impl(std::make_unique<BigInt_Impl>(BigInt_Impl::default_allocated_precision))
{
	set(value);
}

BigInt::BigInt(int64_t value) : impl(std::make_unique<BigInt_Impl>(BigInt_Impl::default_allocated_precision))
{
	set(value);
}

BigInt::BigInt(const BigInt &other)
{
	if (&other != this)
	{
		impl = std::unique_ptr<BigInt_Impl>(new BigInt_Impl(*other.impl.get()));
	}
}

BigInt &BigInt::operator=(const BigInt& other)
{
	if (&other != this)
	{
		other.impl->copy(this->impl.get());
	}
	return *this;
}

BigInt::~BigInt()
{
}

void BigInt::zero()
{
	impl->zero();
}

int32_t BigInt::cmp_z() const
{
	return impl->cmp_z();
}

void BigInt::read_unsigned_octets(const uint8_t *input_str, uint32_t input_length)
{
	impl->read_unsigned_octets(input_str, input_length);
}

void BigInt::set_bit(uint32_t bit_number, uint32_t value)
{
	impl->set_bit(bit_number, value);
}

int32_t BigInt::significant_bits() const
{
	return impl->significant_bits();
}

void BigInt::div_d(uint32_t d, BigInt *q, uint32_t *r) const
{
	impl->div_d(d, q->impl.get(), r);
}

uint32_t BigInt::mod_d(uint32_t d) const
{
	return impl->mod_d(d);
}

void BigInt::sieve(const uint32_t *primes, uint32_t num_primes, std::vector<uint8_t> &sieve)
{
	impl->sieve(primes, num_primes, sieve);
}

void BigInt::set(uint32_t d)
{
	impl->set(d);
}

void BigInt::set(int32_t d)
{
	impl->set(d);
}

void BigInt::set(uint64_t d)
{
	impl->set(d);
}

void BigInt::set(int64_t d)
{
	impl->set(d);
}

void BigInt::get(uint32_t &d)
{
	impl->get(d);
}

void BigInt::get(int32_t &d)
{
	impl->get(d);
}

void BigInt::get(uint64_t &d)
{
	impl->get(d);
}

void BigInt::get(int64_t &d)
{
	impl->get(d);
}

void BigInt::div(uint32_t d, BigInt *q, BigInt *r) const
{
	impl->div_2d(d, q->impl.get(), r->impl.get());
}

void BigInt::mod(const BigInt *m, BigInt *c) const
{
	impl->mod(m->impl.get(), c->impl.get());
}

BigInt BigInt::operator + (const BigInt& b)
{
	BigInt c;
	impl->add(b.impl.get(), c.impl.get());
	return c;
}
BigInt BigInt::operator + (uint32_t d)
{
	BigInt c;
	impl->add_d(d, c.impl.get());
	return c;
}

BigInt BigInt::operator += (const BigInt& b)
{
	BigInt c;
	impl->add(b.impl.get(), impl.get());
	return *this;
}

BigInt BigInt::operator += (uint32_t d)
{
	BigInt c;
	impl->add_d(d, impl.get());
	return *this;
}

BigInt BigInt::operator - (const BigInt& b)
{
	BigInt c;
	impl->sub(b.impl.get(), c.impl.get());
	return c;
}

BigInt BigInt::operator - (uint32_t d)
{
	BigInt c;
	impl->sub_d(d, c.impl.get());
	return c;
}

BigInt BigInt::operator -= (const BigInt& b)
{
	impl->sub(b.impl.get(), impl.get());
	return *this;
}

BigInt BigInt::operator -= (uint32_t d)
{
	impl->sub_d(d, impl.get());
	return *this;
}

BigInt BigInt::operator * (const BigInt& b)
{
	BigInt c;
	impl->mul(b.impl.get(), c.impl.get());
	return c;
}

BigInt BigInt::operator * (uint32_t d)
{
	BigInt c;
	impl->mul_d(d, c.impl.get());
	return c;
}

BigInt BigInt::operator *= (const BigInt& b)
{
	BigInt c;
	impl->mul(b.impl.get(), impl.get());
	return *this;
}

BigInt BigInt::operator *= (uint32_t d)
{
	BigInt c;
	impl->mul_d(d, impl.get());
	return *this;
}

BigInt BigInt::operator / (const BigInt& b)
{
	BigInt c;
	impl->div(b.impl.get(), c.impl.get(), nullptr);
	return c;
}

BigInt BigInt::operator / (uint32_t d)
{
	BigInt c;
	impl->div_d(d, c.impl.get(), nullptr);
	return c;
}

BigInt BigInt::operator /= (const BigInt& b)
{
	BigInt c;
	impl->div(b.impl.get(), impl.get(), nullptr);
	return *this;
}

BigInt BigInt::operator /= (uint32_t d)
{
	BigInt c;
	impl->div_d(d, impl.get(), nullptr);
	return *this;
}

BigInt BigInt::operator % (const BigInt& b)
{
	BigInt c;
	impl->div(b.impl.get(), nullptr, c.impl.get());
	return c;
}

BigInt BigInt::operator % (uint32_t d)
{
	BigInt c;
	impl->div_2d(d, nullptr, c.impl.get());
	return c;
}

BigInt BigInt::operator %= (const BigInt& b)
{
	BigInt c;
	impl->div(b.impl.get(), nullptr, impl.get());
	return *this;
}

BigInt BigInt::operator %= (uint32_t d)
{
	BigInt c;
	impl->div_2d(d, nullptr, impl.get());
	return *this;
}

void BigInt::div(const BigInt &b, BigInt *q, BigInt *r) const
{
	impl->div(b.impl.get(), q->impl.get(), r->impl.get());
}

int32_t BigInt::cmp(const BigInt *b) const
{
	return impl->cmp(b->impl.get());
}

void BigInt::exptmod(const BigInt *b, const BigInt *m, BigInt *c) const
{
	impl->exptmod(b->impl.get(), m->impl.get(), c->impl.get());
}

bool BigInt::fermat(uint32_t w) const
{
	return impl->fermat(w);
}

int32_t BigInt::cmp_d(uint32_t d) const
{
	return impl->cmp_d(d);
}

void BigInt::neg(BigInt *b) const
{
	impl->neg(b->impl.get());
}

uint32_t BigInt::trailing_zeros() const
{
	return impl->trailing_zeros();
}

void BigInt::sqr(BigInt *b) const
{
	impl->sqr(b->impl.get());
}

void BigInt::sqrmod(const BigInt *m, BigInt *c) const
{
	impl->sqrmod(m->impl.get(), c->impl.get());
}

void BigInt::random()
{
	impl->random();
}

bool BigInt::pprime(int32_t nt) const
{
	return impl->pprime(nt);
}

bool BigInt::invmod(const BigInt *m, BigInt *c) const
{
	return impl->invmod(m->impl.get(), c->impl.get());
}

void BigInt::abs(BigInt *b) const
{
	impl->abs(b->impl.get());
}

bool BigInt::is_even() const
{
	return impl->iseven() != 0;
}

bool BigInt::is_odd() const
{
	return impl->isodd() != 0;
}

void BigInt::div_2(BigInt *c) const
{
	impl->div_2(c->impl.get());
}

void BigInt::xgcd(const BigInt *b, BigInt *g, BigInt *x, BigInt *y) const
{
	impl->xgcd(b->impl.get(), g->impl.get(), x->impl.get(), y->impl.get());
}

void BigInt::exch(BigInt *mp2)
{
	impl->exch(mp2->impl.get());
}

bool BigInt::make_prime(uint32_t num_bits)
{
	return impl->make_prime(num_bits);
}

uint32_t BigInt::unsigned_octet_size() const
{
	return impl->unsigned_octet_size();
}

void BigInt::to_unsigned_octets(uint8_t *output_str, uint32_t output_length) const
{
	impl->to_unsigned_octets(output_str, output_length);
}

