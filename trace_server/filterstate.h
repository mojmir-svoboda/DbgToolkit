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
#include <boost/serialization/nvp.hpp>
#include "tls.h"
#include "config.h"

class FilterWidget;
class PersistentFilter;
class MainWindow;

struct SessionStats {
	size_t m_Read_B;
	size_t m_Write_B;
	//hptimer_t m_StartT;
};

struct Dict {
	QList<QString> m_names;
	QList<QString> m_strvalues;
	QList<int> m_values;

	QString findNameFor (QString const & strval) const
	{
		for (int i = 0, ie = m_strvalues.size(); i < ie; ++i)
			if (m_strvalues.at(i) == strval)
				return m_names.at(i);
		return QString();
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("names", m_names);
		ar & boost::serialization::make_nvp("strvalues", m_strvalues);
		ar & boost::serialization::make_nvp("values", m_values);
	}
};

class FilterState
{
public:
	explicit FilterState (QObject *parent = 0);
	~FilterState ();

/*		
	// cfg
	void setupColumns (QList<QString> * column_setup_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);
	void setupColumnsCSV (QList<QString> * column_setup_template, columns_sizes_t * sizes , columns_align_t * ca_template, columns_elide_t * ce_template);
	void setupThreadColors (QList<QColor> const & tc);
	bool isConfigured () const { return m_columns_setup_current && m_columns_setup_current->size(); }
	QList<QString> const * getColumnsSetupCurrent () const { return m_columns_setup_current; }
	QList<QString> * getColumnsSetupCurrent () { return m_columns_setup_current; }
	QList<QString> const * getColumnsSetupTemplate () const { return m_columns_setup_template; }
	columns_sizes_t const * getColumnSizes () const { return m_columns_sizes; }
	columns_sizes_t * getColumnSizes () { return m_columns_sizes; }
	QList<QString> const * getColumnsAlignTemplate () const { return m_columns_align_template; }
	QList<QString> const * getColumnsElideTemplate () const { return m_columns_elide_template; }
*/

	// filter state
	// file
	typedef tree_filter<TreeModelItem> file_filters_t;
	typedef file_filters_t::node_t node_t;
	bool isFileLinePresent (fileline_t const & p, TreeModelItem & state) const; /// checks for file:line existence in the tree
	bool isFileLinePresent (QString const & fileline, TreeModelItem & state) const; /// checks for file:line existence in the tree
	void merge_with (file_filters_t const & rhs);
	void merge (node_t * lhs, node_t const * rhs);
	void merge_state (node_t * lhs, node_t const * rhs);
	void merge_rhs (node_t * lhs, node_t const * rhs);

	// ctx
	typedef QList<FilteredContext> ctx_filters_t;
	bool isCtxPresent (QString const & item, bool & enabled) const;
	void appendCtxFilter (QString const & item);
	void removeCtxFilter (QString const & item);

	// tid
	typedef std::vector<QString> tid_filters_t;
	void appendTIDFilter (QString const & item);
	void removeTIDFilter (QString const & item);
	bool isTIDExcluded (QString const & item) const;

	// lvl
	typedef QList<FilteredLevel> lvl_filters_t;
	void appendLvlFilter (QString const & item);
	void removeLvlFilter (QString const & item);
	bool isLvlPresent (QString const & item, bool & enabled, E_LevelMode & lvlmode) const;
	bool setLvlMode (QString const & item, bool enabled, E_LevelMode lvlmode);

	// blocks
	void appendCollapsedBlock (QString tid, int from, int to, QString file, QString line);
	bool findCollapsedBlock (QString tid, int from, int to) const;
	bool eraseCollapsedBlock (QString tid, int from, int to);
	bool isBlockCollapsed (QString tid, int row) const;
	bool isBlockCollapsedIncl (QString tid, int row) const;

	// text colorization
	void appendToColorRegexFilters (QString const & str);
	void removeFromColorRegexFilters (QString const & str);
	bool isMatchedColorizedText (QString str, QColor & color, E_ColorRole & role) const;
	void setRegexColor (QString const & s, QColor col);
	void setColorRegexChecked (QString const & s, bool checked);

	// regex filtering
	void appendToRegexFilters (QString const & str, bool checked, bool inclusive);
	void removeFromRegexFilters (QString const & str);
	bool isMatchedRegexExcluded (QString str) const;
	void setRegexChecked (QString const & s, bool checked);
	void setRegexInclusive (QString const & s, bool inclusive);

	// string filtering
	void appendToStringFilters (QString const & str, bool checked, int state);
	void removeFromStringFilters (QString const & str);
	bool isMatchedStringExcluded (QString str) const;
	void setStringChecked (QString const & s, bool checked);
	void setStringState (QString const & s, int state);


	void clearFilters ();
	void onClearFileFilter ()
	{
		m_file_filters.set_state_to_childs(m_file_filters.root, TreeModelItem(e_Checked, false));
	}
	void onClearCtxFilter () { m_ctx_filters.clear(); }
	void onClearTIDFilter () { m_tid_filters.clear(); }
	void onClearScopeFilter () { m_collapse_blocks.clear(); }
	void onClearColorizedRegexFilter () { m_colorized_texts.clear(); }
	void onClearLvlFilter () { m_lvl_filters.clear(); }
	void onClearRegexFilter () { m_filtered_regexps.clear(); }
	void onClearStringFilter () { m_filtered_strings.clear(); }



signals:
	
private:
public:

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("file_filters", m_file_filters);
		ar & boost::serialization::make_nvp("ctx_filters", m_ctx_filters);
		ar & boost::serialization::make_nvp("lvl_filters", m_lvl_filters);
		ar & boost::serialization::make_nvp("data_filters", m_data_filters);
		ar & boost::serialization::make_nvp("colorized_texts", m_colorized_texts);
		ar & boost::serialization::make_nvp("filtered_regexps", m_filtered_regexps);
		ar & boost::serialization::make_nvp("collapse_blocks", m_collapse_blocks);
	}

	friend class Connection;
	friend class FilterProxyModel;
	friend class FilterWidget;
	friend class PersistentFilter;


	file_filters_t			m_file_filters;
	ctx_filters_t			m_ctx_filters;
	lvl_filters_t			m_lvl_filters;
	tid_filters_t			m_tid_filters;
	typedef tree_filter<TreeModelItem> data_filters_t;
	data_filters_t			m_data_filters;	// @TODO: k cemu to je?
	QList<ColorizedText>	m_colorized_texts;
	QList<FilteredRegex>	m_filtered_regexps;
	QList<CollapsedBlock>	m_collapse_blocks;
	QList<FilteredString>	m_filtered_strings;

};

bool saveFilterState (FilterState const & s, char const * filename);
bool loadFilterState (FilterState & s, char const * filename);
bool loadFilterState (FilterState const & src, FilterState & target);
