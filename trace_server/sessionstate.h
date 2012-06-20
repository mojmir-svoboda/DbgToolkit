/**
 * Copyright (C) 2011 Mojmir Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN SessionState WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#pragma once
#include <QString>
#include <QList>
#include <QMap>
#include <QColor>
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h> // htons!
#include <filters/file_filter.hpp>
#include "tls.h"
#include "types.h"

class Server;
class MainWindow;

struct CollapsedBlock {
	QString m_tid;
	int m_from;
	int m_to;
	QString m_file;
	QString m_line;

	CollapsedBlock () { }

	CollapsedBlock (QString tid, int from, int to, QString file, QString line)
		: m_tid(tid), m_from(from), m_to(to), m_file(file), m_line(line)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_tid;
		ar & m_from;
		ar & m_to;
		ar & m_file;
		ar & m_line;
	}
};

struct FilteredRegex {
	std::string m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;
	bool m_is_inclusive;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	FilteredRegex () { }
	FilteredRegex (std::string const & rs, bool is_inclusive)
        : m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0), m_is_inclusive(is_inclusive)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_regex_str;
		ar & m_regex;
		ar & m_is_enabled;
		ar & m_is_inclusive;
	}
};

struct ColorizedText {
	E_ColorRole m_role;
	QColor m_qcolor;
	QColor m_bgcolor;
	std::string m_regex_str;
	QRegExp m_regex;
	bool m_is_enabled;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	ColorizedText () { }

	ColorizedText (std::string const & rs, E_ColorRole r)
        : m_role(r)
        , m_qcolor(Qt::magenta), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0)
	{ }

	ColorizedText (std::string const & rs, QColor const & col, E_ColorRole r)
        : m_role(r)
        , m_qcolor(col), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_is_enabled(0)
	{ }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_role;
		ar & m_regex_str;
		ar & m_qcolor;
		ar & m_bgcolor;
		ar & m_regex_str;
		ar & m_regex;
		ar & m_is_enabled;
	}
};

struct SessionExport {
	std::string m_name;
	std::string m_file_filters;
	std::string m_regex_filters;
	std::string m_regex_fmode;
	std::string m_regex_enabled;
	std::string m_colortext_regexs;
	std::string m_colortext_colors;
	std::string m_colortext_enabled;
	std::string m_collapsed_blocks;
};

struct SessionStats {
	size_t m_Read_B;
	size_t m_Write_B;
	//hptimer_t m_StartT;
};

class SessionState
{
public:
	explicit SessionState(QObject *parent = 0);
	~SessionState ();

	void setTabWidget (int n) { m_tab_idx = n; }
	void setTabWidget (QWidget * w) { m_tab_widget = w; }
	void setupColumns (QList<QString> const * column_setup_template, columns_sizes_t * sizes
			, columns_align_t const * ca_template, columns_elide_t const * ce_template);
	void setupThreadColors (QList<QColor> const & tc);

	QList<QString> const * getColumnsSetupCurrent () const { return m_columns_setup_current; }
	QList<QString> * getColumnsSetupCurrent () { return m_columns_setup_current; }
	QList<QString> const * getColumnsSetupTemplate () const { return m_columns_setup_template; }

	columns_sizes_t const * getColumnSizes () const { return m_columns_sizes; }
	columns_sizes_t * getColumnSizes () { return m_columns_sizes; }

	QList<QString> const * getColumnsAlignTemplate () const { return m_columns_align_template; }
	QList<QString> const * getColumnsElideTemplate () const { return m_columns_elide_template; }

	int findColumn4TagInTemplate (tlv::tag_t tag) const;
	int findColumn4Tag (tlv::tag_t tag) const;
	void insertColumn4Tag (tlv::tag_t tag, int column_idx);
	void insertColumn ();
	int insertColumn (tlv::tag_t tag);

	QList<QColor> const & getThreadColors () const { return m_thread_colors; }

	ThreadSpecific & getTLS () { return m_tls; }
	ThreadSpecific const & getTLS () const { return m_tls; }

	typedef file_filter file_filters_t;
	file_filters_t const & getFileFilters () const { return m_file_filters; }
	bool isFileLinePresent (fileline_t const & p, E_NodeStates & state) const; /// checks for file:line existence in the tree
	bool isFileLinePresent (std::string const & fileline, E_NodeStates & state) const; /// checks for file:line existence in the tree
	void stateToFileChilds (fileline_t const & item, E_NodeStates const state);

	typedef QList<context_t> ctx_filters_t;
	ctx_filters_t const & getCtxFilters () const { return m_ctx_filters; }
	void appendCtxFilter (context_t item);
	void flipFilterMode (E_FilterMode mode);
	void removeCtxFilter (context_t item);
	bool isCtxExcluded (context_t item) const;


	typedef std::vector<std::string> tid_filters_t;
	void appendTIDFilter (std::string const & item);
	void removeTIDFilter (std::string const & item);
	bool isTIDExcluded (std::string const & item) const;

	typedef std::vector<std::string> lvl_filters_t;
	void appendLvlFilter (std::string const & item);
	void removeLvlFilter (std::string const & item);
	bool isLvlExcluded (std::string const & item) const;

	void appendCollapsedBlock (QString tid, int from, int to, QString file, QString line);
	bool findCollapsedBlock (QString tid, int from, int to) const;
	bool eraseCollapsedBlock (QString tid, int from, int to);
	bool isBlockCollapsed (QString tid, int row) const;
	bool isBlockCollapsedIncl (QString tid, int row) const;

	// text colorization
	void appendToColorRegexFilters (std::string const & str);
	void removeFromColorRegexFilters (std::string const & str);
	bool isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const;
	void setRegexColor (std::string const & s, QColor col);
	void setColorRegexChecked (std::string const & s, bool checked);

	// regex filtering
	void appendToRegexFilters (std::string const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (std::string const & str);
	bool isMatchedRegexExcluded (QString str) const;
	void setRegexChecked (std::string const & s, bool checked);
	void setRegexInclusive (std::string const & s, bool inclusive);

	void excludeContentToRow (int row) { m_exclude_content_to_row = row; }
	int excludeContentToRow () const { return m_exclude_content_to_row; }

	void toggleRefFromRow (int row) { m_toggle_ref_row = row; }
	int toggleRefFromRow () const { return m_toggle_ref_row; }

	void setFilterMode (E_FilterMode m) { m_filter_mode = m; }
	E_FilterMode getFilterMode () const { return m_filter_mode; }

	void sessionDump (SessionExport & e) const;

	void clearFilters ();
	void onClearFileFilter ()
	{
		m_file_filters.set_state_to_childs(m_file_filters.root, e_Unchecked);
	}
	void onClearCtxFilter () { m_ctx_filters.clear(); }
	void onClearTIDFilter () { m_tid_filters.clear(); }
	void onClearScopeFilter () { m_collapse_blocks.clear(); }
	void onClearColorizedRegexFilter () { m_colorized_texts.clear(); }
	void onClearLvlFilter () { m_lvl_filters.clear(); }
	void onClearRegexFilter () { m_filtered_regexps.clear(); }

	unsigned getRecvBytes () const { return m_recv_bytes; }
	
signals:
	
private:
public:

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & m_file_filters;
		//ar & m_thread_colors;
		ar & m_colorized_texts;
		ar & m_filtered_regexps;
		ar & m_collapse_blocks;
		ar & m_thread_colors;
	}

	friend class Connection;
	friend class Server;
	friend class MainWindow;
	friend class FilterProxyModel;

private:
	int m_app_idx;
	int m_tab_idx;
	QWidget * m_tab_widget;
	int m_from_file;
	int m_exclude_content_to_row;
	int m_toggle_ref_row;
	E_FilterMode m_filter_mode;
	file_filters_t m_file_filters;
	ctx_filters_t m_ctx_filters;
	tid_filters_t m_tid_filters;
	lvl_filters_t m_lvl_filters;

	QList<QColor> m_thread_colors;
	QList<ColorizedText> m_colorized_texts;
	QList<FilteredRegex> m_filtered_regexps;
	QList<QString> * m_columns_setup_current;
	QList<QString> const * m_columns_setup_template;
	QList<QString> const * m_columns_align_template;
	QList<QString> const * m_columns_elide_template;
	columns_sizes_t * m_columns_sizes;
	QMap<tlv::tag_t, int> m_tags2columns;
	ThreadSpecific m_tls;
	QString m_name;
	QList<CollapsedBlock> m_collapse_blocks;
	unsigned m_recv_bytes;
};

