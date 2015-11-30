// page_head.h
//
#ifndef __SDL_SYSTEM_PAGE_HEAD_H__
#define __SDL_SYSTEM_PAGE_HEAD_H__

#pragma once

#include "common/type_seq.h"
#include "common/static_type.h"

//http://ugts.azurewebsites.net/data/UGTS/document/2/4/46.aspx
//http://www.sqlskills.com/blogs/paul/inside-the-storage-engine-anatomy-of-a-page/

namespace sdl { namespace db {

#pragma pack(push, 1) 

struct pageType // 1 byte
{
    enum T {
        null = 0,
        data = 1,           //1 � data page. This holds data records in a heap or clustered index leaf-level.
        index = 2,          //2 � index page. This holds index records in the upper levels of a clustered index and all levels of non-clustered indexes.
        textmix = 3,        //3 � text mix page. A text page that holds small chunks of LOB values plus internal parts of text tree. These can be shared between LOB values in the same partition of an index or heap.
        texttree = 4,       //4 � text tree page. A text page that holds large chunks of LOB values from a single column value.
        sort = 7,           //7 � sort page. A page that stores intermediate results during a sort operation.
        GAM = 8,            //8 � GAM page. Holds global allocation information about extents in a GAM interval (every data file is split into 4GB chunks � the number of extents that can be represented in a bitmap on a single database page). Basically whether an extent is allocated or not. GAM = Global Allocation Map. The first one is page 2 in each file. More on these in a later post.
        SGAM = 9,           //9 � SGAM page. Holds global allocation information about extents in a GAM interval. Basically whether an extent is available for allocating mixed-pages. SGAM = Shared GAM. the first one is page 3 in each file. More on these in a later post.
        IAM = 10,           //10 � IAM page. Holds allocation information about which extents within a GAM interval are allocated to an index or allocation unit, in SQL Server 2000 and 2005 respectively. IAM = Index Allocation Map. More on these in a later post.
        PFS = 11,           //11 � PFS page. Holds allocation and free space information about pages within a PFS interval (every data file is also split into approx 64MB chunks � the number of pages that can be represented in a byte-map on a single database page. PFS = Page Free Space. The first one is page 1 in each file. More on these in a later post.
        boot = 13,          //13 � boot page. Holds information about the database. There�s only one of these in the database. It�s page 9 in file 1.
        fileheader = 15,    //15 � file header page.Holds information about the file.There�s one per file and it�s page 0 in the file.
        diffmap = 16,       //16 � diff map page. Holds information about which extents in a GAM interval have changed since the last full or differential backup. The first one is page 6 in each file.
        MLmap = 17,         //17 � ML map page.Holds information about which extents in a GAM interval have changed while in bulk - logged mode since the last backup.This is what allows you to switch to bulk - logged mode for bulk - loads and index rebuilds without worrying about breaking a backup chain.The first one is page 7 in each file.
        deallocated = 18,   //18 � a page that�s be deallocated by DBCC CHECKDB during a repair operation.
        temporary = 19,     //19 � the temporary page that ALTER INDEX � REORGANIZE(or DBCC INDEXDEFRAG) uses when working on an index.
        preallocated = 20,  //20 � a page pre - allocated as part of a bulk load operation, which will eventually be formatted as a �real� page.
    };
    uint8 val;
    operator T() const {
        static_assert(sizeof(*this) == 1, "");
        return static_cast<T>(val);
    }
    pageType & operator=(T t) {
        val = static_cast<decltype(val)>(t);
        return *this;
    }
    pageType() = default;
    pageType(T t) { 
        *this = t;
    }
};

struct guid_t // 16 bytes
{
    uint32 a;
    uint16 b;
    uint16 c;
    uint8 d;
    uint8 e;
    uint8 f;
    uint8 g;
    uint8 h;
    uint8 i;
    uint8 j;
    uint8 k;
};

struct pageFileID // 6 bytes
{
    uint32 pageId;  // 4 bytes : PageID
    uint16 fileId;  // 2 bytes : FileID
};

struct pageLSN // 10 bytes
{
    uint32 lsn1;
    uint32 lsn2;
    uint16 lsn3;
};

struct pageXdesID // 6 bytes 
{
    uint32 id2;
    uint16 id1;
};

struct nchar_t // 2 bytes
{
    uint16 c;
};

/*
Datetime Data Type

The datetime data type is a packed byte array which is composed of two integers - the number of days since 1900-01-01 (a signed integer value),
and the number of clock ticks since midnight (where each tick is 1/300th of a second), as explored on this blog and this Microsoft article.
http://www.sql-server-performance.com/2004/datetime-datatype/
https://msdn.microsoft.com/en-us/library/aa175784(v=sql.80).aspx

This gives the interesting result that a zero datetime value with all bytes zero is equal to 1900-01-01 at midnight. 
It also tells us that the datetime structure is a very inefficient way to store time (the datetime2 data type was created to address this concern), 
except that it is excellent at defaulting to a reasonable zero point, and that the date and time parts can be split apart very easily by SQL server.
Note that while it is capable of storing days up to the year plus or minus 58 million, it is limited by rule to only go between 1753-01-01 and 9999-12-31.
And note that while the clock ticks part is a 32-bit number, in practice the highest value used will be 25919999.
Since the datatime clock ticks are 1/300ths of a second, while they display accuracy to the millisecond, 
the will actually be rounded to the nearest 0, 3, 7, or 10 millisecond boundary in all conversions and comparisons.
*/
struct datetime_t // 8 bytes
{
    uint32 t;   // clock ticks since midnight (where each tick is 1/300th of a second)
    int32 d;    // days since 1900-01-01

    enum { u_date_diff = 25567 }; // = SELECT DATEDIFF(d, '19000101', '19700101');

    // convert to number of seconds that have elapsed since 00:00:00 UTC, 1 January 1970
    static size_t get_unix_time(datetime_t const & src);
    size_t get_unix_time() const {
        return get_unix_time(*this);
    }
    static datetime_t set_unix_time(size_t);

    bool is_null() const {
        return !d && !t;
    }
    bool is_valid() const {
        return d >= u_date_diff;
    }
};

struct page_head // 96 bytes page header
{
    enum { page_size = kilobyte<8>::value }; // A database file at its simplest level is an array of 8KB pages (8192 bytes)
    enum { head_size = 96 };
    enum { body_size = page_size - head_size }; // 8096 bytes

    struct data_type { // IS_LITTLE_ENDIAN
        uint8       headerVersion;  //0x00 : Header Version(m_headerVersion) - 1 byte - 0x01 for SQL Server up to 2008 R2
        pageType    type;           //0x01 : PageType(m_type) - 1 byte - as described above, this will be 0x01 for data pages, or 0x02 for index pages.
        uint8       typeFlagBits;   //0x02 : TypeFlagBits(m_typeFlagBits) - 1 byte - for PFS pages, this will be 1 in any of the pages in the interval have ghost records.For all other page types, this field is ignored.
        uint8       level;          //0x03 : Level(m_level) - 1 byte - for a B - Tree, this is the level in the tree, with 0 being the leaf nodes.For a heap(which is just a flat list), this value is ignored.
        uint16      flagBits;       //0x04 : FlagBits(m_flagBits) - 2 bytes - various page flags : 0x200 means the page has a page checksum stored in the TornBits field.  0x100 means torn page protection is on, and has detected an error.
        uint16      indexId;        //0x06 : IndexID(m_indexId(AllocUnitId.idInd)) - 2 bytes - the idInd member on the allocation unit(similar to index_id on sysindexes, but not the same, see the section on allocation units for details)
        pageFileID  prevPage;       //0x08 : PrevPage - 6 bytes - PageID : FileID of the previous page at the same level in this B - tree, 0 : 0 if this is the first page.
        uint16      pminlen;        //0x0E : PMinLen(pminlen) - 2 bytes - the size in bytes of the fixed length part of the data records on this page.
        pageFileID  nextPage;       //0x10 : NextPage - 6 bytes - PageID : FileID of the next page at the same level in this B - tree, 0 : 0 if this is the last page.
        uint16      slotCnt;        //0x16 : SlotCount - 2 bytes - the number of entries in the slot array(though some of them may be deallocated).
        uint32      objId;          //0x18 : ObjectID(m_objId(AllocUnitId.idObj)) - 4 bytes - in SQL 2000 and earlier, this held the ObjectID.In SQL 2005 and higher, this holds the idObj member of an allocation unit ID, which is not usually the same as sys.objects.object_id.For system tables, it this is the same, but for user defined tables, idObj continues to be sequentially assigned, whereas object_id is a random 32 - bit number.See the section on allocation units for details.
        uint16      freeCnt;        //0x1C : FreeCount(m_freeCnt) - 2 bytes - the total number of bytes of free space on this page, not necessarily contiguous.
        uint16      freeData;       //0x1E : FreeData(m_freeData) - 2 bytes - the offset to the next available position to store row data on this page.
        pageFileID  pageId;         //0x20 : ThisPage(m_pageId) - 6 bytes - PageID : FileID of this page - not really necessary because the position in the file determines this, but helps to detect database corruption.
        uint16      reservedCnt;    //0x26 : ReservedCount(m_reservedCnt) - 2 bytes - the number of bytes that have been reserved by open transactions to allow for rollback and to prevent that space for being used for any other purpose.
        pageLSN     lsn;            //0x28 : LSN(m_lsn) - 10 bytes - the LSN of the last log record that changed this page.
        uint16      xactReserved;   //0x32 : XactReserved(m_xactReserved) - 2 bytes - the amount that was last added to m_reservedCnt.
        pageXdesID  xdesId;         //0x34 : TransactionID - XdesID - 6 bytes - the ID of the last transaction that affected ReservedCount.
        uint16      ghostRecCnt;    //0x3A : GhostRecord(m_ghostRecCnt) - 2 bytes - the number of ghost records on this page.
        int32       tornBits;       //0x3C : TornBits(m_tornBits) - 4 bytes - this will either hold a page or torn bits checksum, which is used to check for corrupted pages due to interrupted disk I / O operations.This is described in greater detail later on.
        uint8       reserved[32];   //0x40 : Reserved - 32 bytes - these don't appear to be used for anything, and are usually zero.
    };
    union {
        data_type data;
        char raw[head_size];
    };
    bool is_null() const {
        return pageType::null == data.type;
    }
};

#pragma pack(pop)

template<class T>
inline T const * page_body(page_head const * p) {
    if (p) {
        A_STATIC_ASSERT_IS_POD(T);
        static_assert(sizeof(T) <= page_head::body_size, "");
        char const * body = ((char const *)p) + page_head::head_size;
        return (T const *)body;
    }
    SDL_ASSERT(0);
    return nullptr;
}

// At the end of page is a slot array of 2-byte values, 
// each holding the offset to the start of the record. 
// The slot array grows backwards as records are added.
class slot_array {
    page_head const * const head;
public:
    explicit slot_array(page_head const * h) : head(h){}
    size_t size() const {
        if (head)
            return head->data.slotCnt;
        return 0;
    }
    uint16 operator[](size_t i) const;
    std::vector<uint16> copy() const;
};

namespace meta {
    template<size_t _offset, class T>
    struct col_type {
        enum { offset = _offset };
        typedef T type;
    };
}

#define typedef_col_type(pagetype, member) \
    typedef meta::col_type<offsetof(pagetype, data.member), decltype(pagetype().data.member)> member

struct page_header_meta {

    typedef_col_type(page_head, headerVersion);
    typedef_col_type(page_head, type);
    typedef_col_type(page_head, typeFlagBits);
    typedef_col_type(page_head, level);
    typedef_col_type(page_head, flagBits);
    typedef_col_type(page_head, indexId);
    typedef_col_type(page_head, prevPage);
    typedef_col_type(page_head, pminlen);
    typedef_col_type(page_head, nextPage);
    typedef_col_type(page_head, slotCnt);
    typedef_col_type(page_head, objId);
    typedef_col_type(page_head, freeCnt);
    typedef_col_type(page_head, freeData);
    typedef_col_type(page_head, pageId);
    typedef_col_type(page_head, reservedCnt);
    typedef_col_type(page_head, lsn);
    typedef_col_type(page_head, xactReserved);
    typedef_col_type(page_head, xdesId);
    typedef_col_type(page_head, ghostRecCnt);
    typedef_col_type(page_head, tornBits);

    typedef TL::Seq<
        headerVersion
        ,type
        ,typeFlagBits
        ,level
        , flagBits
        , indexId
        , prevPage
        , pminlen
        , nextPage
        , slotCnt
        , objId
        , freeCnt
        , freeData
        , pageId
        , reservedCnt
        , lsn
        , xactReserved
        , xdesId
        , ghostRecCnt
        ,tornBits
    >::Type type_list;
};

} // db
} // sdl

#endif // __SDL_SYSTEM_PAGE_HEAD_H__