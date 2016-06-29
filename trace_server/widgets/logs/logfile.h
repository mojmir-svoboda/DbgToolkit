#pragma once
#include <membuffer.h>
#include <virtualallocbuffer.h>
#include <tuple>
#include <trace_proto/decoder.h>
//#include <trace_proto/LogScopeType.h>
//#include <trace_proto/LogMsg.h>
#include "logfile_protocol.h"
#include "logconfig.h"
#include <utils_qt/qstringdatatable.h>
#include <utils_qt/utils_tuple.h>
#include <sysfn/time_query.h>
#include "tls.h"

namespace logs { namespace proto {

	struct Attrs
	{
		fgcols_t m_fgcols;
		bgcols_t m_bgcols;
    indents_t m_indents;
		flags_t m_flags;
	};

	struct Record
	{
		Record (values_t const & v) : m_values(v) { }
		values_t m_values;
	};

	using rowdata_t = std::tuple<Record const *, Attrs const *>;
	using rw_rowdata_t = std::tuple<Record *, Attrs *>;

	struct RecordTable : MemBuffer<VirtualAllocBuffer>
	{
		constexpr size_t getRecordSize () const { return sizeof(Record); }

		bool canAddRecord () const
		{
			return canMoveEnd(getRecordSize());
		}

		bool canAddRecordWithRealloc ()
		{
			if (canAddRecord())
				return true;

			resizeStorage(calcNextSize());
			return canMoveEnd(getRecordSize());
		}

		bool addRecord (values_t const & values)
		{
			if (char * mem = static_cast<char *>(Malloc(getRecordSize())))
			{
				Record * data = new (mem) Record(values);
				return true;
			}
			return false;
		}

		Record const * getRecord (size_t row) const
		{
			char const * const ptr = begin() + getRecordSize() * row;
			Record const * const rec = reinterpret_cast<Record const *>(ptr);
			return rec;
		}
		Record * getRecord (size_t row)
		{
			char * const ptr = begin() + getRecordSize() * row;
			Record * const rec = reinterpret_cast<Record *>(ptr);
			return rec;
		}
		void clear () { Reset(); }
	};

	struct AttrsTable : MemBuffer<VirtualAllocBuffer>
	{
		constexpr size_t getAttrsSize () const { return sizeof(Attrs); }
		bool canAddAttrs () const
		{
			return canMoveEnd(getAttrsSize());
		}
		bool canAddAttrsWithRealloc()
		{
			if (canAddAttrs())
				return true;

			resizeStorage(calcNextSize());
			return canMoveEnd(getAttrsSize());
		}

		bool addAttrs ()
		{
			if (char * mem = static_cast<char *>(Malloc(getAttrsSize())))
			{
				Attrs * data = new (mem) Attrs;
				return true;
			}
			return false;
		}
		bool addAttrs (Attrs const & attrs)
		{
			if (char * mem = static_cast<char *>(Malloc(getAttrsSize())))
			{
				Attrs * data = new (mem) Attrs(attrs);
				return true;
			}
			return false;
		}

		Attrs const * getAttrs (size_t row) const
		{
			char const * const ptr = begin() + getAttrsSize() * row;
			Attrs const * const attrs = reinterpret_cast<Attrs const *>(ptr);
			return attrs;
		}
		Attrs * getAttrs (size_t row)
		{
			char * const ptr = begin() + getAttrsSize() * row;
			Attrs * const attrs = reinterpret_cast<Attrs *>(ptr);
			return attrs;
		}

		void clear() { Reset(); }
	};


	struct LogData {
		RecordTable m_records;
		AttrsTable m_attrs;
		QStringDataTable m_strings;

		void clear ()
		{
			m_records.clear();
			m_attrs.clear();
			m_strings.clear();
		}
	};

	struct LogFile {
		LogData m_data;
		ThreadSpecific m_tls;

		LogFile () { }

		void init ()
		{
			if (sizeof(void *) == 4)
			{
				// @TODO @FIXME running out of address space on x86
				m_data.m_records.mkStorage(64 * 1024 * 1024);
				m_data.m_attrs.mkStorage(96 * 1024 * 1024);
				m_data.m_strings.mkStorage(128 * 1024 * 1024);
			}
			else
			{
				m_data.m_records.mkStorage(128 * 1024 * 1024);
				m_data.m_attrs.mkStorage(256 * 1024 * 1024);
				m_data.m_strings.mkStorage(512 * 1024 * 1024);
			}
		}

		void clear ()
		{
			m_data.clear();
		}

		bool handleLogCommand (DecodedCommand const & cmd, LogConfig const & config)
		{
			//sys::hptimer_t const now = sys::queryTime_us();

			if (!m_data.m_records.canAddRecordWithRealloc() || !m_data.m_attrs.canAddAttrsWithRealloc())
				return false;

			tag2type<int_<tag_stime>> stime = cmd.m_stime;
			tag2type<int_<tag_dt>> dt = 0;
			tag2type<int_<tag_ctime>> ctime = cmd.choice.log.ctime;
			tag2type<int_<tag_lvl>> lvl = cmd.choice.log.lvl;
			tag2type<int_<tag_ctx>> ctx = cmd.choice.log.ctx;
			tag2type<int_<tag_tid>> tid = cmd.choice.log.tid;
			std::tuple<bool, stroffs_t> qtfile = m_data.m_strings.AddQStringDataFromOCTET_STRING(cmd.choice.log.file);
			if (std::get<0>(qtfile) == false)
				return false;
			tag2type<int_<tag_file>> file = std::get<1>(qtfile);
			tag2type<int_<tag_line>> line = cmd.choice.log.line;
			std::tuple<bool, stroffs_t> qtfunc = m_data.m_strings.AddQStringDataFromOCTET_STRING(cmd.choice.log.func);
			if (std::get<0>(qtfunc) == false)
				return false;
			tag2type<int_<tag_func>> func = std::get<1>(qtfunc);
			LogScopeType scptype = static_cast<LogScopeType>(cmd.choice.log.scope);
			tag2type<int_<tag_msg>> msg;
			// scope based indenting (per thread id (tid))
			int const thread_idx = m_tls.findThreadId(tid);
			int indent = 0;
			if (config.m_indent)
			{
				if (thread_idx >= 0)
					indent = m_tls.m_indents[thread_idx];

				if (indent > 0)
				{
					if (scptype == LogScopeType_scopeExit)
						--indent; // indent is decreased after this call, that's why
				}
			}
			// set dt
 			unsigned long long last_t = m_tls.lastTime(thread_idx);
			if (last_t == 0)
				last_t = ctime; // first entry
 			unsigned long long const t = ctime;
 			dt = t - last_t;
 			m_tls.setLastTime(thread_idx, t);

			std::tuple<bool, stroffs_t> qtmsg = m_data.m_strings.AddQStringDataFromOCTET_STRING(cmd.choice.log.msg, indent, scptype);
			msg = std::get<1>(qtmsg);
			if (std::get<0>(qtmsg) == false)
				return false;
			//QString const & dbg = m_data.m_strings.GetQString(std::get<1>(qtmsg));

			values_t r = std::make_tuple(stime, dt, ctime, tid, lvl, ctx, file, line, func, msg);
			m_data.m_records.addRecord(r);
			Attrs a;
			std::get<tag2col<int_<tag_msg>>::value>(a.m_flags).m_scope_type = scptype;
			std::get<tag2col<int_<tag_msg>>::value>(a.m_indents) = indent;
			std::get<tag2col<int_<tag_tid>>::value>(a.m_fgcols) = config.m_thread_colors[thread_idx].first.rgba();
			std::get<tag2col<int_<tag_tid>>::value>(a.m_bgcols) = config.m_thread_colors[thread_idx].second.rgba();
			m_data.m_attrs.addAttrs(a);

			if (config.m_indent)
			{
				int const idx = m_tls.findThreadId(tid);
				if (scptype == LogScopeType_scopeEntry)
					m_tls.incrIndent(idx);
				if (scptype == LogScopeType_scopeExit)
					m_tls.decrIndent(idx);
			}

			return true;
		}

		int getRecordCount () const
		{
			size_t const sz = m_data.m_records.m_end_offset;
			size_t const n = sz / sizeof(values_t);
			return static_cast<int>(n);
		}

		int rowCount () const
		{
			return getRecordCount();
		}

		constexpr int columnCount () const
		{
			return std::tuple_size<values_t>::value;
		}

		Record const * getRecord (size_t row) const { return m_data.m_records.getRecord(row); }
		Record * getRecord (size_t row) { return m_data.m_records.getRecord(row); }
		Attrs const * getAttrs (size_t row) const { return m_data.m_attrs.getAttrs(row); }
		Attrs * getAttrs (size_t row) { return m_data.m_attrs.getAttrs(row); }

		template<typename T>
		QVariant getVal (T const & t) const { return QVariant::fromValue(t); }
		QVariant getVal (stroffs_t stroffs) const
		{
			const QString & s = m_data.m_strings.GetQString(stroffs);
			return s;
		}

		template<int N, class T>
		QVariant getNthFromRecord (T & t) const
		{
			return getVal(std::get<N>(t));
		}

		template<class T, int... Is>
		QVariant getRecordData (T & t, int index, seq<Is...>) const
		{
			using get_fn_prototype = QVariant (LogFile::*)(T &) const;
			static get_fn_prototype const funcs[] = { &LogFile::getNthFromRecord<Is, T>... };
			get_fn_prototype const & nth = funcs[index];
			return (this->*nth)(t);
		}
		template<class T>
		QVariant getRecordData (T & t, int index) const
		{
			return getRecordData(t, index, gen_seq<std::tuple_size<T>::value>{ });
		}

		template<typename T, typename U>
		void setVal (T & t, U const & u) const { t = u; }
		void setVal (stroffs_t stroffs) const
		{
			// not supported
		}

		template<int N, class T, class U>
		void setNthFromRecord (T & t, U const & u) const
		{
			return setVal(std::get<N>(t), u);
		}

		template<class T, class U, int... Is>
		void setRecordData (T & t, U const & u, int index, seq<Is...>) const
		{
			using set_fn_prototype = void (LogFile::*)(T &, U const &) const;
			constexpr static set_fn_prototype const funcs[] = { &LogFile::setNthFromRecord<Is, T, U>... };
			set_fn_prototype const & nth = funcs[index];
			(this->*nth)(t, u);
		}
		template<class T, class U>
		void setRecordData (T & t, int index, U const & u) const
		{
			setRecordData(t, u, index, gen_seq<std::tuple_size<T>::value>{ });
		}

		rowdata_t const getRecordDataForRow (int row) const
		{
			proto::Record const * rec = nullptr;
			proto::Attrs const * attrs = nullptr;
			if (row >= 0 && row < rowCount())
			{
				rec = getRecord(row);
				attrs = getAttrs(row);
			}
			return std::make_tuple(rec, attrs);
		}
		rw_rowdata_t getRecordDataForRowReadWrite (int row)
		{
			proto::Record * rec = nullptr;
			proto::Attrs * attrs = nullptr;
			if (row >= 0 && row < rowCount())
			{
				rec = getRecord(row);
				attrs = getAttrs(row);
			}
			return std::make_tuple(rec, attrs);
		}

	};
}}

