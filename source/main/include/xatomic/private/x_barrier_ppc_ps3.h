/**
 * @file xatomic\private\x_barrier_ppc_ps3.h
 * PPC (64 bit) specific barriers for PS3.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */

// Memory barriers
inline void barrier::comp()		{ ; }

inline void barrier::memr()		{ __builtin_fence(); }
inline void barrier::memw()		{ __builtin_fence(); }
inline void barrier::memrw()	{ __builtin_fence(); }
