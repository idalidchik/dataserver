// static.h
//
#ifndef __SDL_COMMON_STATIC_H__
#define __SDL_COMMON_STATIC_H__

#pragma once

#include "config.h"
#include <stdio.h> // for sprintf_s

#define A_NONCOPYABLE(classname) \
private: \
    classname(classname const &) = delete; \
    classname & operator=(classname const &) = delete; 

namespace sdl {

using uint8 = std::uint8_t;
using int16 = std::int16_t;
using uint16 = std::uint16_t;
using int32 = std::int32_t;
using uint32 = std::uint32_t;
using int64 = std::int64_t;
using uint64 = std::uint64_t;

inline bool is_str_empty(const char * str)
{
    return !(str && str[0]);
}

inline bool is_str_empty(const wchar_t * str)
{
    return !(str && str[0]);
}

inline bool is_str_valid(const char * str)
{
    return str && str[0];
}

inline bool is_str_valid(const wchar_t * str)
{
    return str && str[0];
}

template <class T> inline T a_min(const T a, const T b)
{
    static_assert(sizeof(T) <= sizeof(double), "");
    return (a < b) ? a : b;
}

template <class T> inline T a_max(const T a, const T b)
{
    static_assert(sizeof(T) <= sizeof(double), "");
    return (b < a) ? a : b;
}

template <class T> inline T a_abs(const T a)
{
    static_assert(sizeof(T) <= sizeof(double), "");
    return (a < 0) ? (-a) : a;
}

template<class T> inline void memset_pod(T& dest, int value)
{
    A_STATIC_ASSERT_IS_POD(T);
    memset(&dest, value, sizeof(T));
}

template<class T> inline void memset_zero(T& dest)
{
    A_STATIC_ASSERT_IS_POD(T);
    memset(&dest, 0, sizeof(T));
}

template<unsigned int v> struct is_power_2
{
    enum { value = v && !(v & (v - 1)) };
};

template<size_t N> struct megabyte
{
    enum { value = N * (1 << 20) };
};

template<size_t N> struct kilobyte
{
    enum { value = N * (1 << 10) };
};

template<size_t N> struct min_to_sec
{
    enum { value = N * 60 };
};

template<size_t N> struct hour_to_sec
{
    enum { value = N * 60 * 60 };
};

template<size_t N> struct day_to_sec
{
    enum { value = 24 * hour_to_sec<N>::value };
};

// This template function declaration is used in defining A_ARRAY_SIZE.
// Note that the function doesn't need an implementation, as we only use its type.
template <typename T, size_t N> char(&A_ARRAY_SIZE_HELPER(T(&array)[N]))[N];
#define A_ARRAY_SIZE(a) (sizeof(A_ARRAY_SIZE_HELPER(a)))

template <typename T> struct array_info;
template <typename T, size_t N>
struct array_info<T[N]>
{
    typedef T value_type;
    enum { size = N };
};

//Alternative of macros A_ARRAY_SIZE
//This function template can't be instantiated with pointer argument, only array.
template< class Type, size_t n > inline size_t count_of(Type(&)[n])
{
    return n;
}

template<size_t buf_size, typename... Ts> inline
const char * format_s(char(&buf)[buf_size], Ts&&... params) {
    if (sprintf(buf, std::forward<Ts>(params)...) > 0)
        return buf;
    SDL_ASSERT(0);
    buf[0] = 0;
    return buf;
}

} // sdl

#endif // __SDL_COMMON_STATIC_H__