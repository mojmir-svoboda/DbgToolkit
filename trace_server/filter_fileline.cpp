#include "filter_fileline.h"

FilterFileLine::FilterFileLine (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterFileLine)
	, m_file_model(0)
	, m_file_proxy(0)
	, m_proxy_selection(0)
{
	setupModelFile();
}

void FilterFileLine::initUI ()
{
	m_ui->setupUi(this);
}

void FilterFileLine::doneUI ()
{
	destroyModelFile();
}

bool FilterFileLine::accept (DecodedCommand const & cmd) const
{
	return true;
}

void FilterFileLine::loadConfig (QString const & path)
{
}

void FilterFileLine::saveConfig (QString const & path)
{
	//QString const fsname = fname + "." + g_filterStateTag;
	//saveFilterState(m_filter_state, fsname.toStdString());
}

void FilterFileLine::applyConfig ()
{
	//m_filter_state.merge_with(src.m_file_filters);
}

FilterTreeModel::FilterTreeModel (QObject * parent, tree_data_t * data)
	: TreeModel<TreeModelItem>(parent, data)
{
	qDebug("%s", __FUNCTION__);
}

QModelIndex FilterTreeModel::insertItemWithPath (QStringList const & path, bool checked)
{
	QString const name = path.join("/");
	return insertItemWithHint(name, checked);
}



///////// file filters
bool FilterFileLine::isFileLinePresent (fileline_t const & item, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(item.first + "/" + item.second, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}
bool FilterFileLine::isFileLinePresent (QString const & fileline, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_file_filters.is_present(fileline, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}

void FilterFileLine::merge_rhs (node_t * lhs, node_t const * rhs)
{
	node_t const * rhs_child = rhs->children;
	while (rhs_child)
	{
		node_t * lhs_child = lhs->children;
		bool found = false;
		while (lhs_child)
		{
			if (lhs_child->key == rhs_child->key)
			{
				found = true;
				break;
			}
			lhs_child = lhs_child->next;
		}

		if (!found)
		{
			lhs_child = new node_t(*rhs_child);
			node_t::node_append(lhs, lhs_child);
		}
		else
			lhs_child->data = rhs_child->data;

		merge(lhs_child, rhs_child);

		rhs_child = rhs_child->next;
	}
}

void FilterFileLine::merge_state (node_t * lhs, node_t const * rhs)
{
	node_t * lhs_child = lhs->children;

	while (lhs_child)
	{
		node_t * rhs_child = rhs->children;
		bool found = false;
		while (rhs_child)
		{
			if (rhs_child->key == lhs_child->key)
			{
				found = true;
				// assert na state
				break;
			}
			rhs_child = rhs_child->next;
		}

		if (found)
		{
			lhs_child->data = rhs_child->data;
			merge_state(lhs_child, rhs_child);
		}
		else
		{
			node_t * parent = lhs_child->parent;
			if (parent)
			{
				switch (parent->data.m_state)
				{
					case e_Unchecked:
						m_file_filters.set_state_to_childs(lhs_child, parent->data);
						break;
					case e_PartialCheck:
						m_file_filters.set_state_to_childs(lhs_child, TreeModelItem(e_Unchecked, 1));
						break;
					case e_Checked:
						m_file_filters.set_state_to_childs(lhs_child, parent->data);
						break;
				}
			}
		}

		lhs_child = lhs_child->next;
	}
}

void FilterFileLine::merge (node_t * lhs, node_t const * rhs)
{
	merge_rhs(lhs, rhs);
	merge_state(lhs, rhs);
}

void FilterFileLine::merge_with (file_filters_t const & rhs)
{   
	node_t * const rhs_root = rhs.root;
	if (m_file_filters.root && rhs_root)
		m_file_filters.root->data = rhs_root->data;
	merge(m_file_filters.root, rhs_root);
} 

TreeView * FilterFileLine::getWidgetFile () { return m_ui->treeViewFile; }
TreeView const * FilterFileLine::getWidgetFile () const { return m_ui->treeViewFile; }

void FilterFileLine::setupModelFile ()
{
	if (!m_file_model)
	{
		qDebug("new tree view file model");
		m_file_model = new FilterTreeModel(this, &m_file_filters);

		  //->setFilterBehavior( KSelectionProxyModel::ExactSelection );
		m_proxy_selection = new QItemSelectionModel(m_file_model, this);
		m_file_proxy = new TreeProxyModel(m_file_model, m_proxy_selection);
	}
	getWidgetFile()->setModel(m_file_model);
	getWidgetFile()->syncExpandState();
	getWidgetFile()->hideLinearParents();
	//connect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
}

void FilterFileLine::destroyModelFile ()
{
	if (m_file_model)
	{
		qDebug("destroying file model");
		//disconnect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
		getWidgetFile()->unsetModel(m_file_model);
		delete m_file_model;
		m_file_model = 0;
		delete m_file_proxy;
		m_file_proxy = 0;
		delete m_proxy_selection;
		m_proxy_selection = 0;
	}
}




