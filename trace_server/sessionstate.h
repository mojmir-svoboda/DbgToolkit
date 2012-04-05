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
#include "mainwindow.h"
#include <tlv_parser/tlv_parser.h>
#include <tlv_parser/tlv_decoder.h> // htons!
#include <filters/file_filter.hpp>
#include "tls.h"

class Server;
class MainWindow;

typedef std::pair<std::string, std::string> fileline_t;

struct CollapsedBlock {
	QString m_tid;
	int m_from;
	int m_to;

	CollapsedBlock (QString tid, int from, int to) : m_tid(tid), m_from(from), m_to(to) { }
};

enum E_ColorRole { e_Bg, e_Fg };

struct ColorizedText {
	E_ColorRole m_role;
	Qt::GlobalColor m_color;
	QRegExp m_regex;

	bool isValid () const { return m_regex.isValid(); }
	bool exactMatch (QString str) const { return m_regex.exactMatch(str); }

	ColorizedText (QString regex, Qt::GlobalColor col, E_ColorRole r) : m_color(col), m_role(r), m_regex(regex) { }
};

class SessionState
{
public:
	explicit SessionState(QObject *parent = 0);
	~SessionState ();

	void setTabWidget (int n) { m_tab_idx = n; }
	void setTabWidget (QWidget * w) { m_tab_widget = w; }
	void setupColumns (QList<QString> const * column_setup_template, MainWindow::columns_sizes_t * sizes);
	void setupThreadColors (QList<QColor> const & tc);
	QList<QString> const * getColumnsSetupCurrent () const { return m_columns_setup_current; }
	QList<QString> * getColumnsSetupCurrent () { return m_columns_setup_current; }
	QList<QString> const * getColumnsSetupTemplate () const { return m_columns_setup_template; }
	MainWindow::columns_sizes_t const * getColumnSizes () const { return m_columns_sizes; }
	MainWindow::columns_sizes_t * getColumnSizes () { return m_columns_sizes; }

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
	void appendFileFilter(fileline_t const & item);		/// add file + line pair
	void appendFileFilter(std::string const & item);	/// add concantenated item
	void removeFileFilter(fileline_t const & item);
	bool isFileLineExcluded (fileline_t const & p) const;

	typedef std::vector<std::string> tid_filters_t;
	void appendTIDFilter (std::string const & item);
	void removeTIDFilter (std::string const & item);
	bool isTIDExcluded (std::string const & item) const;

	void appendCollapsedBlock (QString tid, int from, int to);
	bool findCollapsedBlock (QString tid, int from, int to) const;
	bool eraseCollapsedBlock (QString tid, int from, int to);
	bool isBlockCollapsed (QString tid, int row);
	bool isBlockCollapsedIncl (QString tid, int row);

	bool isMatchedText (QString str, int & color, E_ColorRole & role) const;

	void exclude_content_to_row (int row) { m_exclude_content_to_row = row; }
	int exclude_content_to_row () const { return m_exclude_content_to_row; }
	
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
	file_filters_t m_file_filters;
	tid_filters_t m_tid_filters;

	QList<QColor> m_thread_colors;
	QList<ColorizedText> m_colorized_texts;
	QList<QString> * m_columns_setup_current;
	QList<QString> const * m_columns_setup_template;
	MainWindow::columns_sizes_t * m_columns_sizes;
	QMap<tlv::tag_t, int> m_tags2columns;
	ThreadSpecific m_tls;
	QString m_name;
	QList<CollapsedBlock> m_collapse_blocks;
};

