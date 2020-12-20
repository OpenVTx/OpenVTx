#pragma once

#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define PACKED __attribute__((packed))

// Macros for big-endian (assume little endian host for now) etc
#define BYTE_SWAP_U16(x) ((uint16_t)__builtin_bswap16(x))
#define BYTE_SWAP_U32(x) ((uint32_t)__builtin_bswap32(x))


#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
