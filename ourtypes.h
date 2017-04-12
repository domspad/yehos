#ifdef YEHOS
#include "ourstdint.h"
#else
#include <stdint.h>
#endif

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
