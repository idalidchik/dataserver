// vm_unix.h
//
#pragma once
#ifndef __SDL_BPOOL_VM_UNIX_H__
#define __SDL_BPOOL_VM_UNIX_H__

#if defined(SDL_OS_UNIX)

#include "dataserver/bpool/vm_base.h"

namespace sdl { namespace db { namespace bpool {

class vm_unix final : public vm_base {
    enum { commit_all = true };
public:
    size_t const byte_reserved;
    size_t const page_reserved;
    size_t const slot_reserved;
    size_t const block_reserved;
public:
    explicit vm_unix(size_t, vm_commited);
    ~vm_unix();
    char * base_address() const {
        return m_base_address;
    }
    char * end_address() const {
        return m_base_address + byte_reserved;
    }
    bool is_open() const {
        return m_base_address != nullptr;
    }
    bool is_alloc(char * start, size_t) const;
    char * alloc(char * start, size_t);
    char * alloc_all() {
        return alloc(base_address(), byte_reserved);
    }
    bool release(char * start, size_t);
    bool release_all() {
        return release(base_address(), byte_reserved);
    }
private:
    bool assert_address(char const * const start, size_t const size) const {
        SDL_ASSERT(m_base_address <= start);
        SDL_ASSERT(size && !(size % page_size));
        SDL_ASSERT(start + size <= end_address());
        return true;
    }
    static char * init_vm_alloc(size_t);
    size_t last_block() const {
        return block_reserved - 1;
    }
    size_t last_block_page_count() const {
        const size_t n = page_reserved % block_page_num;
        return n ? n : block_page_num;
    }
    size_t last_block_size() const {
        return page_size * last_block_page_count();
    }
    size_t alloc_block_size(const size_t b) const {
        SDL_ASSERT(b < block_reserved);
        if (b == last_block())
            return last_block_size();
        return block_size;
    }
private:
    char * const m_base_address = nullptr;
    std::vector<bool> m_block_commit;
};

}}} // db

#endif // SDL_OS_UNIX
#endif // __SDL_BPOOL_VM_UNIX_H__