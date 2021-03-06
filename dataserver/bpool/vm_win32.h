// vm_win32.h
//
#pragma once
#ifndef __SDL_BPOOL_VM_WIN32_H__
#define __SDL_BPOOL_VM_WIN32_H__

#if defined(SDL_OS_WIN32)

#include "dataserver/bpool/vm_base.h"

namespace sdl { namespace db { namespace bpool {

class vm_win32 final : public vm_base {
public:
    size_t const byte_reserved;
    size_t const page_reserved;
    size_t const block_reserved;
public:
    explicit vm_win32(size_t, vm_commited);
    ~vm_win32();
    char * base_address() const {
        return m_base_address;
    }
    char * end_address() const {
        return m_base_address + byte_reserved;
    }
    bool is_open() const {
        return m_base_address != nullptr;
    }
    char * alloc(char * start, size_t);
    bool release(char * start, size_t);
    size_t alloc_block_count() const {
        return m_alloc_block_count;
    }
private:
#if SDL_DEBUG
    bool assert_address(char const *, size_t) const;
#endif
    static char * init_vm_alloc(size_t, vm_commited);
private:
    char * const m_base_address = nullptr;
    size_t m_alloc_block_count = 0;
    SDL_DEBUG_HPP(std::vector<bool> d_block_commit;)
};

}}} // db

#endif // SDL_OS_WIN32
#endif // __SDL_BPOOL_VM_WIN32_H__
