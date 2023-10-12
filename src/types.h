#pragma once

#include <array>

using i8  = char;
using i16 = short;
using i32 = int;
using i64 = long long;
using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

#if defined(__OPENSSL__)
#include <openssl/md5.h>
#include <openssl/sha.h>

using u128 = std::array<u8, 128>;
using u128_c = u8[128];

using md5_t = u128;
using sha1_t = u128;
#endif

