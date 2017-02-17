// This document is hereby released under the public domain.
// If you country does not have the concept of public domain,
// you need a new country.  To get a new country, call your
// regional Chinese embassy and ask about their free Tibet. ;)
#ifndef __PORTABLE_ENDIAN_H__
#define __PORTABLE_ENDIAN_H__

// *************************************************************************
//
// The following macros are ensured to be defined
//
// BYTE_ORDER
// __BYTE_ORDER
// __BYTE_ORDER__
//
// LITTLE_ENDIAN
// __LITTLE_ENDIAN
// __LITTLE_ENDIAN__
//
// BIG_ENDIAN
// __BIG_ENDIAN
// __BIG_ENDIAN__
//
// PDP_ENDIAN
// __PDP_ENDIAN
// __PDP_ENDIAN__
//
//
// The following functions/macros are ensured to be defined
//
// uint16_t htobe16 (uint16_t)
// uint16_t htole16 (uint16_t)
// uint16_t be16toh (uint16_t)
// uint16_t le16toh (uint16_t)
//
// uint32_t htobe32 (uint32_t)
// uint32_t htole32 (uint32_t)
// uint32_t be32toh (uint32_t)
// uint32_t le32toh (uint32_t)
//
// uint32_t htobe64 (uint64_t)
// uint32_t htole64 (uint64_t)
// uint32_t be64toh (uint64_t)
// uint32_t le64toh (uint64_t)
//
// For details on how to use these functions, see:
// http://man7.org/linux/man-pages/man3/endian.3.html
//
// *************************************************************************

// OS identifiers
# if !defined(WIN32) && (defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__) || defined(_WIN32_WCE))
#  define WIN32
# elif !defined(__BSD__) && (defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__))
#  define __BSD__
# endif
# if !defined(__unix__) && (defined(__APPLE__) || defined(__linux__) || defined(__CYGWIN__) || defined(__BSD__))
#  define __unix__
# endif

// OS includes
# if defined(WIN32)
#  include <winsock2.h>
#  include <sys/param.h>
# elif defined(__linux__) || defined(__CYGWIN__)
#  include <endian.h>
#  include <byteswap.h>
# elif defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
# elif defined(__BSD__)
#  include <sys/endian.h>
# elif defined(__DREAMCAST__)
#  include <machine/endian.h>
#  include <arpa/inet.h>
# else
#  error Unknown platform.  Please add support in "portable_endian.h"
# endif

// universal fixups
# if !defined(BYTE_ORDER)
# if defined(__BYTE_ORDER)
#  define BYTE_ORDER __BYTE_ORDER
#  elif defined(__BYTE_ORDER__)
#   define BYTE_ORDER __BYTE_ORDER__
#  endif
# else
#  if !defined(__BYTE_ORDER)
#   define __BYTE_ORDER BYTE_ORDER
#  endif
#  if !defined(__BYTE_ORDER__)
#   define __BYTE_ORDER__ BYTE_ORDER
#  endif
# endif

# if !defined(LITTLE_ENDIAN)
#  if defined(__LITTLE_ENDIAN)
#   define LITTLE_ENDIAN __LITTLE_ENDIAN
#  elif defined(__LITTLE_ENDIAN__)
#   define LITTLE_ENDIAN __LITTLE_ENDIAN__
#  endif
# else
#  if !defined(__LITTLE_ENDIAN)
#   define __LITTLE_ENDIAN LITTLE_ENDIAN
#  endif
#  if !defined(__LITTLE_ENDIAN__)
#   define __LITTLE_ENDIAN__ LITTLE_ENDIAN
#  endif
# endif

# if !defined(BIG_ENDIAN)
#  if defined(__BIG_ENDIAN)
#   define BIG_ENDIAN __BIG_ENDIAN
#  elif defined(__BIG_ENDIAN__)
#   define BIG_ENDIAN __BIG_ENDIAN__
#  endif
# else
#  if !defined(__BIG_ENDIAN)
#   define __BIG_ENDIAN BIG_ENDIAN
#  endif
#  if !defined(__BIG_ENDIAN__)
#   define __BIG_ENDIAN__ BIG_ENDIAN
#  endif
# endif

# if !defined(PDP_ENDIAN)
#  if defined(__PDP_ENDIAN)
#   define PDP_ENDIAN __PDP_ENDIAN
#  elif defined(__PDP_ENDIAN__)
#   define PDP_ENDIAN __PDP_ENDIAN__
#  endif
# else
#  if !defined(__PDP_ENDIAN)
#   define __PDP_ENDIAN PDP_ENDIAN
#  endif
#  if !defined(__PDP_ENDIAN__)
#   define __PDP_ENDIAN__ PDP_ENDIAN
#  endif
# endif

// OS fixups
# if defined(__APPLE__)
#  define htobe16(x) OSSwapHostToBigInt16(x)
#  define htole16(x) OSSwapHostToLittleInt16(x)
#  define be16toh(x) OSSwapBigToHostInt16(x)
#  define le16toh(x) OSSwapLittleToHostInt16(x)
#  define htobe32(x) OSSwapHostToBigInt32(x)
#  define htole32(x) OSSwapHostToLittleInt32(x)
#  define be32toh(x) OSSwapBigToHostInt32(x)
#  define le32toh(x) OSSwapLittleToHostInt32(x)
#  define htobe64(x) OSSwapHostToBigInt64(x)
#  define htole64(x) OSSwapHostToLittleInt64(x)
#  define be64toh(x) OSSwapBigToHostInt64(x)
#  define le64toh(x) OSSwapLittleToHostInt64(x)
# elif defined(__BSD__) && !defined(be16toh)
#  define be16toh(x) betoh16(x)
#  define le16toh(x) letoh16(x)
#  define be32toh(x) betoh32(x)
#  define le32toh(x) letoh32(x)
#  define be64toh(x) betoh64(x)
#  define le64toh(x) letoh64(x)
# elif defined(WIN32)
#  if BYTE_ORDER == LITTLE_ENDIAN
#   define htobe16(x) htons(x)
#   define htole16(x) (x)
#   define be16toh(x) ntohs(x)
#   define le16toh(x) (x)
#   define htobe32(x) htonl(x)
#   define htole32(x) (x)
#   define be32toh(x) ntohl(x)
#   define le32toh(x) (x)
#   define htobe64(x) htonll(x)
#   define htole64(x) (x)
#   define be64toh(x) ntohll(x)
#   define le64toh(x) (x)
#  elif BYTE_ORDER == BIG_ENDIAN
#   define htobe16(x) (x)
#   define htole16(x) __builtin_bswap16(x)
#   define be16toh(x) (x)
#   define le16toh(x) __builtin_bswap16(x)
#   define htobe32(x) (x)
#   define htole32(x) __builtin_bswap32(x)
#   define be32toh(x) (x)
#   define le32toh(x) __builtin_bswap32(x)
#   define htobe64(x) (x)
#   define htole64(x) __builtin_bswap64(x)
#   define be64toh(x) (x)
#   define le64toh(x) __builtin_bswap64(x)
#  endif
# elif defined(__linux__) && !defined(be16toh)
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define htobe16(x) __bswap_16 (x)
#   define htole16(x) (x)
#   define be16toh(x) __bswap_16 (x)
#   define le16toh(x) (x)
#   define htobe32(x) __bswap_32 (x)
#   define htole32(x) (x)
#   define be32toh(x) __bswap_32 (x)
#   define le32toh(x) (x)
#   define htobe64(x) __bswap_64 (x)
#   define htole64(x) (x)
#   define be64toh(x) __bswap_64 (x)
#   define le64toh(x) (x)
#  else
#   define htobe16(x) (x)
#   define htole16(x) __bswap_16 (x)
#   define be16toh(x) (x)
#   define le16toh(x) __bswap_16 (x)
#   define htobe32(x) (x)
#   define htole32(x) __bswap_32 (x)
#   define be32toh(x) (x)
#   define le32toh(x) __bswap_32 (x)
#   define htobe64(x) (x)
#   define htole64(x) __bswap_64 (x)
#   define be64toh(x) (x)
#   define le64toh(x) __bswap_64 (x)
#  endif
# elif defined(__DREAMCAST__) && !defined(be16toh)
#   define htobe16(x) htons(x)
#   define htole16(x) (x)
#   define be16toh(x) ntohs(x)
#   define le16toh(x) (x)
#   define htobe32(x) htonl(x)
#   define htole32(x) (x)
#   define be32toh(x) ntohl(x)
#   define le32toh(x) (x)
#   define htobe64(x) htonll(x)
#   define htole64(x) (x)
#   define be64toh(x) ntohll(x)
#   define le64toh(x) (x)
# endif
#endif
