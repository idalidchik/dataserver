// usertable.cpp
//
#include "common/common.h"
#include "usertable.h"

#include <algorithm>
#include <sstream>

namespace sdl { namespace db {

usertable::column::column(syscolpars_row const * _colpar,
                          sysscalartypes_row const * _scalar,
                          std::string && _name)
    : colpar(_colpar)
    , scalar(_scalar)
    , name(std::move(_name))
    , type(_scalar->data.id)
    , length(_colpar->data.length)
{
    SDL_ASSERT(colpar && scalar);    
    SDL_ASSERT(colpar->data.utype == scalar->data.id);
    SDL_ASSERT(this->type != scalartype::t_none);
}

bool usertable::column::is_fixed() const
{
    if (scalartype::is_fixed(this->type)) {
        if (!length.is_var()) {
            SDL_ASSERT(length._16 > 0);
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------

usertable::usertable(sysschobjs_row const * p,
                     std::string && n, 
                     columns && c)
    : schobj(p)
    , m_name(std::move(n))
    , m_schema(std::move(c))
{
    SDL_ASSERT(schobj);
    SDL_ASSERT(schobj->is_USER_TABLE_id());

    SDL_ASSERT(!m_name.empty());
    SDL_ASSERT(!m_schema.empty());
    SDL_ASSERT(get_id()._32);

    init_offset();
}

void usertable::init_offset()
{
    size_t offset = 0;
    size_t col_index = 0;
    size_t var_index = 0;
    m_offset.resize(m_schema.size());
    for (auto & c : m_schema) {
        if (c->is_fixed()) {
            m_offset[col_index] = offset;
            offset += c->fixed_size();
        }
        else {
            m_offset[col_index] = var_index++;
        }
        ++col_index;
    }
}

size_t usertable::fixed_offset(size_t i) const
{
    SDL_ASSERT(i < this->size());
    SDL_ASSERT(m_schema[i]->is_fixed());
    return m_offset[i];
}

size_t usertable::var_offset(size_t i) const
{
    SDL_ASSERT(i < this->size());
    SDL_ASSERT(!m_schema[i]->is_fixed());
    return m_offset[i];
}

size_t usertable::count_var() const
{
    return count_if([](column_ref c){
        return !c.is_fixed();
    });
}

size_t usertable::count_fixed() const
{
    return count_if([](column_ref c){
        return c.is_fixed();
    });
}

size_t usertable::fixed_size() const
{
    size_t ret = 0;
    for_col([&ret](column_ref c){
        if (c.is_fixed()) {
            ret += c.fixed_size();
        }
    });
    return ret;
}

std::string usertable::type_schema() const
{
    usertable const & ut = *this;
    std::stringstream ss;
    ss  << "name = " << ut.m_name
        << "\nid = " << ut.get_id()._32
        << std::uppercase << std::hex 
        << " (" << ut.get_id()._32 << ")"
        << std::dec
        << "\nColumns(" << ut.m_schema.size() << ")"
        << "\n";
    for (auto & p : ut.m_schema) {
        column_ref d = *p;
        ss << "[" << d.colpar->data.colid << "] ";
        ss << d.name << " : " << scalartype::get_name(d.type) << " (";
        if (d.length.is_var())
            ss << "var";
        else 
            ss << d.length._16;
        ss << ")";
        if (d.is_fixed()) {
            ss << " fixed";
        }
        ss << "\n";
    }
    return ss.str();
}


} // db
} // sdl

