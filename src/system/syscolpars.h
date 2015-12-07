// syscolpars.h
//
#ifndef __SDL_SYSTEM_SYSCOLPARS_H__
#define __SDL_SYSTEM_SYSCOLPARS_H__

#pragma once

#include "page_head.h"

namespace sdl { namespace db {

#pragma pack(push, 1) 

// System Table: syscolpars (ObjectID = 41)
// The sys.syscolumns and sys.columns DMVs show data in the syscolpars table, 
// which is a list of every column defined in any table, view, 
// input parameter to a procedure or function, or output column of a table valued function in the database.
// The columns in parenthesis are what the sys.sycolumns view shows.

struct syscolpars_row
{
    struct data_type {
          
        datarow_head head; // 4 bytes

        uint32 id; /*4 bytes - the ObjectID of the table or view that owns this object*/

        /*number (number) - 2 bytes - for functions and procedures,
        this will be 1 for input parameters, it is 0 for output columns of table-valued functions and tables and views*/
        uint16 number;

        /*colid (colid) - 4 bytes - the unique id of the column within this object,
        starting at 1 and counting upward in sequence.  (id, colid, number) is
        unique among all columns in the database.*/
        uint32 colid;

        /*xtype (xtype) - 1 byte - an ID for the data type of this column.
        This references the system table sys.sysscalartypes.xtype.*/
        uint8 xtype;

        /*utype (xusertype) - 4 bytes - usually equal to xtype,
        except for user defined types and tables.  This references
        the system table sys.sysscalartypes.id*/
        uint32 utype;

        /*length (length) - 2 bytes - length of this column in bytes,
        -1 if this is a varchar(max) / text / image data type with no practical maximum length.*/
        uint16 length;
        
        uint8   prec;           // prec (prec) - 1 byte       
        uint8   scale;          // scale (scale) - 1 byte      
        uint32  collationid;    // collationid (collationid) - 4 bytes
        uint32  status;         // status (status) - 4 bytes 
        uint16  maxinrow;       // maxinrow - 2 bytes
        uint32  xmlns;          // xmlns - 4 bytes
        uint32  dflt;           // dflt - 4 bytes
        uint32  chk;            // chk - 4 bytes

        // TODO: RawType.VarBinary("idtval")
        // idtval - variable length?

        /*name (name) - variable length, nvarchar - the name of this column.*/
    };
    union {
        data_type data;
        char raw[sizeof(data_type)];
    };
};

#pragma pack(pop)

struct syscolpars_row_meta {

    typedef_col_type_n(syscolpars_row, head);
    typedef_col_type_n(syscolpars_row, id);
    typedef_col_type_n(syscolpars_row, number);
    typedef_col_type_n(syscolpars_row, colid);
    typedef_col_type_n(syscolpars_row, xtype);
    typedef_col_type_n(syscolpars_row, utype);
    typedef_col_type_n(syscolpars_row, length);
    typedef_col_type_n(syscolpars_row, prec);
    typedef_col_type_n(syscolpars_row, scale);
    typedef_col_type_n(syscolpars_row, collationid);
    typedef_col_type_n(syscolpars_row, status);
    typedef_col_type_n(syscolpars_row, maxinrow);
    typedef_col_type_n(syscolpars_row, xmlns);
    typedef_col_type_n(syscolpars_row, dflt);
    typedef_col_type_n(syscolpars_row, chk);

    typedef TL::Seq<
        head
        ,id
        ,number
        ,colid
        ,xtype
        ,utype
        ,length
        ,prec
        ,scale
        ,collationid
        ,status
        ,maxinrow
        ,xmlns
        ,dflt
        ,chk
    >::Type type_list;

    syscolpars_row_meta() = delete;
};

struct syscolpars_row_info {
    syscolpars_row_info() = delete;
    static std::string type_meta(syscolpars_row const &);
    static std::string type_raw(syscolpars_row const &);
};


} // db
} // sdl

#endif // __SDL_SYSTEM_SYSCOLPARS_H__