// conv.cpp
//
#include "common/common.h"
#include "utils/conv.h"
#include "utils/encoding_utf.hpp"

namespace sdl { namespace db {

std::wstring conv::cp1251_to_wide(std::string const & s)
{
    std::wstring w(s.size(), L'\0');
    if (std::mbstowcs(&w[0], s.c_str(), w.size()) == s.size()) {
        return w;
    }
    SDL_ASSERT(!"cp1251_to_wide");
    return{};
}

std::string conv::cp1251_to_utf8(std::string const & s)
{
    A_STATIC_ASSERT_TYPE(char, std::string::value_type);
    std::wstring w(s.size(), L'\0');
    if (std::mbstowcs(&w[0], s.c_str(), w.size()) == s.size()) {
        return sdl::locale::conv::utf_to_utf<std::string::value_type>(w);
    }
    SDL_ASSERT(!"cp1251_to_utf8");
    return {};
}

std::wstring conv::utf8_to_wide(std::string const & s)
{
    A_STATIC_ASSERT_TYPE(wchar_t, std::wstring::value_type); // sizeof(wchar_t) can be 2 or 4 bytes
    return sdl::locale::conv::utf_to_utf<std::wstring::value_type>(s);
}

std::string conv::wide_to_utf8(std::wstring const & s)
{
    return sdl::locale::conv::utf_to_utf<std::string::value_type>(s);
}

std::string conv::nchar_to_utf8(vector_mem_range_t const & data)
{
    const size_t len = mem_size(data);
    if (!(len % sizeof(nchar_t))) {
        const std::vector<char> src = make_vector(data);
        SDL_ASSERT(src.size() == len);
        using CharIn = uint16;
        static_assert(sizeof(nchar_t) == sizeof(CharIn), "");
        static_assert(sizeof(nchar_t) == 2, "");
        const CharIn * const begin = reinterpret_cast<const CharIn *>(src.data());
        const CharIn * const end = begin + (src.size() / sizeof(CharIn));
        return sdl::locale::conv::utf_to_utf<std::string::value_type, CharIn>(begin, end);
    }
    SDL_ASSERT(!"nchar_to_utf8");
    return{};
}

} // db
} // sdl

#if SDL_DEBUG
namespace sdl { namespace db { namespace {
    class unit_test {
    public:
        unit_test() {
            SDL_ASSERT(!conv::cp1251_to_utf8("cp1251_to_utf8").empty());
            SDL_ASSERT(conv::cp1251_to_utf8("").empty());
        }
    };
    static unit_test s_test;
}}} // sdl::db
#endif //#if SV_DEBUG