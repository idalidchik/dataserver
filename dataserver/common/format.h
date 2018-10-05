// format.h
//
#pragma once
#ifndef __SDL_COMMON_FORMAT_H__
#define __SDL_COMMON_FORMAT_H__

#include "dataserver/common/common.h"
#include <cwchar>

namespace sdl {

inline size_t strcount(const char * str1, const char * str2) {
    SDL_ASSERT(str1 && str2);
    const size_t len2 = strlen(str2);
    size_t count = 0;
    while ((str1 = strstr(str1, str2)) != nullptr) {
        ++count;
        str1 += len2;
    }
    return count;
}

inline size_t wstrcount(const wchar_t * str1, const wchar_t * str2) {
    SDL_ASSERT(str1 && str2);
    const size_t len2 = wcslen(str2);
    size_t count = 0;
    while ((str1 = wcsstr(str1, str2)) != nullptr) {
        ++count;
        str1 += len2;
    }
    return count;
}

template<size_t buf_size> inline
const char * format_s(char(&buf)[buf_size], const char * const str) {
    static_assert(buf_size > 20, "");
    if (str) {
        const size_t len = strlen(str);
        if (len < buf_size) {
            memcpy(buf, str, len);
            buf[len] = 0;
            return buf;
        }
    }
    SDL_ASSERT(!"format_s");
    buf[0] = 0;
    return buf;
}

template<size_t buf_size, typename... Ts> inline
const char * format_s(char(&buf)[buf_size], const char * const str, Ts&&... params) {
    static_assert(buf_size > 20, "");
    if (is_str_valid(str)) {
        SDL_ASSERT(strlen(str) < buf_size);
        SDL_ASSERT((strcount(str, "%") - 2 * strcount(str, "%%")) == sizeof...(params));
        const auto count = snprintf(buf, buf_size, str, std::forward<Ts>(params)...);
        A_STATIC_CHECK_TYPE(const int, count);
        if ((count > 0) && (count < buf_size)) {
            buf[buf_size-1] = 0;
            return buf;
        }
    }
    SDL_ASSERT(!"format_s");
    buf[0] = 0;
    return buf;
}

template<typename... Ts>
std::string to_string_format_s(const char * const mask, Ts&&... params) {
    if (is_str_valid(mask)) {
        enum { param_num = sizeof...(params) };
        SDL_ASSERT((strcount(mask, "%") - 2 * strcount(mask, "%%")) == param_num);
        std::string str(::strlen(mask) + param_num * 64, char(0)); // reserve estimate size
        for (;;) {
            const auto count = snprintf(&str[0], str.size(), mask, std::forward<Ts>(params)...);
            A_STATIC_CHECK_TYPE(const int, count);
            if ((count > 0) && (count <= str.size())) {
                str.resize(count);
                return str;
            }
            SDL_WARNING(0);
            str.resize(str.size() * 3 / 2 + 4); // try to grow by 50%
        }
    }
    SDL_ASSERT(!"to_string_format");
    return {};
}

template<size_t buf_size> inline
const wchar_t * wide_format_s(wchar_t(&buf)[buf_size], const wchar_t * const str) {
    static_assert(buf_size > 20, "");
    if (str) {
        const size_t len = wcslen(str);
        if (len < buf_size) {
            memcpy(buf, str, len * sizeof(buf[0]));
            buf[len] = 0;
            return buf;
        }
    }
    SDL_ASSERT(!"wide_format_s");
    buf[0] = 0;
    return buf;
}

template<size_t buf_size, typename... Ts> inline
const wchar_t * wide_format_s(wchar_t(&buf)[buf_size], const wchar_t * const str, Ts&&... params) {
    static_assert(buf_size > 20, "");
    if (is_str_valid(str)) {
        SDL_ASSERT(wcslen(str) < buf_size);
        SDL_ASSERT((wstrcount(str, L"%") - 2 * wstrcount(str, L"%%")) == sizeof...(params));
        const auto count = swprintf(buf, buf_size, str, std::forward<Ts>(params)...);
        A_STATIC_CHECK_TYPE(const int, count);
        if ((count > 0) && (count < buf_size)) {
            buf[buf_size-1] = 0;
            return buf;
        }
    }
    SDL_ASSERT(!"wide_format_s");
    buf[0] = 0;
    return buf;
}

// precision - number of digits after dot; trailing zeros are discarded
template<int buf_size>
const char * format_double(char(&buf)[buf_size], const double value, const int precision) {
    static_assert(buf_size > 20, "");
    static_assert(buf_size > limits::double_max_digits10, "");
    static_assert(limits::double_max_digits10 == 17, "");
#if SDL_DEBUG
    memset_zero(buf);
#endif
    SDL_ASSERT((precision > 0) || debug::is_unit_test());
    SDL_ASSERT((precision < buf_size - 1) || debug::is_unit_test());
    int c = snprintf(buf, buf_size, "%#.*f", 
        a_min_max<int, 0, limits::double_max_digits10>(precision),
        value); // print '.' char even if the value is integer
    if (c > 0) { 
        SDL_ASSERT(c < buf_size);
        c = a_min<int, limits::double_max_digits10>(a_min<int, buf_size - 1>(c));
        char * p = buf + c - 1; // pointer to last meaning char 
        while ((p > buf) && (*p == '0')) {
            --p;
        }
        SDL_ASSERT(p >= buf);
        if ((*p == '.') || (*p == ',')) {
            --p;
        }
        SDL_ASSERT((p - buf + 1) >= 0);
        p[1] = 0;
        while(--p >= buf) {
            if (*p == ',') {
                *p = '.';
                break;
            }
        }
        return buf;
    }
    SDL_ASSERT(0);
    buf[0] = 0;
    return buf;
}

} // sdl

#endif // __SDL_COMMON_FORMAT_H__
