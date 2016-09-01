// geography.h
//
#pragma once
#ifndef __SDL_SYSTEM_GEOGRAPHY_H__
#define __SDL_SYSTEM_GEOGRAPHY_H__

#include "geo_data.h"

namespace sdl { namespace db {

class geo_mem : noncopyable { // movable
public:
    class point_access {
        using obj_type = geo_pointarray;
        spatial_point const * const m_begin;
        spatial_point const * const m_end;
    public:
        point_access(obj_type const * p, geo_tail const * tail, size_t const subobj)
            : m_begin(tail->begin(*p, subobj))
            , m_end(tail->end(*p, subobj))
        {
            SDL_ASSERT(m_begin && m_end);
            SDL_ASSERT(size());
        }
        spatial_point const * begin() const {
            return m_begin;
        }
        spatial_point const * end() const {
            return m_end;
        }
        size_t size() const {
            return m_end - m_begin;
        }
        spatial_point const & operator[](size_t const i) const {
            SDL_ASSERT(i < this->size());
            return *(begin() + i);
        }
    };
public:
    using data_type = vector_mem_range_t;
    geo_mem(){}
    explicit geo_mem(data_type && m);
    geo_mem(geo_mem && v): m_type(spatial_type::null) {
        this->swap(v);
    }
    const geo_mem & operator=(geo_mem && v) {
        this->swap(v);
        return *this;
    }
    bool is_null() const {
        return m_type == spatial_type::null;
    }
    explicit operator bool() const {
        return !is_null();
    }
    spatial_type type() const {
        return m_type;
    }
    data_type const & data() const {
        return m_data;
    }
    size_t size() const {
        return mem_size(m_data);
    }
    std::string STAsText() const;
    bool STContains(spatial_point const &) const;
private:
    template<class T> T const * cast_t() const && = delete;
    template<class T> T const * cast_t() const & {        
        SDL_ASSERT(T::this_type == m_type);    
        T const * const obj = reinterpret_cast<T const *>(m_geography);
        SDL_ASSERT(size() >= obj->data_mem_size());
        return obj;
    }
    geo_pointarray const * cast_pointarray() const { // for get_subobj
        SDL_ASSERT((m_type == spatial_type::multipolygon) || 
                   (m_type == spatial_type::multilinestring));
        geo_pointarray const * const obj = reinterpret_cast<geo_pointarray const *>(m_geography);
        SDL_ASSERT(size() >= obj->data_mem_size());
        return obj;
    }
public:
    geo_point const * cast_point() const && = delete;
    geo_polygon const * cast_polygon() const && = delete;    
    geo_multipolygon const * cast_multipolygon() const && = delete;
    geo_linesegment const * cast_linesegment() const && = delete;
    geo_linestring const * cast_linestring() const && = delete;    
    geo_multilinestring const * cast_multilinestring() const && = delete;    
    geo_point const * cast_point() const &                      { return cast_t<geo_point>(); }
    geo_polygon const * cast_polygon() const &                  { return cast_t<geo_polygon>(); }
    geo_multipolygon const * cast_multipolygon() const &        { return cast_t<geo_multipolygon>(); }
    geo_linesegment const * cast_linesegment() const &          { return cast_t<geo_linesegment>(); }
    geo_linestring const * cast_linestring() const &            { return cast_t<geo_linestring>(); }
    geo_multilinestring const * cast_multilinestring() const &  { return cast_t<geo_multilinestring>(); }  
    
    size_t numobj() const; // if multipolygon or multilinestring then numobj > 1 else numobj = 0 
    point_access get_subobj(size_t subobj) const && = delete;
    point_access get_subobj(size_t subobj) const &;
    orientation ring_orient(size_t subobj) const;
private:
    spatial_type init_type();
    void init_geography();
    geo_tail const * get_tail() const;
    void swap(geo_mem &);
private:
    using geo_mem_error = sdl_exception_t<geo_mem>;
    using buf_type = std::vector<char>;
    spatial_type m_type = spatial_type::null;
    geo_data const * m_geography = nullptr;
    data_type m_data;
    std::unique_ptr<buf_type> m_buf;
};

inline size_t geo_mem::numobj() const {
    geo_tail const * const tail = get_tail();
    return tail ? tail->size() : 0;
}

inline geo_mem::point_access
geo_mem::get_subobj(size_t const subobj) const & {
    SDL_ASSERT(subobj < numobj());
    return point_access(cast_pointarray(), get_tail(), subobj);
}

using geography_t = vector_mem_range_t; //FIXME: replace by geo_mem ?

} // db
} // sdl

#endif // __SDL_SYSTEM_GEOGRAPHY_H__