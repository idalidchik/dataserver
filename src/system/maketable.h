// maketable.h
//
#ifndef __SDL_SYSTEM_MAKETABLE_H__
#define __SDL_SYSTEM_MAKETABLE_H__

#pragma once

#include "common/type_seq.h"
#include "common/static_type.h"
#include "page_info.h"
#include "database.h"

namespace sdl { namespace db { namespace make {
namespace meta {

    template<bool PK, size_t id = 0, sortorder ord = sortorder::ASC>
    struct key {
        enum { is_primary_key = PK };
        enum { subid = id };
        static const sortorder order = ord;
    };
    using key_true = key<true, 0, sortorder::ASC>;
    using key_false = key<false, 0, sortorder::NONE>;

    template<scalartype::type, int> struct value_type; 
    template<> struct value_type<scalartype::t_int, 4> {
        using type = int;
        enum { fixed = 1 };
    };  
    template<> struct value_type<scalartype::t_bigint, 8> {
        using type = uint64;
        enum { fixed = 1 };
    };
    template<> struct value_type<scalartype::t_smallint, 2> {
        using type = uint16;
        enum { fixed = 1 };
    }; 
    template<> struct value_type<scalartype::t_float, 8> { 
        using type = double;
        enum { fixed = 1 };
    };
    template<> struct value_type<scalartype::t_real, 4> { 
        using type = float;
        enum { fixed = 1 };
    };
    template<> struct value_type<scalartype::t_smalldatetime, 4> { 
        using type = smalldatetime_t;
        enum { fixed = 1 };
    };
    template<> struct value_type<scalartype::t_uniqueidentifier, 16> { 
        using type = guid_t;
        enum { fixed = 1 };
    };
    template<int len> 
    struct value_type<scalartype::t_char, len> {
        using type = char[len];
        enum { fixed = 1 };
    };
    template<int len> 
    struct value_type<scalartype::t_nchar, len> {
        using type = nchar_t[len];
        enum { fixed = 1 };
    };
    template<int len> 
    struct value_type<scalartype::t_varchar, len> {
        using type = var_mem;
        enum { fixed = 0 };
    };
    template<> struct value_type<scalartype::t_geometry, -1> {
        using type = var_mem;
        enum { fixed = 0 };
    };
    template<size_t off, scalartype::type _type, int len, typename base_key = key_false>
    struct col : base_key {
    private:
        using traits = value_type<_type, len>;
        using T = typename traits::type;
    public:
        using val_type = T;
        using ret_type = typename std::conditional<std::is_array<T>::value, T const &, T>::type;
        enum { fixed = traits::fixed };
        enum { offset = off };
        enum { length = len };
        static const char * name() { return ""; }
        static const scalartype::type type = _type;
        static void test() {
            static_assert(!fixed || (std::is_array<T>::value ? 
                (length == sizeof(val_type)/sizeof(typename std::remove_extent<T>::type)) :
                (length == sizeof(val_type))), "col::val_type");
        }
    };

    template <bool v> struct is_fixed { enum { value = v }; };
    template <bool v> struct is_array { enum { value = v }; };

} // meta

template<class META>
class make_base_table: public noncopyable
{
    using type_list = typename META::type_list;
    database * const m_db;
protected:
    explicit make_base_table(database * p) : m_db(p) {
        SDL_ASSERT(m_db);
    }
    ~make_base_table() = default;

    template<class T> // T = col::
    using ret_type = typename T::ret_type;

    template<class T> // T = col::
    static ret_type<T> get_empty(meta::is_array<0>) {
        return typename T::val_type{};
    }
    template<class T> // T = col::
    static ret_type<T> get_empty(meta::is_array<1>) {
        static const typename T::val_type val{};
        return val;
    }
    template<class T> // T = col::
    static ret_type<T> get_empty() {
        return get_empty<T>(meta::is_array<std::is_array<T>::value>());
    }
    template<class T> // T = col::
    ret_type<T> fixed_val(row_head const * const p, meta::is_fixed<1>) const { // is fixed 
        static_assert(T::fixed, "");
        return p->fixed_val<T::val_type>(T::offset);
    }
    template<class T> // T = col::
    ret_type<T> fixed_val(row_head const * const p, meta::is_fixed<0>) const { // is variable 
        static_assert(!T::fixed, "");
        return m_db->get_variable(p, T::offset, T::type);
    }
    template<size_t i> 
    using col_t = typename TL::TypeAt<type_list, i>::Result;

    template<class T> // T = col::
    using col_index = TL::IndexOf<type_list, T>;

    template<class T> // T = col::
    ret_type<T> get_value(row_head const * const p, identity<T>) const {
        if (null_bitmap(p)[col_index<T>::value]) {
            return get_empty<T>();
        }
        return fixed_val<T>(p, meta::is_fixed<T::fixed>());
    }
protected:
    template<class this_table>
    class base_record {
        this_table const * const table;
        row_head const * const row;
    private:
        template<class T> // T = col::
        std::string type_col(meta::is_fixed<1>) const {
            return to_string::type(this->val<T>());
        }
        template<class T> // T = col::
        std::string type_col(meta::is_fixed<0>) const {
            return to_string::dump_mem(this->val<T>());
        }
    protected:
        base_record(this_table const * p, row_head const * h)
            : table(p), row(h) {
            SDL_ASSERT(table && row);
        }
        ~base_record() = default;
    public:
        static size_t size() {
            return META::col_size;
        }
        template<class T> // T = col::
        ret_type<T> val() const {
            return table->get_value(row, identity<T>());
        }
        template<size_t i>
        auto get() const -> decltype(val<col_t<i>>()) {
            static_assert(i < META::col_size, "");
            return this->val<col_t<i>>();
        }
        bool is_null(size_t const i) const {   
            return null_bitmap(row)[i];
        }
        template<size_t i>
        bool is_null() const {
            static_assert(i < META::col_size, "");
            return is_null(i);
        }
        template<class T> // T = col::
        std::string type_col() const {
            return type_col<T>(meta::is_fixed<T::fixed>());
        }
    };
    template<class this_table, class record_type>
    class base_access: noncopyable {
        this_table const * const table;
        datatable _datatable;
        using record_iterator = datatable::record_iterator;
    public:
        using iterator = forward_iterator<base_access, record_iterator>;
        base_access(this_table const * p, database * const d, shared_usertable const & s)
            : table(p), _datatable(d, s)
        {
            SDL_ASSERT(table);
        }
        iterator begin() {
            return iterator(this, _datatable._record.begin());
        }
        iterator end() {
            return iterator(this, _datatable._record.end());
        }
    private:
        friend iterator;
        record_type dereference(record_iterator const & it) {
            return record_type(table, (*it).head());
        }
        void load_next(record_iterator & it) {
            ++it;
        }
        bool is_end(record_iterator const & it) {
            return (it == _datatable._record.end());
        }
    };
};

template<class table, class record>
class make_query
{
    using record_range = std::vector<record>; // prototype
    table & m_table;
public:
    explicit make_query(table * const p) : m_table(*p) {}

    template<class fun_type>
    void scan_if(fun_type fun) {
        for (auto p : m_table) {
            if (!fun(p)) {
                break;
            }
        }
    }
    template<class fun_type> //FIXME: range of tuple<> ?
    record_range select(fun_type fun) {
        record_range ret;
        for (auto & p : m_table) {
            if (fun(p)) {
                ret.push_back(p);
            }
        }
        return ret;
    }
    template<class fun_type>
    std::unique_ptr<record> find(fun_type fun) {
        for (auto & p : m_table) {
            if (fun(p)) {
                return sdl::make_unique<record>(p);
            }
        }
        return nullptr;
    }
};

namespace sample {
struct dbo_META
{
    struct col {
        using Id = meta::col<0, scalartype::t_int, 4, meta::key_true>;
        using Col1 = meta::col<0, scalartype::t_varchar, 255>;
    };
    typedef TL::Seq<
        col::Id
        ,col::Col1
    >::Type type_list;

    enum { col_size = TL::Length<type_list>::value };
    static const char * name() { return ""; }
    static const int32 id = 0;
};
class dbo_table : public dbo_META, public make_base_table<dbo_META>
{
    using base_table = make_base_table<dbo_META>;
    using this_table = dbo_table;
public:
    class record : public base_record<this_table> {
        using base = base_record<this_table>;
    public:
        record(this_table const * p, row_head const * h): base(p, h) {}
        auto Id() const -> col::Id::ret_type { return val<col::Id>(); }
        auto Col1() const -> col::Col1::ret_type { return val<col::Col1>(); }
    };
private:
    using record_access = base_access<this_table, record>;
    using query_type = make_query<this_table, record>;
    record_access _record;
public:
    using iterator = record_access::iterator;
    explicit dbo_table(database * p, shared_usertable const & s)
        : base_table(p), _record(this, p, s)
    {}
    iterator begin() { return _record.begin(); }
    iterator end() { return _record.end(); }
    query_type query{ this };
};
} // sample
} // make
} // db
} // sdl

#endif // __SDL_SYSTEM_MAKETABLE_H__
