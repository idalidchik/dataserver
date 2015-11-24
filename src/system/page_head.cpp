// page_head.cpp
//
#include "stdafx.h"
#include "page_head.h"

namespace sdl {	namespace db {

	const char * page_info::type(pageType const t)
	{
		switch (t) {
		case pageType::null: return "null";
		case pageType::data: return "data";
		case pageType::index: return "index";
		case pageType::textmix: return "textmix";
		case pageType::texttree: return "texttree";
		case pageType::sort: return "sort";
		case pageType::GAM: return "GAM";
		case pageType::SGAM: return "SGAM";
		case pageType::IAM: return "IAM";
		case pageType::PFS: return "PFS";
		case pageType::boot: return "boot";
		case pageType::fileheader: return "fileheader";
		case pageType::diffmap: return "diffmap";
		case pageType::MLmap: return "MLmap";
		case pageType::deallocated: return "deallocated";
		case pageType::temporary: return "temporary";
		case pageType::preallocated: return "preallocated";
		default:
			return ""; // unknown type
		}
	}
	std::string page_info::type(page_header const & p, pageId_t const t)
	{
		enum { page_size = page_header::page_size };
		const size_t pageId = t.value();
		const auto & d = p.data;
		std::stringstream ss;
		char buf[128] = {};
		const size_t page_beg = size_t(pageId * page_size);
		const size_t page_end = page_beg + page_size;
		ss
			<< "\npageId = " << pageId
			<< "\nrange = " << format_s(buf, "(%x:%x)", page_beg, page_end)
			<< "\nheaderVersion = " << int(d.headerVersion)
			<< "\ntype = " << type(d.type) << " (" << int(d.type) << ")"
			<< "\ntypeFlagBits = " << int(d.typeFlagBits)
			<< "\nlevel = " << int(d.level)
			<< "\nflagBits = " << d.flagBits
			<< "\nindexId = " << d.indexId
			<< "\nprevPage = (" << d.prevPage.fileId << ":" << d.prevPage.pageId << ")"
			<< "\npminlen = " << d.pminlen
			<< "\nnextPage = (" << d.nextPage.fileId << ":" << d.nextPage.pageId << ")"
			<< "\nslotCnt = " << d.slotCnt
			<< "\nobjId = " << d.objId
			<< "\nfreeCnt = " << d.freeCnt
			<< "\nfreeData = " << d.freeData
			<< "\npageId = (" << d.pageId.fileId << ":" << d.pageId.pageId << ")"
			<< "\nreservedCnt = " << d.reservedCnt
			<< "\nlsn = (" << d.lsn.lsn1 << ":" << d.lsn.lsn2 << ":" << d.lsn.lsn3 << ")" // (32:120:1)
			<< "\nxactReserved = " << d.xactReserved
			<< "\nxdesId = (" << d.xdesId.id1 << ":" << d.xdesId.id2 << ")" // (0:357)
			<< "\nghostRecCnt = " << d.ghostRecCnt
			<< "\ntornBits = " << int(d.tornBits) // compatible with DBCC PAGE
			<< std::endl;
		auto s = ss.str();
		return s;
	}

	std::string page_info::type_raw(char const * buf, size_t const buf_size, const bool show_address)
	{
		SDL_ASSERT(buf_size);
		char xbuf[128] = {};
		std::stringstream ss;
		for (size_t i = 0; i < buf_size; ++i) {
			if (show_address) {
				if (!(i % 20)) {
					ss << "\n";
					ss << format_s(xbuf, "%016X: ", i);
				}
				if (!(i % 4)) {
					ss << " ";
				}
			}
			auto n = reinterpret_cast<uint8 const&>(buf[i]);
			ss << format_s(xbuf, "%02X", int(n));
		}
		ss << std::endl;
		return ss.str();
	}

	std::string page_info::type(guid_t const & g)
	{
		char buf[128] = {};
		std::stringstream ss;
		ss << format_s(buf, "%x", uint32(g.a)) << "-";
		ss << format_s(buf, "%x", uint32(g.b)) << "-";
		ss << format_s(buf, "%x", uint32(g.c)) << "-";
		ss << format_s(buf, "%x", uint32(g.d));
		ss << format_s(buf, "%x", uint32(g.e)) << "-";;
		ss << format_s(buf, "%x", uint32(g.f));
		ss << format_s(buf, "%x", uint32(g.g));
		ss << format_s(buf, "%x", uint32(g.h));
		ss << format_s(buf, "%x", uint32(g.i));
		ss << format_s(buf, "%x", uint32(g.j));
		ss << format_s(buf, "%x", uint32(g.k));
		auto s = ss.str();
		return s;
	}
	std::string page_info::type(pageLSN const & d)
	{
		std::stringstream ss;
		ss << d.lsn1 << ":" << d.lsn2 << ":" << d.lsn3;
		return ss.str();
	}

	std::string page_info::type(pageFileID const & d)
	{
		std::stringstream ss;
		ss << d.fileId << ":" << d.pageId;
		return ss.str();
	}

	std::string page_info::type(wchar_t const * p, const size_t buf_size)
	{
		wchar_t const * const end = p + buf_size;
		std::string s;
		s.reserve(buf_size);
		for (; p != end; ++p) {
			char c = *reinterpret_cast<char const *>(p);
			if (c)
				s += c;
		}
		return s;
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

					SDL_ASSERT(IS_LITTLE_ENDIAN);
					A_STATIC_ASSERT(sizeof(uint8) == 1);
					A_STATIC_ASSERT(sizeof(int16) == 2);
					A_STATIC_ASSERT(sizeof(uint16) == 2);
					A_STATIC_ASSERT(sizeof(int32) == 4);
					A_STATIC_ASSERT(sizeof(uint32) == 4);
					A_STATIC_ASSERT(sizeof(int64) == 8);
					A_STATIC_ASSERT(sizeof(uint64) == 8);

					A_STATIC_ASSERT_IS_POD(page_header);
					A_STATIC_ASSERT(sizeof(page_header) == 96);
					A_STATIC_ASSERT(sizeof(pageFileID) == 6);
					A_STATIC_ASSERT(sizeof(pageLSN) == 10);
					A_STATIC_ASSERT(sizeof(pageXdesID) == 6);
					A_STATIC_ASSERT(sizeof(guid_t) == 16);
					A_STATIC_ASSERT_IS_POD(guid_t);

					A_STATIC_ASSERT(page_header::page_size == 8 * 1024);
					A_STATIC_ASSERT(page_header::body_size == 8 * 1024 - 96);

					A_STATIC_ASSERT(offsetof(page_header, data.headerVersion) == 0);
					A_STATIC_ASSERT(offsetof(page_header, data.type) == 0x01);
					A_STATIC_ASSERT(offsetof(page_header, data.typeFlagBits) == 0x02);
					A_STATIC_ASSERT(offsetof(page_header, data.level) == 0x03);
					A_STATIC_ASSERT(offsetof(page_header, data.flagBits) == 0x04);
					A_STATIC_ASSERT(offsetof(page_header, data.indexId) == 0x06);
					A_STATIC_ASSERT(offsetof(page_header, data.prevPage) == 0x08);
					A_STATIC_ASSERT(offsetof(page_header, data.pminlen) == 0x0E);
					A_STATIC_ASSERT(offsetof(page_header, data.nextPage) == 0x10);
					A_STATIC_ASSERT(offsetof(page_header, data.slotCnt) == 0x16);
					A_STATIC_ASSERT(offsetof(page_header, data.objId) == 0x18);
					A_STATIC_ASSERT(offsetof(page_header, data.freeCnt) == 0x1C);
					A_STATIC_ASSERT(offsetof(page_header, data.freeData) == 0x1E);
					A_STATIC_ASSERT(offsetof(page_header, data.pageId) == 0x20);
					A_STATIC_ASSERT(offsetof(page_header, data.reservedCnt) == 0x26);
					A_STATIC_ASSERT(offsetof(page_header, data.lsn) == 0x28);
					A_STATIC_ASSERT(offsetof(page_header, data.xactReserved) == 0x32);
					A_STATIC_ASSERT(offsetof(page_header, data.xdesId) == 0x34);
					A_STATIC_ASSERT(offsetof(page_header, data.ghostRecCnt) == 0x3A);
					A_STATIC_ASSERT(offsetof(page_header, data.tornBits) == 0x3C);
					A_STATIC_ASSERT(offsetof(page_header, data.reserved) == 0x40);

					static_assert(std::is_same<pageId_t::value_type, uint32>::value, "");
					static_assert(std::is_same<fileId_t::value_type, uint16>::value, "");
					{
						typedef page_header_meta T;
						A_STATIC_ASSERT(T::headerVersion::offset == 0);
						A_STATIC_ASSERT(T::type::offset == 1);
						static_assert(std::is_same<T::headerVersion::type, uint8>::value, "");
						static_assert(std::is_same<T::type::type, pageType>::value, "");
						static_assert(std::is_same<T::tornBits::type, uint32>::value, "");
						A_STATIC_ASSERT(TL::Length<T::type_list>::value == 20);
					}
					SDL_TRACE_2(__FILE__, " end");
				}
			};
			static unit_test s_test;
		}
	} // db
} // sdl
#endif //#if SV_DEBUG

