// database.cpp
//
#include "common/common.h"
#include "database.h"
#include "file_map.h"

#include <algorithm>
#include <sstream>

namespace sdl { namespace db {

//----------------------------------------------------------------------------

tablecolumn::tablecolumn(syscolpars_row const * _colpar,
                         sysscalartypes_row const * _scalar,
                         const std::string & _name)
    : colpar(_colpar)
    , scalar(_scalar)
    , m_data(_name)
{
    SDL_ASSERT(colpar);
    SDL_ASSERT(scalar);    
    SDL_ASSERT(colpar->data.utype == scalar->data.id);

    A_STATIC_SAME_TYPE(colpar->data.utype, scalar->data.id);
    A_STATIC_SAME_TYPE(m_data.length, colpar->data.length);

    m_data.length = colpar->data.length;
    m_data.type = scalar->id_scalartype();
}

//----------------------------------------------------------------------------

tableschema::tableschema(sysschobjs_row const * p)
    : schobj(p)
{
    SDL_ASSERT(schobj);
    SDL_ASSERT(schobj->is_USER_TABLE_id());
}

void tableschema::push_back(std::unique_ptr<tablecolumn> p)
{
    SDL_ASSERT(p);
    m_cols.push_back(std::move(p));
}

//----------------------------------------------------------------------------

usertable::usertable(sysschobjs_row const * p, const std::string & _name)
    : m_sch(p)
    , m_name(_name)
{
    SDL_ASSERT(!m_name.empty());
}

void usertable::push_back(std::unique_ptr<tablecolumn> p)
{
    SDL_ASSERT(p);
    m_sch.push_back(std::move(p));
}

std::string usertable::type_sch(usertable const & ut)
{
    auto & cols = ut.sch().cols();
    std::stringstream ss;
    ss  << "name = " << ut.name()
        << "\nid = " << ut.get_id()
        << "\nColumns(" << cols.size() << ")"
        << "\n";
    for (auto & p : cols) {
        auto const & d = p->data();
        ss << d.name << " : "
            << scalartype_info::type(d.type)
            << " (";
        if (d.is_varlength())
            ss << "var";
        else 
            ss << d.length;
        ss << ")\n";
    }
    return ss.str();
}

//----------------------------------------------------------------------------

class database::data_t : noncopyable
{
    enum { page_size = page_head::page_size };
public:
    const std::string filename;
    explicit data_t(const std::string & fname);

    bool is_open() const
    {
        return m_fmap.IsFileMapped();
    }
    void const * start_address() const
    {
        return m_fmap.GetFileView();
    }
    size_t page_count() const
    {   
        return m_pageCount;
    }
    page_head const * load_page(pageIndex) const;
    page_head const * load_page(pageFileID const &) const;
private:
    size_t m_pageCount;
    FileMapping m_fmap;
};

database::data_t::data_t(const std::string & fname)
    : filename(fname)
    , m_pageCount(0)
{
    static_assert(page_size == 8 * 1024, "");
    if (m_fmap.CreateMapView(fname.c_str())) {
        uint64 sz = m_fmap.GetFileSize();
        uint64 pp = sz / page_size;
        SDL_ASSERT(!(sz % page_size));
        SDL_ASSERT(pp < size_t(-1));
        m_pageCount = static_cast<size_t>(pp);
    }
    else {
        SDL_WARNING(false);
    }
}

page_head const *
database::data_t::load_page(pageIndex const i) const
{
    const size_t pageIndex = i.value();
    if (pageIndex < m_pageCount) {
        const char * const data = static_cast<const char *>(m_fmap.GetFileView());
        const char * p = data + pageIndex * page_size;
        return reinterpret_cast<page_head const *>(p);
    }
    SDL_ASSERT(0);
    return nullptr;
}

page_head const *
database::data_t::load_page(pageFileID const & id) const
{
    if (id.is_null()) {
        return nullptr;
    }
    return load_page(pageIndex(id.pageId));
}

database::database(const std::string & fname)
    : m_data(sdl::make_unique<data_t>(fname))
    //, m_sysallocunits(*this)
{
}

database::~database()
{
}

const std::string & database::filename() const
{
    return m_data->filename;
}

bool database::is_open() const
{
    return m_data->is_open();
}

void const * database::start_address() const
{
    return m_data->start_address();
}

void const * database::memory_offset(void const * p) const
{
    char const * p1 = (char const *)start_address();
    char const * p2 = (char const *)p;
    SDL_ASSERT(p2 >= p1);
    void const * offset = reinterpret_cast<void const *>(p2 - p1);    
    return offset;
}

size_t database::page_count() const
{
    return m_data->page_count();
}

page_head const *
database::load_page(pageIndex i)
{
    return m_data->load_page(i);
}

page_head const *
database::load_page(pageFileID const & id)
{
    return m_data->load_page(id);
}

page_head const * 
database::load_page(sysPage i)
{
    return load_page(static_cast<pageIndex::value_type>(i));
}

std::vector<page_head const *>
database::load_page_list(page_head const * p)
{
    std::vector<page_head const *> vec;
    if (p) {
        auto prev = p;
        while ((prev = load_prev(prev)) != nullptr) {
            vec.push_back(prev);
        }
        std::reverse(vec.begin(), vec.end());
        vec.push_back(p);
        while ((p = load_next(p)) != nullptr) {
            vec.push_back(p);
        }
        vec.shrink_to_fit();
    }
    return vec;
}

database::page_ptr<bootpage>
database::get_bootpage()
{
    page_head const * const h = load_page(sysPage::boot_page);
    if (h) {
        return make_unique<bootpage>(h, cast::page_body<bootpage_row>(h));
    }
    return std::unique_ptr<bootpage>();
}

database::page_ptr<datapage>
database::get_datapage(pageIndex i)
{
    page_head const * const h = load_page(i);
    if (h) {
        return make_unique<datapage>(h);
    }
    return std::unique_ptr<datapage>();
}

database::page_ptr<fileheader>
database::get_fileheader()
{
    page_head const * const h = load_page(0);
    if (h) {
        return make_unique<fileheader>(h);
    }
    return std::unique_ptr<fileheader>();
}

database::page_ptr<sysallocunits>
database::get_sysallocunits()
{
    auto boot = get_bootpage();
    if (boot) {
        auto & id = boot->row->data.dbi_firstSysIndexes;
        page_head const * const h = m_data->load_page(id);
        if (h) {
            return make_unique<sysallocunits>(h);
        }
    }
    return std::unique_ptr<sysallocunits>();
}

page_head const *
database::load_sys_obj(sysallocunits const * p, const sysObj id)
{
    if (p) {
        if (auto row = p->find_auid(static_cast<uint32>(id))) {
            return load_page(row->data.pgfirst);
        }
    }
    return nullptr;
}

//-----------------------------------------------------------------------

template<class T, database::sysObj id> 
database::page_ptr<T>
database::get_sys_obj(sysallocunits const * p)
{
    if (auto h = load_sys_obj(p, id)) {
        return std::make_shared<T>(h);
    }
    return page_ptr<T>();
}

template<class T, database::sysObj id> 
database::page_ptr<T>
database::get_sys_obj()
{
    return get_sys_obj<T, id>(get_sysallocunits().get());
}

template<class T> 
database::vector_page_ptr<T>
database::get_sys_list(page_ptr<T> && p)
{
    vector_page_ptr<T> vec;
    if (p) {
        auto page_head_list = load_page_list(p->head);
        if (!page_head_list.empty()) {
            vec.reserve(page_head_list.size());
            for (auto h : page_head_list) {
                vec.push_back(std::make_shared<T>(h));
            }
        }
    }
    return vec;
}

template<class T, database::sysObj id> 
database::vector_page_ptr<T>
database::get_sys_list()
{
    return get_sys_list(get_sys_obj<T, id>());
}

//-----------------------------------------------------------------------

database::vector_page_ptr<sysallocunits>
database::get_sysallocunits_list()
{
    return get_sys_list(get_sysallocunits());
}

database::page_ptr<sysschobjs>
database::get_sysschobjs()
{
    return get_sys_obj<sysschobjs, sysObj::sysschobjs>();
}

database::vector_page_ptr<sysschobjs>
database::get_sysschobjs_list()
{
    return get_sys_list<sysschobjs, sysObj::sysschobjs>();
}

database::page_ptr<syscolpars>
database::get_syscolpars()
{
    return get_sys_obj<syscolpars, sysObj::syscolpars>();
}

database::vector_page_ptr<syscolpars>
database::get_syscolpars_list()
{
    return get_sys_list<syscolpars, sysObj::syscolpars>();
}

database::page_ptr<sysidxstats>
database::get_sysidxstats()
{
    return get_sys_obj<sysidxstats, sysObj::sysidxstats>();
}

database::vector_page_ptr<sysidxstats>
database::get_sysidxstats_list()
{
    return get_sys_list<sysidxstats, sysObj::sysidxstats>();
}

database::page_ptr<sysscalartypes>
database::get_sysscalartypes()
{
    return get_sys_obj<sysscalartypes, sysObj::sysscalartypes>();
}

database::vector_page_ptr<sysscalartypes>
database::get_sysscalartypes_list()
{
    return get_sys_list<sysscalartypes, sysObj::sysscalartypes>();
}

database::page_ptr<sysobjvalues>
database::get_sysobjvalues()
{
    return get_sys_obj<sysobjvalues, sysObj::sysobjvalues>();
}

database::vector_page_ptr<sysobjvalues>
database::get_sysobjvalues_list()
{
    return get_sys_list<sysobjvalues, sysObj::sysobjvalues>();
}

database::page_ptr<sysiscols>
database::get_sysiscols()
{
    return get_sys_obj<sysiscols, sysObj::sysiscols>();
}

database::vector_page_ptr<sysiscols>
database::get_sysiscols_list()
{
    return get_sys_list<sysiscols, sysObj::sysiscols>();
}

//---------------------------------------------------------

page_head const * database::load_next(page_head const * p)
{
    if (p) {
        return m_data->load_page(p->data.nextPage);
    }
    return nullptr;
}

page_head const * database::load_prev(page_head const * p)
{
    if (p) {
        return m_data->load_page(p->data.prevPage);
    }
    return nullptr;
}

//---------------------------------------------------------
#if 0
database::page_access::value_type
database::page_access::load_first()
{
    return parent.get_sysallocunits();
}

void database::page_access::load_next(value_type & p)
{
    SDL_ASSERT(p);
    if (p) {
        A_STATIC_CHECK_TYPE(page_head const * const, p->head);
        if (auto h = parent.load_next(p->head)) {
            A_STATIC_CHECK_TYPE(page_head const *, h);
            p = std::make_shared<page_type>(h);
        }
        else {
            p.reset();
        }
    }
}

void database::page_access::load_prev(value_type & p)
{
    SDL_ASSERT(p);
    if (p) {
        A_STATIC_CHECK_TYPE(page_head const * const, p->head);
        if (auto h = parent.load_prev(p->head)) {
            A_STATIC_CHECK_TYPE(page_head const *, h);
            p = std::make_shared<page_type>(h);
        }
        else {
            p.reset();
        }
    }
    p.reset();
}
#endif
//---------------------------------------------------------

database::vector_usertable
database::get_usertables()
{
    vector_usertable ret;

    auto colpar_list = get_syscolpars_list();
    auto scalar_list = get_sysscalartypes_list();

    SDL_ASSERT(!colpar_list.empty());
    SDL_ASSERT(!scalar_list.empty());

    for_USER_TABLE([&](sysschobjs_row const * const schobj_row)
    {        
        auto utable = sdl::make_unique<usertable>(schobj_row, schobj_row->col_name());
        usertable & ut = *utable.get();
        {
            SDL_ASSERT(schobj_row->data.id == ut.get_id());
            for (auto & colpar : colpar_list) {
                colpar->for_row([&ut, &scalar_list](syscolpars_row const * const colpar_row) {
                    if (colpar_row->data.id == ut.get_id()) {
                        auto const utype = colpar_row->data.utype;
                        for (auto & scalar : scalar_list) {
                            auto const s = scalar->find_if([utype](sysscalartypes_row const * const p) {
                                return (p->data.id == utype);
                            });
                            if (s) {
                                ut.push_back(sdl::make_unique<tablecolumn>(colpar_row, s,
                                    colpar_row->col_name()));
                            }
                        }
                    }
                });
            }
            SDL_ASSERT(ut.sch().cols().size());
            ret.push_back(std::move(utable));
        }
    });
    using table_type = vector_usertable::value_type;
    std::sort(ret.begin(), ret.end(),
        [](table_type const & x, table_type const & y){
        return x->name() < y->name();
    });
    return ret;
}

} // db
} // sdl

#if SDL_DEBUG
namespace sdl {
    namespace db {
        namespace {
            class unit_test {
            public:
                unit_test()
                {
                    SDL_TRACE(__FILE__);
                }
            };
            static unit_test s_test;
        }
    } // db
} // sdl
#endif //#if SV_DEBUG

#if 0
------------------------------------------------------
PFS_page
The first PFS page is at *:1 in each database file, 
and it stores the statistics for pages 0-8087 in that database file. 
There will be one PFS page for just about every 64MB of file size (8088 bytes * 8KB per page).
A large database file will use a long chain of PFS pages, linked together using the LastPage and NextPage pointers in the 96 byte header. 
------------------------------------------------------
linked list of pages using the PrevPage, ThisPage, and NextPage page locators, when one page is not enough to hold all the data.
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
#endif

