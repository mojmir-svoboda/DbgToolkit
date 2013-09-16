#pragma once
#include "filterbase.h"
#include "ui_filter_fileline.h"
#include <boost/serialization/nvp.hpp>

#include <filters/file_filter.hpp>
#include "treemodel.h"
#include "treeproxy.h"

class FilterTreeModel : public TreeModel<TreeModelItem>
{       
	Q_OBJECT
public:

	explicit FilterTreeModel (QObject * parent = 0, tree_data_t * data = 0);
	~FilterTreeModel () { }

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

	virtual bool accept (DecodedCommand const & cmd) const;

	virtual void loadConfig (QString const & path);
	virtual void saveConfig (QString const & path);
	virtual void applyConfig ();

	Q_OBJECT

	typedef tree_filter<TreeModelItem> file_filters_t;
	typedef file_filters_t::node_t node_t;
	file_filters_t			m_file_filters;

	bool isFileLinePresent (fileline_t const & p, TreeModelItem & state) const; /// checks for file:line existence in the tree
	bool isFileLinePresent (QString const & fileline, TreeModelItem & state) const; /// checks for file:line existence in the tree
	void merge_with (file_filters_t const & rhs);
	void merge (node_t * lhs, node_t const * rhs);
	void merge_state (node_t * lhs, node_t const * rhs);
	void merge_rhs (node_t * lhs, node_t const * rhs);

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("file_filters", m_file_filters);
	}

	void clear ()
	{
		m_file_filters.set_state_to_childs(m_file_filters.root, TreeModelItem(e_Checked, false));
	}

	friend class FilterProxyModel;

	FilterTreeModel * fileModel () { return m_file_model; }
	FilterTreeModel const * fileModel () const { return m_file_model; }
	TreeView * getWidgetFile ();
	TreeView const * getWidgetFile () const;
	void setupModelFile ();
	void destroyModelFile ();
	void onGotoFileFilter ();
	void onFilterFileComboChanged (QString str);
	void onCancelFilterFileButton ();

	void onExcludeFileLine ();

	FilterTreeModel * m_file_model;
	TreeProxyModel * m_file_proxy;
	QItemSelectionModel * m_proxy_selection;
};
