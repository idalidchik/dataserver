// boot_page.cpp
//
#include "common/common.h"
#include "boot_page.h"
#include "page_info.h"
#include <sstream>

namespace sdl { namespace db {

static_col_name(bootpage_row_meta, dbi_version);
static_col_name(bootpage_row_meta, dbi_createVersion);
static_col_name(bootpage_row_meta, dbi_status);
static_col_name(bootpage_row_meta, dbi_nextid);
static_col_name(bootpage_row_meta, dbi_crdate);
static_col_name(bootpage_row_meta, dbi_dbname);
static_col_name(bootpage_row_meta, dbi_dbid);
static_col_name(bootpage_row_meta, dbi_maxDbTimestamp);
static_col_name(bootpage_row_meta, dbi_checkptLSN);
static_col_name(bootpage_row_meta, dbi_differentialBaseLSN);
static_col_name(bootpage_row_meta, dbi_dbccFlags);
static_col_name(bootpage_row_meta, dbi_collation);
static_col_name(bootpage_row_meta, dbi_familyGuid);
static_col_name(bootpage_row_meta, dbi_maxLogSpaceUsed);
static_col_name(bootpage_row_meta, dbi_recoveryForkNameStack);
static_col_name(bootpage_row_meta, dbi_differentialBaseGuid);
static_col_name(bootpage_row_meta, dbi_firstSysIndexes);
static_col_name(bootpage_row_meta, dbi_createVersion2);
static_col_name(bootpage_row_meta, dbi_versionChangeLSN);
static_col_name(bootpage_row_meta, dbi_LogBackupChainOrigin);
static_col_name(bootpage_row_meta, dbi_modDate);
static_col_name(bootpage_row_meta, dbi_verPriv);
static_col_name(bootpage_row_meta, dbi_svcBrokerGUID);
static_col_name(bootpage_row_meta, dbi_AuIdNext);
//
static_col_name(file_header_row_meta, NumberFields);

std::string boot_info::type_raw(bootpage_row const & b)
{
    return to_string::type_raw(b.raw);
}

std::string boot_info::type(bootpage_row const & b) //FIXME: will be replaced by boot_info::type_meta
{
    const auto & d = b.data;
    typedef to_string S;
    std::stringstream ss;
    ss
        << "\ndbi_version = " << d.dbi_version
        << "\ndbi_createVersion = " << d.dbi_createVersion
        << "\ndbi_status = " << d.dbi_status
        << "\ndbi_nextid = " << d.dbi_nextid
        << "\ndbi_crdate = " << S::type(d.dbi_crdate)
        << "\ndbi_dbname = " << S::type(d.dbi_dbname)
        << "\ndbi_dbid = " << d.dbi_dbid
        << "\ndbi_maxDbTimestamp = " << d.dbi_maxDbTimestamp
        << "\ndbi_checkptLSN = " << S::type(d.dbi_checkptLSN)
        << "\ndbi_differentialBaseLSN = " << S::type(d.dbi_differentialBaseLSN)
        << "\ndbi_dbccFlags = " << d.dbi_dbccFlags
        << "\ndbi_collation = " << d.dbi_collation
        << "\ndbi_familyGuid = " << S::type(d.dbi_familyGuid)
        << "\ndbi_maxLogSpaceUsed = " << d.dbi_maxLogSpaceUsed
        << "\ndbi_recoveryForkNameStack = ?"
        << "\ndbi_differentialBaseGuid = " << S::type(d.dbi_differentialBaseGuid)
        << "\ndbi_firstSysIndexes = " << S::type(d.dbi_firstSysIndexes)
        << "\ndbi_createVersion2 = " << d.dbi_createVersion2
        << "\ndbi_versionChangeLSN = " << S::type(d.dbi_versionChangeLSN)
        << "\ndbi_LogBackupChainOrigin = " << S::type(d.dbi_LogBackupChainOrigin)
        << "\ndbi_modDate = " << S::type(d.dbi_modDate)
        << "\ndbi_verPriv = " << d.dbi_verPriv
        << "\ndbi_svcBrokerGUID = " << S::type(d.dbi_svcBrokerGUID)
        << "\ndbi_AuIdNext = " << d.dbi_AuIdNext
    << std::endl;
    auto s = ss.str();
    return s;
}

std::string boot_info::type_meta(bootpage_row const & b)
{
    std::stringstream ss;
    impl::processor<bootpage_row_meta::type_list>::print(ss, &b);
    return ss.str();
}

std::string file_header_row_info::type(file_header_row const & row)
{
    std::stringstream ss;
    impl::processor<file_header_row_meta::type_list>::print(ss, &row);
    return ss.str();
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
                    
                    A_STATIC_ASSERT_IS_POD(bootpage_row);
                    A_STATIC_ASSERT_IS_POD(file_header_row);
                    
                    static_assert(sizeof(bootpage_row().data.dbi_dbname) == 256, "");
                    static_assert(sizeof(recovery_t) == 28, "");
                    static_assert(sizeof(bootpage_row().raw) > sizeof(bootpage_row().data), "");

                    //--------------------------------------------------------------
                    // http://stackoverflow.com/questions/21201888/how-to-make-wchar-t-16-bit-with-clang-for-linux-x64
                    // static_assert(sizeof(wchar_t) == 2, "wchar_t"); Note. differs on 64-bit Clang 
                    static_assert(offsetof(bootpage_row, data._0x00) == 0x00, "");
                    static_assert(offsetof(bootpage_row, data._0x08) == 0x08, "");
                    static_assert(offsetof(bootpage_row, data.dbi_status) == 0x24, "");
                    static_assert(offsetof(bootpage_row, data.dbi_nextid) == 0x28, "");
                    static_assert(offsetof(bootpage_row, data.dbi_crdate) == 0x2C, "");
                    static_assert(offsetof(bootpage_row, data.dbi_dbname) == 0x34, "");
                    static_assert(offsetof(bootpage_row, data._0x134) == 0x134, "");
                    static_assert(offsetof(bootpage_row, data._0x13A) == 0x13A, "");
                    static_assert(offsetof(bootpage_row, data._0x140) == 0x140, "");
                    static_assert(offsetof(bootpage_row, data._0x15A) == 0x15A, "");
                    static_assert(offsetof(bootpage_row, data._0x168) == 0x168, "");
                    static_assert(offsetof(bootpage_row, data._0x18C) == 0x18C, "");
                    static_assert(offsetof(bootpage_row, data._0x1AC) == 0x1AC, "");
                    static_assert(offsetof(bootpage_row, data._0x20C) == 0x20C, "");
                    static_assert(offsetof(bootpage_row, data._0x222) == 0x222, "");
                    static_assert(offsetof(bootpage_row, data._0x28A) == 0x28A, "");
                    static_assert(offsetof(bootpage_row, data._0x2B0) == 0x2B0, "");
                    static_assert(offsetof(bootpage_row, data._0x2C4) == 0x2C4, "");
                    static_assert(offsetof(bootpage_row, data._0x2E8) == 0x2E8, "");
                    //--------------------------------------------------------------
                    //SDL_TRACE_2("sizeof(bootpage_row::data_type) = ", sizeof(bootpage_row::data_type));
                    //SDL_TRACE_2("sizeof(bootpage_row) = ", sizeof(bootpage_row));

                    static_assert(offsetof(file_header_row, data.NumberFields) == 0x10, "");
                    static_assert(offsetof(file_header_row, data.FieldEndOffsets) == 0x12, "");
                }
            };
            static unit_test s_test;
        }
    } // db
} // sdl
#endif //#if SV_DEBUG



