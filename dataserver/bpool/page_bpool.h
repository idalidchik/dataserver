// page_bpool.h
//
#pragma once
#ifndef __SDL_BPOOL_PAGE_BPOOL_H__
#define __SDL_BPOOL_PAGE_BPOOL_H__

#include "dataserver/bpool/file.h"
#include "dataserver/bpool/alloc.h"
#include "dataserver/bpool/thread_id.h"
#include "dataserver/bpool/block_list.h"
#include "dataserver/bpool/lock_page.h"
#include "dataserver/bpool/flag_type.h"
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
    size_t block_page_count(pageIndex const p) const {
        SDL_ASSERT(p.value() < page_count);
        return block_page_count(p.value() / pool_limits::block_page_num);
    }
};

//----------------------------------------------------------

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
    size_t free_pool_block(size_t) const;
    const pool_info_t info;
    size_t min_pool_size() const { return m_min_pool_size; }
    size_t max_pool_size() const { return m_max_pool_size; }
private:
    size_t m_min_pool_size = 0;
    size_t m_max_pool_size = 0;
};

inline size_t base_page_bpool::free_pool_block(size_t const current) const {
    SDL_ASSERT(current <= info.filesize);
    return a_max((current - m_min_pool_size) / pool_limits::block_size, size_t(2));
}

//----------------------------------------------------------

class page_bpool final : base_page_bpool {
    using block32 = block_index::block32;
    sdl_noncopyable(page_bpool)
public:
    const std::thread::id init_thread_id;
    page_bpool(const std::string & fname, size_t min_size, size_t max_size);
    explicit page_bpool(const std::string & fname): page_bpool(fname, 0, 0){}
    ~page_bpool();
public:
    static bool is_zero_block(pageIndex);
    bool is_open() const;
    void const * start_address() const;
    size_t page_count() const;
    page_head const * lock_page(pageIndex);
    bool unlock_page(pageIndex);
    bool unlock_page(page_head const *);
    lock_page_head auto_lock_page(pageIndex const pageId) {
        return lock_page_head(lock_page(pageId));
    }
    size_t unlock_thread(std::thread::id, removef);
    size_t unlock_thread(removef);
    size_t free_unlocked(decommitf); // returns blocks number
public:
    size_t alloc_used_size() const;
    size_t alloc_unused_size() const;
    size_t alloc_free_size() const;
private:
    enum class unlock_result { false_, true_, fixed_ };
    using threadId_mask = thread_id_t::pos_mask;
    bool is_init_thread(std::thread::id const & id) const {
        return this->init_thread_id == id;
    }
    bool is_init_thread() const {
        return is_init_thread(std::this_thread::get_id());
    }
    fixedf thread_fixed(std::thread::id const & id) const {
        return make_fixedf(is_init_thread(id));
    }
    bool thread_unlock_page(threadIndex, pageIndex); // called from unlock_thread
    bool thread_unlock_block(threadIndex, size_t); // called from unlock_thread
    static pageIndex block_pageIndex(pageIndex);
    static pageIndex block_pageIndex(pageIndex, size_t);
    void load_zero_block();
    void read_block_from_file(char * block_adr, size_t);
    block_head const * get_block_head(block32, pageIndex) const;
    static page_head * get_block_page(char * block_adr, size_t);
    static block_head * get_block_head(char * block_adr, size_t);
    static block_head * first_block_head(char * block_adr);
    static uint32 realBlock(pageIndex); // file block 
    block_head * first_block_head(block32) const;
    block_head * first_block_head(block32, freelist) const;
    page_head const * zero_block_page(pageIndex);
    page_head const * lock_block_init(block32, pageIndex, threadId_mask const &, fixedf); // block is loaded from file
    page_head const * lock_block_head(block32, pageIndex, threadId_mask const &, fixedf, uint8); // block was loaded before
    unlock_result unlock_block_head(block_index &, block32, pageIndex, threadIndex);
    size_t free_unlock_blocks(size_t); // returns number of free blocks
    uint32 pageAccessTime() const;
    bool can_alloc_block();
    char * alloc_block();
#if SDL_DEBUG
    void trace_free_block_list();
    void trace_block(const char *, block32, pageIndex);
#endif
    void async_decommit(bool); // called from thread_data
private:
    friend page_bpool_friend; // for first_block_head
    friend lock_page_head;
    using lock_guard = std::lock_guard<std::mutex>;
    mutable std::mutex m_mutex; // should be improved
    mutable uint32 m_pageAccessTime = 0;
    std::vector<block_index> m_block;
    thread_id_t m_thread_id;
    page_bpool_alloc m_alloc;
    block_list_t m_lock_block_list;
    block_list_t m_unlock_block_list;
    block_list_t m_free_block_list;
    block_list_t m_fixed_block_list;
private:
    enum { trace_enable = 0 };
    enum { run_thread = 1 };
    class thread_data {
        page_bpool & m_parent;
        std::atomic_bool m_shutdown;
        std::atomic_bool m_ready;
        std::atomic<int> m_period; // in seconds
        std::mutex m_cv_mutex;
        std::condition_variable m_cv;
        std::unique_ptr<joinable_thread> m_thread;
    public:
        explicit thread_data(page_bpool *);
        ~thread_data();
        void launch();
        void notify();
    private:
        void shutdown();
        void run_thread();
    };
    thread_data m_td;
    friend thread_data;
};

inline size_t page_bpool::unlock_thread(const removef f) {
    return unlock_thread(std::this_thread::get_id(), f);
}

}}} // sdl

#include "dataserver/bpool/page_bpool.inl"

#endif // __SDL_BPOOL_PAGE_BPOOL_H__
