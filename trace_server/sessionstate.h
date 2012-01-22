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
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include "tls.h"
#include <boost/circular_buffer.hpp>

class Server;

typedef std::pair<std::string, std::string> fileline_t;

class SessionState
{
public:
	explicit SessionState(QObject *parent = 0);
	~SessionState ();

	void setTabWidget (int n) { m_tab_idx = n; }
	void setTabWidget (QWidget * w) { m_tab_widget = w; }
	void setupColumns (QList<QString> * cs, MainWindow::columns_sizes_t * csz);
	void setupThreadColors (QList<QColor> const & tc);
	void setupModelFile ();
	QList<QString> const * getColumnsSetup() const { return m_columns_setup; }
	QList<QString> * getColumnsSetup() { return m_columns_setup; }
	MainWindow::columns_sizes_t const * getColumnSizes () const { return m_columns_sizes; }
	MainWindow::columns_sizes_t * getColumnSizes () { return m_columns_sizes; }

	int findColumn4Tag (tlv::tag_t tag) const;
	void insertColumn4Tag (tlv::tag_t tag, int column_idx);
	void insertColumn ();

	QList<QColor> const & getThreadColors () const { return m_thread_colors; }

	ThreadSpecific & getTLS () { return m_tls; }
	ThreadSpecific const & getTLS () const { return m_tls; }

	typedef file_filter file_filters_t;
	file_filters_t const & getFileFilters () const { return m_file_filters; }
	void appendFileFilter(fileline_t const & item);
	void removeFileFilter(fileline_t const & item);
	bool isFileLineExcluded (fileline_t const & p);
	
signals:
	
private:
	friend class Connection;
	friend class Server;

private:
	//MainWindow * m_main_window;
	int m_app_idx;
	int m_tab_idx;
	QWidget * m_tab_widget;
	int m_from_file;
	file_filters_t m_file_filters;

	QList<QColor> m_thread_colors;
	QList<QString> * m_columns_setup;
	MainWindow::columns_sizes_t * m_columns_sizes;
	QMap<tlv::tag_t, int> m_tags2columns;
	ThreadSpecific m_tls;
	QString m_name;
};

