// sysiscols.h
//
#pragma once
#ifndef __SDL_SYSOBJ_SYSISCOLS_H__
#define __SDL_SYSOBJ_SYSISCOLS_H__

#include "dataserver/system/page_head.h"

namespace sdl { namespace db {

#pragma pack(push, 1) 

struct sysiscols_row_meta;
struct sysiscols_row_info;

/*System Table: sysiscols (ObjectID = 55)
The sysiscols is a table with only 22 bytes of fixed length data per row, 
and it defines all indexes and statistics in system and user tables, 
both clustered and non-clustered indexes. 
Heaps (index id = 0) are not included in this table. 
The data in this table is visible via the DMV sys.index_columns (indexes only, statistics columns are not included). 
This table is clustered by (idmajor, idminor, subid):
*/
struct sysiscols_row
{
    using meta = sysiscols_row_meta;
    using info = sysiscols_row_info;

    struct data_type { // 26 bytes

        row_head        head;       // 4 bytes
        schobj_id       idmajor;    // (object_id) - 4 bytes - the object_id of the object that the index is defined on
        index_id        idminor;    // 4 bytes - the index id or statistics id for each object_id
        uint32          subid;      // (index_column_id) - 4 bytes - when an index contains multiple columns, this is the order of the columns within the index.For statistics, this appears to always be 1
        iscolstatus     status;     // 4 bytes - bit mask : 0x1 appears to always be set, 0x2 for index, 0x4 for a descending index column(is_descending_key).
        column_id       intprop;    // 4 bytes - appears to the column(syscolpars.colid)
        uint8           tinyprop1;  // 1 byte - appears to be equal to the subid for an index, 0 for a statistic.
        uint8           tinyprop2;  // 1 byte - appears to always be 0
        uint8           tinyprop3;  // 1 byte - appears to always be 0
    };
    union {
        data_type data;
        char raw[sizeof(data_type)];
    };
};

#pragma pack(pop)

struct sysiscols_row_meta: is_static {

    typedef_col_type_n(sysiscols_row, head);
    typedef_col_type_n(sysiscols_row, idmajor);
    typedef_col_type_n(sysiscols_row, idminor);
    typedef_col_type_n(sysiscols_row, subid);
    typedef_col_type_n(sysiscols_row, status);
    typedef_col_type_n(sysiscols_row, intprop);
    typedef_col_type_n(sysiscols_row, tinyprop1);
    typedef_col_type_n(sysiscols_row, tinyprop2);
    typedef_col_type_n(sysiscols_row, tinyprop3);

    typedef TL::Seq<
        head
        ,idmajor
        ,idminor
        ,subid
        ,status
        ,intprop
        ,tinyprop1
        ,tinyprop2
        ,tinyprop3
    >::Type type_list;
};

struct sysiscols_row_info: is_static {
    static std::string type_meta(sysiscols_row const &);
    static std::string type_raw(sysiscols_row const &);
};

} // db
} // sdl

#endif // __SDL_SYSOBJ_SYSISCOLS_H__