#if X_WORDSIZE == 64

// 64bit ops

// Set a bit
static inline void __set(u32 n, value_type *addr)
{
	*addr |= (s64)1 << n;
}

// Clear a bit
static inline void __clear(u32 n, value_type *addr)
{
	*addr &= ~((s64)1 << n);
}

// Test a bit
static inline bool __test(u32 n, value_type *addr)
{
	bool bit = (*addr & ((s64)1 << n)) != 0;
	return bit;
}

// Test and set
static inline bool __tas(u32 n, value_type *addr)
{
	bool bit = (*addr & ((s64)1 << n)) != 0;
	*addr |= ((s64)1 << n);
	return bit;
}

// Test and clear
static inline bool __tac(u32 n, value_type *addr)
{
	bool bit = (*addr & ((s64)1 << n)) != 0;
	*addr &= ~((s64)1 << n);
	return bit;
}

// Atomic test and set
static inline bool __atas(u32 n, volatile value_type *addr)
{
	s64 bit = ((s64)1 << n);
	s64 old;
	do
	{
		old = *addr;
	} while (cas_s64(addr, old, old | bit) == false);
	return (old & bit)!=0;
}

// Atomic test and clear
static inline bool __atac(u32 n, volatile value_type *addr)
{
	s64 bit = ((s64)1 << n);
	s64 old;
	do
	{
		old = *addr;
	} while (cas_s64(addr, old, old & ~bit) == false);
	return (old & bit)!=0;
}

// Find first non-zero bit
static inline s32 __ffs(value_type d)
{
	if (d == 0)
		return -1;

	// 64-bit word input to count zero bits on right
	s64 v = d & X_CONSTANT_64(0x7fffffffffffffff);

	s32 c = 63;				// c will be the number of zero bits on the right
	if (v & X_CONSTANT_64(0x00000000FFFFFFFF)) c -= 32;
	if (v & X_CONSTANT_64(0x0000FFFF0000FFFF)) c -= 16;
	if (v & X_CONSTANT_64(0x00FF00FF00FF00FF)) c -= 8;
	if (v & X_CONSTANT_64(0x0F0F0F0F0F0F0F0F)) c -= 4;
	if (v & X_CONSTANT_64(0x3333333333333333)) c -= 2;
	if (v & X_CONSTANT_64(0x5555555555555555)) c -= 1;

	return c;
}

// Find first zero bit
static inline s32 __ffz(value_type d)
{
	d = ~d;
	return ffs(d);
}

#else

// 32bit ops

// Set a bit
static inline void __set(u32 n, value_type *addr)
{
	*addr |= (1 << n);
}

// Clear a bit
static inline void __clear(u32 n, value_type *addr)
{
	*addr &= ~(1 << n);
}

// Test a bit
static inline bool __test(u32 n, value_type *addr)
{
	bool bit = (*addr & (1<<n)) != 0;
	return bit;
}

// Test and set
static inline bool __tas(u32 n, value_type *addr)
{
	bool bit = (*addr & (1<<n)) != 0;
	*addr |= (1 << n);
	return bit;
}

// Test and clear
static inline bool __tac(u32 n, value_type *addr)
{
	bool bit = (*addr & (1<<n)) != 0;
	*addr &= ~(1 << n);
	return bit;
}

// Atomic test and set
static inline bool __atas(u32 n, volatile value_type *addr)
{
	s32 bit = (1 << n);
	s32 old;
	do
	{
		old = *addr;
	} while (cas_s32(addr, old, old | bit) == false);
	return (old & bit)!=0;
}

// Atomic test and clear
static inline bool __atac(u32 n, volatile value_type *addr)
{
	s32 bit = (1 << n);
	s32 old;
	do
	{
		old = *addr;
	} while (cas_s32(addr, old, old & ~bit) == false);
	return (old & bit)!=0;
}

// Find first non-zero bit
static inline s32 __ffs(value_type d)
{
	if (d == 0)
		return -1;

	// 32-bit word input to count zero bits on right
	s32 v = d & 0x7fffffff;

	u32 c = 31;				// c will be the number of zero bits on the right
	if (v & 0x0000FFFF) c -= 16;
	if (v & 0x00FF00FF) c -= 8;
	if (v & 0x0F0F0F0F) c -= 4;
	if (v & 0x33333333) c -= 2;
	if (v & 0x55555555) c -= 1;

	return c;
}

// Find first zero bit
static inline s32 __ffz(value_type d)
{
	d = ~d;
	return ffs(d);
}

#endif
