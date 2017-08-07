// alloc_unix.cpp
//
#include "dataserver/bpool/alloc_unix.h"

#if defined(SDL_OS_UNIX) || SDL_DEBUG

namespace sdl { namespace db { namespace bpool {

page_bpool_alloc_unix::page_bpool_alloc_unix(const size_t size)
    : m_alloc(get_alloc_size(size), vm_commited::false_) // may throw
{
    SDL_ASSERT(size <= capacity());
    SDL_ASSERT(size && !(size % pool_limits::page_size));
    SDL_TRACE(__FUNCTION__, " size = ", size, " ", size / megabyte<1>::value, " MB");
}

void page_bpool_alloc_unix::release(block_list_t & free_block_list)
{
    if (!free_block_list) {
        return;
    }
    break_or_continue const ret =
    free_block_list.for_each([this](block_head const * const p, block32 const id){
        SDL_ASSERT(get_block(id) == (char *)block_head::get_page_head(p));
        return m_alloc.release_block(id);
    }, freelist::true_);
    throw_error_if_t<page_bpool_alloc_unix>(is_break(ret), "release failed");
    free_block_list.clear();
}

#if SDL_DEBUG
void page_bpool_alloc_unix::trace() const {
    SDL_TRACE("~used_size = ", used_size(), ", ", used_size() / megabyte<1>::value, " MB");
    SDL_TRACE("count_free_arena_list = ", m_alloc.count_free_arena_list());
    SDL_TRACE("count_mixed_arena_list = ", m_alloc.count_mixed_arena_list());
    SDL_TRACE("arena_brk = ", m_alloc.arena_brk());
}
#endif

#if SDL_DEBUG
namespace {
class unit_test {
public:
    unit_test() {
        if (1) {
            enum { N = 8 };
            page_bpool_alloc_unix test(pool_limits::block_size * N);
            SDL_ASSERT(!test.used_block());
            SDL_ASSERT(test.unused_block() >= N); // = 16
            for (size_t i = 0; i < N; ++i) {
                SDL_ASSERT(test.alloc_block());
            }
            SDL_ASSERT(0 <= test.unused_size());
            SDL_ASSERT(test.used_block() == N);
        }
    }
};
static unit_test s_test;
}
#endif // SDL_DEBUG
}}} // sdl

#endif // #if defined(SDL_OS_UNIX) || SDL_DEBUG