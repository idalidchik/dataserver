// page_iterator.h
//
#ifndef __SDL_SYSTEM_PAGE_ITERATOR_H__
#define __SDL_SYSTEM_PAGE_ITERATOR_H__

#pragma once

#include <iterator>

namespace sdl { namespace db {

template<class T, class _value_type, class Friend = T>
class page_iterator : public std::iterator<
        std::bidirectional_iterator_tag,
        _value_type>
{
public:
    using value_type = _value_type;
private:
    T * parent;
    value_type current; // std::shared_ptr to allow iterator assignment

    friend Friend;
    page_iterator(T * p, value_type && v): parent(p), current(std::move(v)) {
        SDL_ASSERT(parent);
    }
    explicit page_iterator(T * p): parent(p) {
        SDL_ASSERT(parent);
    }
public:
    page_iterator() : parent(nullptr), current{} {}

    page_iterator & operator++() { // preincrement
        SDL_ASSERT(parent && current.get());
        parent->load_next(current);
        return (*this);
    }
    page_iterator operator++(int) { // postincrement
        auto temp = *this;
        ++(*this);
        SDL_ASSERT(temp != *this);
        return temp;
    }
    page_iterator & operator--() { // predecrement
        SDL_ASSERT(parent && current.get());
        parent->load_prev(current);
        return (*this);
    }
    page_iterator operator--(int) { // postdecrement
        auto temp = *this;
        --(*this);
        SDL_ASSERT(temp != *this);
        return temp;
    }
    bool operator==(const page_iterator& it) const {
        SDL_ASSERT(!parent || !it.parent || (parent == it.parent));
        return (parent == it.parent) && T::is_same(current, it.current);
    }
    bool operator!=(const page_iterator& it) const {
        return !(*this == it);
    }
    value_type const & operator*() const {
        SDL_ASSERT(parent && current.get());
        return current;
    }
    value_type const * operator->() const {
        return &(**this);
    }
};

} // db
} // sdl

#endif // __SDL_SYSTEM_PAGE_ITERATOR_H__
