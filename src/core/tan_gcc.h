/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_GCC_H
#define TAN_GCC_H


#define tan_likely(x)      __builtin_expect((x),1)
#define tan_unlikely(x)    __builtin_expect((x),0)

#define tan_atomic_inc(x)  __sync_fetch_and_add(&(x), 1)
#define tan_atomic_dec(x)  __sync_fetch_and_sub(&(x), 1)


#endif /* TAN_GCC_H */
