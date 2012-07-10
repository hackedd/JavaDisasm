#ifndef BYTEORDER_H
#define BYTEORDER_H

#if defined(__linux__)
#include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/endian.h>
#elif defined(__OpenBSD__)
#include <sys/types.h>
#define be16toh(x) betoh16(x)
#define be32toh(x) betoh32(x)
#define be64toh(x) betoh64(x)
#else
#error Unknown OS
#endif

#endif