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
//#include "mainwindow.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h> // htons!
#include <filters/file_filter.hpp>
#include "tls.h"
#include "types.h"

class Server;
class MainWindow;

typedef std::pair<std::string, std::string> fileline_t;

struct CollapsedBlock {
	QString m_tid;
	int m_from;
	int m_to;
	QString m_file;
	QString m_line;

	CollapsedBlock (QString tid, int from, int to, QString file, QString line)
		: m_tid(tid), m_from(from), m_to(to), m_file(file), m_line(line)
	{ }
};

struct ColorizedText {
	E_ColorRole m_role;
	QColor m_qcolor;
	std::string m_regex_str;
	QRegExp m_regex;
	bool m_isEnabled;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	ColorizedText (std::string const & rs, E_ColorRole r)
        : m_role(r)
        , m_qcolor(Qt::magenta), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_isEnabled(0)
	{ }

	ColorizedText (std::string const & rs, QColor const & col, E_ColorRole r)
        : m_role(r)
        , m_qcolor(col), m_regex_str(rs), m_regex(QString::fromStdString(rs)), m_isEnabled(0)
	{ }

};

struct SessionExport {
	std::string m_name;
	std::string m_file_filters;
	std::string m_regex_filters;
	std::string m_color_regex_filters;
	std::string m_collapsed_blocks;
};

typedef unsigned long long context_t;
struct ContextFilter {
	context_t m_context;
	std::string m_text;
	E_FilterMode m_filterMode;
	bool m_isEnabled;

	ContextFilter (context_t ctx) : m_context(ctx), m_text(), m_filterMode(e_Include), m_isEnabled(false) { }
};

class SessionState
{
public:
	explicit SessionState(QObject *parent = 0);
	~SessionState ();

	void setTabWidget (int n) { m_tab_idx = n; }
	void setTabWidget (QWidget * w) { m_tab_widget = w; }
	void setupColumns (QList<QString> const * column_setup_template, columns_sizes_t * sizes);
	void setupThreadColors (QList<QColor> const & tc);
	QList<QString> const * getColumnsSetupCurrent () const { return m_columns_setup_current; }
	QList<QString> * getColumnsSetupCurrent () { return m_columns_setup_current; }
	QList<QString> const * getColumnsSetupTemplate () const { return m_columns_setup_template; }
	columns_sizes_t const * getColumnSizes () const { return m_columns_sizes; }
	columns_sizes_t * getColumnSizes () { return m_columns_sizes; }

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
	void appendFileFilter (fileline_t const & item, E_FilterMode);		/// add file + line pair
	void appendFileFilter (std::string const & item, E_FilterMode);	/// add concantenated item
	void removeFileFilter (fileline_t const & item, E_FilterMode);
	bool isFileLineExcluded (fileline_t const & p, E_FilterMode) const;

	typedef QList<ContextFilter> ctx_filters_t;
	ctx_filters_t const & getCtxFilters () const { return m_ctx_filters; }
	void appendCtxFilter (context_t item);
	void flipCtxFilterMode (context_t item, E_FilterMode mode);
	void removeCtxFilter (context_t item);
	bool isCtxExcluded (context_t item) const;


	typedef std::vector<std::string> tid_filters_t;
	void appendTIDFilter (std::string const & item);
	void removeTIDFilter (std::string const & item);
	bool isTIDExcluded (std::string const & item) const;

	void appendCollapsedBlock (QString tid, int from, int to, QString file, QString line);
	bool findCollapsedBlock (QString tid, int from, int to) const;
	bool eraseCollapsedBlock (QString tid, int from, int to);
	bool isBlockCollapsed (QString tid, int row) const;
	bool isBlockCollapsedIncl (QString tid, int row) const;

	void appendToColorRegexFilters (std::string const & str);
	void removeFromColorRegexFilters (std::string const & str);
	bool isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const;
	void setRegexColor (std::string const & s, QColor col);
	void setColorRegexChecked (std::string const & s, bool checked);

	void excludeContentToRow (int row) { m_exclude_content_to_row = row; }
	int excludeContentToRow () const { return m_exclude_content_to_row; }

	void toggleRefFromRow (int row) { m_toggle_ref_row = row; }
	int toggleRefFromRow () const { return m_toggle_ref_row; }

	void makeInexactCopy (SessionState const & rhs);
   
	void sessionExport (SessionExport & e) const;
	void sessionImport (SessionExport const & e);
	
signals:
	
private:
	friend class Connection;
	friend class Server;
	friend class MainWindow;

private:
	//MainWindow * m_main_window;
	int m_app_idx;
	int m_tab_idx;
	QWidget * m_tab_widget;
	int m_from_file;
	int m_exclude_content_to_row;
	int m_toggle_ref_row;
	file_filters_t m_file_filters;
	ctx_filters_t m_ctx_filters;
	tid_filters_t m_tid_filters;

	QList<QColor> m_thread_colors;
	QList<ColorizedText> m_colorized_texts;
	QList<QString> * m_columns_setup_current;
	QList<QString> const * m_columns_setup_template;
	columns_sizes_t * m_columns_sizes;
	QMap<tlv::tag_t, int> m_tags2columns;
	ThreadSpecific m_tls;
	QString m_name;
	QList<CollapsedBlock> m_collapse_blocks;
};

