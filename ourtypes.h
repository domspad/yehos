#ifndef OURTYPES_H
#define OURTYPES_H

#include <stdint.h>

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifndef NULL
#define NULL ((void *) 0x0)
#endif

#endif
