// priority_queue.h
//
#pragma once
#ifndef __SDL_NUMERIC_PRIORITY_QUEUE_H__
#define __SDL_NUMERIC_PRIORITY_QUEUE_H__

#include "dataserver/common/common.h"
#include <vector>
#include <deque>

namespace sdl {

template<typename T, typename U, class Container = std::vector<T> >
class priority_queue: sdl::noncopyable {  
    using data_array = U;
    data_array & m_qp;  // weight & priority index
    Container m_pq;     // priority queue as binary heap, minimum weight is on top
    static void reserve(std::deque<T> & v, size_t size) {}
    template<typename vector_type>
    static void reserve(vector_type & v, size_t size) { 
        v.reserve(size);
    }
public:
    using value_type = T;
    explicit priority_queue(data_array & m)
        : m_qp(m)
        , m_pq(1) // m_pq[0] reserved
    {
        A_STATIC_CHECK_NOT_TYPE(void, std::declval<typename data_array::value_type>().weight());
        A_STATIC_CHECK_NOT_TYPE(void, std::declval<typename data_array::value_type>().priority);
        SDL_ASSERT(empty());
        reserve(m_pq, 64);
    }
    size_t size() const {
        SDL_ASSERT(!m_pq.empty());
        return m_pq.size() - 1;
    }
    bool empty() const {
        return 0 == size();
    }
    value_type top() const {
        SDL_ASSERT(!empty());
        return m_pq[1];
    }
    value_type getmin();
    void insert(value_type);
    void lower(value_type);
private:
#if SDL_DEBUG
    bool check_index(const size_t i) const {
        SDL_ASSERT(i >= 0);
        SDL_ASSERT(i < m_pq.size());
        SDL_ASSERT(m_pq.size() < (size_t) std::numeric_limits<value_type>::max());
        return true;
    }
#endif
    bool less(size_t i, size_t j) const;
    void exch(size_t i, size_t j);
    void fixUp(size_t k);
    void fixDown(size_t k, size_t N);
};

template<typename T, typename U, class Container>
bool priority_queue<T, U, Container>::less(const size_t i, const size_t j) const {
    SDL_ASSERT(check_index(i));
    SDL_ASSERT(check_index(j));
    const auto & pi = m_qp[m_pq[i]];
    const auto & pj = m_qp[m_pq[j]];
    return pi.weight() < pj.weight();
}

template<typename T, typename U, class Container>
void priority_queue<T, U, Container>::exch(const size_t i, const size_t j)
{
    SDL_ASSERT(check_index(i));
    SDL_ASSERT(check_index(j));
    std::swap(m_pq[i], m_pq[j]);
    auto & pi = m_qp[m_pq[i]];
    auto & pj = m_qp[m_pq[j]];
    SDL_ASSERT((i == j) == (pi.priority == pj.priority));
    A_STATIC_CHECK_TYPE(value_type, pi.priority); // should be the same type
    A_STATIC_CHECK_TYPE(value_type, pj.priority);
    pi.priority = static_cast<value_type>(i);
    pj.priority = static_cast<value_type>(j);
}

template<typename T, typename U, class Container>
void priority_queue<T, U, Container>::fixUp(size_t k)
{
    SDL_ASSERT(check_index(k));
    while ((k > 1) && less(k, k / 2)) {
        exch(k, k / 2);
        k /= 2;
    }
}

template<typename T, typename U, class Container>
void priority_queue<T, U, Container>::fixDown(size_t k, const size_t N)
{
    SDL_ASSERT(check_index(k));
    SDL_ASSERT(check_index(N));
    while (k <= (N / 2)) {
        size_t j = k + k;
        if ((j < N) && less(j + 1, j)) {
            ++j;
        }
        if (!less(j, k))
            break;
        exch(k, j);
        k = j;
        SDL_ASSERT(k <= N);
    }
}

template<typename T, typename U, class Container>
T priority_queue<T, U, Container>::getmin()
{
    SDL_ASSERT(!empty());
    const auto top = m_pq[1];
    exch(1, size());
    fixDown(1, size() - 1);
    m_pq.pop_back();
    return top;
}

template<typename T, typename U, class Container>
void priority_queue<T, U, Container>::insert(value_type v)
{
    m_pq.push_back(v);
    m_qp[v].priority = static_cast<value_type>(size());
    fixUp(size());
}

template<typename T, typename U, class Container>
void priority_queue<T, U, Container>::lower(value_type v)
{
    fixUp(m_qp[v].priority);
}

} // sdl

#endif // __SDL_NUMERIC_PRIORITY_QUEUE_H__