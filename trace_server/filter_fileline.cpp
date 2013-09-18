#include "filter_fileline.h"

FilterFileLine::FilterFileLine (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterFileLine)
	, m_data()
	, m_model(0)
	, m_proxy(0)
	, m_proxy_selection(0)
{
	initUI();
	setupModel();
}

FilterFileLine::~FilterFileLine ()
{
	destroyModel();
	doneUI();
}

void FilterFileLine::initUI ()
{
	m_ui->setupUi(this);
}

void FilterFileLine::doneUI ()
{
}

bool FilterFileLine::accept (DecodedCommand const & cmd) const
{
	QString file, line;
	if (!cmd.getString(tlv::tag_file, file))
		return true;
	if (!cmd.getString(tlv::tag_line, line))
		return true;

	bool excluded = false;
	if (!file.isNull() && !line.isNull() && !file.isEmpty() && !line.isEmpty())
	{
		TreeModelItem ff;
		bool const ff_present = isFileLinePresent(std::make_pair(file, line), ff);
		if (ff_present)
		{
			excluded |= ff.m_state == e_Unchecked;
		}
	}
	return !excluded;
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
void FilterFileLine::setupModel ()
{
	if (!m_model)
	{
		qDebug("new tree view file model");
		m_model = new FilterTreeModel(this, &m_data);

		  //->setFilterBehavior( KSelectionProxyModel::ExactSelection );
		m_proxy_selection = new QItemSelectionModel(m_model, this);
		m_proxy = new TreeProxyModel(m_model, m_proxy_selection);
	}
	getWidgetFile()->setModel(m_model);
	getWidgetFile()->syncExpandState();
	getWidgetFile()->hideLinearParents();
	//connect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
}

void FilterFileLine::destroyModel ()
{
	if (m_model)
	{
		qDebug("destroying file model");
		//disconnect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
		getWidgetFile()->unsetModel(m_model);
		delete m_model;
		m_model = 0;
		delete m_proxy;
		m_proxy = 0;
		delete m_proxy_selection;
		m_proxy_selection = 0;
	}
}


bool FilterFileLine::isFileLinePresent (fileline_t const & item, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_data.is_present(item.first + "/" + item.second, tmp_fi);
	if (exists)
		fi = *tmp_fi;
	return exists;
}
bool FilterFileLine::isFileLinePresent (QString const & fileline, TreeModelItem & fi) const
{
	TreeModelItem const * tmp_fi = 0;
	bool const exists = m_data.is_present(fileline, tmp_fi);
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
						m_data.set_state_to_childs(lhs_child, parent->data);
						break;
					case e_PartialCheck:
						m_data.set_state_to_childs(lhs_child, TreeModelItem(e_Unchecked, 1));
						break;
					case e_Checked:
						m_data.set_state_to_childs(lhs_child, parent->data);
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
	if (m_data.root && rhs_root)
		m_data.root->data = rhs_root->data;
	merge(m_data.root, rhs_root);
} 

TreeView * FilterFileLine::getWidgetFile () { return m_ui->view; }
TreeView const * FilterFileLine::getWidgetFile () const { return m_ui->view; }




