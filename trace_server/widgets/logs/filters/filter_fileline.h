#pragma once
#include <filters/filterbase.h>
#include "ui_filter_fileline.h"
#include <boost/serialization/nvp.hpp>
#include <filters/file_filter.hpp>
#include <models/treemodel.h>
#include <models/treeproxy.h>
#include <widgets/logs/logfile_protocol.h>

namespace logs {

class FilterTreeModel : public TreeModel<TreeModelItem>
{       
	Q_OBJECT
public:

	explicit FilterTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~FilterTreeModel () { }
	TreeModel<TreeModelItem>::node_t const * getItemFromIndex (QModelIndex const & index) const { return itemFromIndex(index); }

public Q_SLOTS:
	void onExpanded (QModelIndex const & idx) { expanded(idx); }
	void onCollapsed (QModelIndex const & idx) { collapsed(idx); }

	QModelIndex insertItemWithPath (QStringList const & path, bool checked);
}; 

struct FilterFileLine : FilterBase
{
	Ui_FilterFileLine * m_ui;

	FilterFileLine (QWidget * parent = 0);
	virtual ~FilterFileLine ();

	virtual void initUI ();
	virtual void doneUI ();

	virtual E_FilterType type () const { return e_Filter_FileLine; }

	virtual bool accept (QModelIndex const & sourceIndex);

	virtual void defaultConfig ();
	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	virtual void clear ()
	{
		m_data.set_state_to_childs(m_data.root, TreeModelItem(e_Checked, false));
	}

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		FilterBase::serialize(ar, version);
		ar & boost::serialization::make_nvp("file_filters", m_data);
	}

	// file line specific
	void setupModel ();
	void destroyModel ();

	bool isFileLinePresent (fileline_t const & p, TreeModelItem & state) const; /// checks for file:line existence in the tree
	bool isFileLinePresent (QString const & fileline, TreeModelItem & state) const; /// checks for file:line existence in the tree
	void locateItem (QString const & item, bool scrollto, bool expand);

	friend class FilterProxyModel;

	FilterTreeModel * fileModel () { return m_model; }
	FilterTreeModel const * fileModel () const { return m_model; }
	TreeView * getWidget ();
	TreeView const * getWidget () const;

	void onGotoFileFilter ();
	void onExcludeFileLine ();

	typedef tree_filter<TreeModelItem> file_filters_t;
	typedef file_filters_t::node_t node_t;
	void mergeWithConfig (file_filters_t const & rhs);
	void merge (node_t * lhs, node_t const * rhs);
	void merge_state (node_t * lhs, node_t const * rhs);
	void merge_rhs (node_t * lhs, node_t const * rhs);
	void recompile ();

	file_filters_t			m_data;
	FilterTreeModel *		m_model;
	TreeProxyModel *		m_proxy;
	QItemSelectionModel *	m_proxy_selection;

	Q_OBJECT
public slots:
	void onCollapseChilds ();
	void onCutParentValueChanged (int i);
	void onCancelFilterFileButton ();
	void onFilterFileComboChanged (QString str);
	void onClickedAtFileTree (QModelIndex idx);
	void onSelectAll ();
	void onSelectNone ();
signals:
};

}
