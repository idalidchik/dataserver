// page_bpool.h
//
#pragma once
#ifndef __SDL_BPOOL_PAGE_BPOOL_H__
#define __SDL_BPOOL_PAGE_BPOOL_H__

#include "dataserver/bpool/block_head.h"
#include "dataserver/bpool/file.h"
#include "dataserver/bpool/vm_alloc.h"
#include "dataserver/common/spinlock.h"
#include "dataserver/common/algorithm.h"

namespace sdl { namespace db { namespace bpool {

struct pool_info_t final {
    using T = pool_limits;
    size_t const filesize = 0;
    size_t const page_count = 0;
    size_t const block_count = 0;
    size_t last_block = 0;
    size_t last_block_page_count = 0;
    size_t last_block_size = 0;
    explicit pool_info_t(size_t);
    size_t block_size_in_bytes(const size_t b) const {
        SDL_ASSERT(b < block_count);
        return (b == last_block) ? last_block_size : T::block_size;
    }
    size_t block_page_count(const size_t b) const {
        SDL_ASSERT(b < block_count);
        return (b == last_block) ? last_block_page_count : T::block_page_num;
    }
};

#if 0
struct bit_info_t {
    size_t const bit_size = 0;
    size_t byte_size = 0;
    size_t last_byte = 0;
    size_t last_byte_bits = 0;
    explicit bit_info_t(pool_info_t const &);
}; 
#endif

class thread_id_t : noncopyable {
    enum { max_thread = pool_limits::max_thread };
public:
    using id_type = std::thread::id;
    using data_type = std::vector<id_type>; //todo: array<id_type, max_thread>
    using size_bool = std::pair<size_t, bool>;
    thread_id_t() {
        m_data.reserve(max_thread);
    }
    static id_type get_id() {
        return std::this_thread::get_id();
    }
    size_t size() const {
        return m_data.size();
    }
    size_bool insert() {
        return insert(get_id());
    }
    size_bool insert(id_type); // throw if too many threads
    size_bool find(id_type) const;
    size_bool find() const {
        return find(get_id());
    }
    bool erase(id_type);
private:
    data_type m_data; // sorted for binary search
};

class page_bpool_alloc final : noncopyable { // to be improved
public:
    explicit page_bpool_alloc(const size_t size)
        : m_alloc(size, vm_commited::false_) {
        m_alloc_brk = base();
        throw_error_if_t<page_bpool_alloc>(!m_alloc_brk, "bad alloc");
    }
    char * base() const {
        return m_alloc.base_address();
    }
    size_t used_size() const {
        SDL_ASSERT(assert_brk());
        return m_alloc_brk - m_alloc.base_address();
    }
    size_t unused_size() const {
        return m_alloc.byte_reserved - used_size();
    }
    char * alloc(const size_t size) {
        SDL_ASSERT(size && !(size % pool_limits::block_size));
        if (size <= unused_size()) {
            if (auto result = m_alloc.alloc(m_alloc_brk, size)) {
                SDL_ASSERT(result == m_alloc_brk);
                m_alloc_brk += size;
                SDL_ASSERT(assert_brk());
                return result;
            }
        }
        SDL_ASSERT(0);
        throw_error_t<page_bpool_alloc>("bad alloc");
        return nullptr;
    }
    size_t block_id(char const * const p) const {
        SDL_ASSERT(p >= m_alloc.base_address());
        SDL_ASSERT(p < m_alloc_brk);
        const size_t size = p - base();
        SDL_ASSERT(!(size % pool_limits::block_size));
        return size / pool_limits::block_size;
    }
    char * get_block(size_t const id) const {
        char * const p = base() + id * pool_limits::block_size;
        SDL_ASSERT(p >= m_alloc.base_address());
        SDL_ASSERT(p < m_alloc_brk);
        return p;
    }
private:
    bool assert_brk() const {
        SDL_ASSERT(m_alloc_brk >= m_alloc.base_address());
        SDL_ASSERT(m_alloc_brk <= m_alloc.end_address());
        return true;
    }
private:
    vm_alloc m_alloc;
    char * m_alloc_brk = nullptr; // end of allocated space
};

class page_bpool_file {
protected:
    explicit page_bpool_file(const std::string & fname);
    ~page_bpool_file(){}
public:
    static bool valid_filesize(size_t);
    size_t filesize() const { 
        return m_file.filesize();
    }
protected:
    PagePoolFile m_file;
};

class base_page_bpool : public page_bpool_file {
protected:
    base_page_bpool(const std::string & fname, size_t, size_t);
    ~base_page_bpool(){}
public:
    const pool_info_t info;
    const size_t min_pool_size;
    const size_t max_pool_size;
};

class page_bpool final : base_page_bpool {
    sdl_noncopyable(page_bpool)
public:
    page_bpool(const std::string & fname, size_t, size_t);
    explicit page_bpool(const std::string & fname): page_bpool(fname, 0, 0){}
    ~page_bpool();
public:
    bool is_open() const;
    void const * start_address() const;
    size_t page_count() const;
    page_head const * lock_page(pageIndex);
    bool unlock_page(pageIndex);
#if SDL_DEBUG
    bool assert_page(pageIndex);
#endif
private:
#if SDL_DEBUG
    bool valid_checksum(char const * block_adr, pageIndex);
#endif
    static size_t page_bit(pageIndex pageId) {
        return pageId.value() & 7; //return pageId.value() % 8;
    }
    static block_head * get_block_head(page_head *);
    page_head const * zero_block_page(pageIndex);
    page_head const * lock_block_head(char * block_adr, pageIndex, size_t thread_id);
    bool unlock_block_head(char * block_adr, pageIndex, size_t thread_id);
    void load_zero_block();
    void read_block_from_file(char * block_adr, size_t);
private:
    using lock_guard = std::lock_guard<std::mutex>;
    mutable std::mutex m_mutex;
    mutable atomic_flag_init m_flag;
    uint32 m_accessCnt = 0;
    std::vector<block_index> m_block;
    thread_id_t m_thread_id;
    page_bpool_alloc m_alloc;
    //joinable_thread
};

}}} // sdl

#include "dataserver/bpool/page_bpool.inl"

#endif // __SDL_BPOOL_PAGE_BPOOL_H__