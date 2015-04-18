#include "filter_script.h"
#include "constants.h"
#include "serialize.h"
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
#include <QPainter>
//#include <QScriptEngine>

FilterScript::FilterScript (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterScript)
	, m_data()
	, m_se(0)
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
	//m_se = new QScriptEngine();
}

FilterScript::~FilterScript ()
{
	destroyModel();
	doneUI();
}

void FilterScript::initUI ()
{
	m_ui->setupUi(this);
}

void FilterScript::doneUI ()
{
}

bool FilterScript::accept (DecodedCommand const & cmd) const
{
	if (m_data.size() > 0)
	{
		/*QString msg;
		if (!cmd.getString(tlv::tag_msg, msg))
			return true;*/

		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			FilteredScript const & fr = m_data.at(i);
			/*if (fr.match(msg))
			{
				if (!fr.m_is_enabled)
					continue;
				else
				{
					if (fr.m_state)
						return true;
					else
						return false;
				}
			}*/
		}
		return false;
	}
	return true; // no strings at all
}

void FilterScript::defaultConfig ()
{
	m_data.clear();
}

void FilterScript::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterScript::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterScript::applyConfig ()
{
	FilterBase::applyConfig();
}

void FilterScript::clear ()
{
	m_data.clear();
	// @TODO m_name_model.clear();
}


///////////////////
void FilterScript::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	connect(m_ui->scriptNameLineEdit, SIGNAL(returnPressed()), this, SLOT(onScriptAdd()));
	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->view->header()->hide();
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtScriptList(QModelIndex)));
	//connect(m_ui->view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtStringList(QModelIndex)));
	//connect(ui->comboBoxString, SIGNAL(activated(int)), this, SLOT(onStringActivate(int)));
	connect(m_ui->buttonAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->buttonRm, SIGNAL(clicked()), this, SLOT(onRm()));

	//m_ui->view->setItemDelegate(m_delegates.get<e_delegate_String>());
}

void FilterScript::destroyModel ()
{
	if (m_ui->view->itemDelegate())
		m_ui->view->setItemDelegate(0);
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = 0;
	delete m_delegate;
	m_delegate = 0;
}


/*bool FilterScript::isMatchedStringExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredScript const & fr = m_data.at(i);
		if (fr.match(str))
		{
			if (!fr.m_is_enabled)
				return false;
			else
			{
				return fr.m_state ? false : true;
			}
		}
	}
	return false;
}*/
void FilterScript::setScriptState (QString const & s, int state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredScript & fr = m_data[i];
		if (fr.m_name == s)
		{
			fr.m_state = state;
		}
	}
}
void FilterScript::setScriptChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredScript & fr = m_data[i];
		if (fr.m_name == s)
		{
			fr.m_is_enabled = checked;
		}
	}
}
void FilterScript::removeFromScriptFilters (QString const & s)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredScript & fr = m_data[i];
		if (fr.m_name == s)
		{
			m_data.removeAt(i);
			return;
		}
	}
	emitFilterChangedSignal();
}
void FilterScript::appendToScriptFilters (QString const & s, bool enabled, int state)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_name == s)
			return;
	m_data.push_back(FilteredScript(s, enabled, state));
	emitFilterChangedSignal();
}

void FilterScript::appendToScriptWidgets (FilteredScript const & flt)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_name);
	if (child == 0)
	{
		bool const mode = static_cast<bool>(flt.m_state);
		QList<QStandardItem *> row_items = addTriRow(flt.m_name, flt.m_is_enabled ? Qt::Checked : Qt::Unchecked, mode);
		row_items[0]->setCheckState(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked);
		root->appendRow(row_items);
	}
}


void FilterScript::onClickedAtScriptList (QModelIndex idx)
{
	if (!idx.isValid())
		return;

	if (idx.column() == 1)
	{
		QString const & filter_item = m_model->data(m_model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		QString const & mod = m_model->data(idx, Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);
		size_t const i = (curr + 1) % e_max_fltmod_enum_value;
		E_FilterMode const new_mode = static_cast<E_FilterMode>(i);
		m_model->setData(idx, QString(fltModToString(new_mode)));

		bool const is_inclusive = new_mode == e_Include;
		setScriptState(filter_item, is_inclusive);
		recompile();
		emitFilterChangedSignal();
	}
	else
	{
		QStandardItem * item = m_model->itemFromIndex(idx);
		Q_ASSERT(item);
		bool const orig_checked = (item->checkState() == Qt::Checked);
		Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		item->setCheckState(checked);

		QString const & mod = m_model->data(m_model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);
		bool const is_inclusive = curr == e_Include;
		QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
		// @TODO: if state really changed
		setScriptState(val, is_inclusive);
		setScriptChecked(val, checked);
		recompile();
		emitFilterChangedSignal();
	}
}

void FilterScript::recompile ()
{ }

void FilterScript::onScriptRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;
	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());
	removeFromScriptFilters(val);
	recompile();
	emitFilterChangedSignal();
}

void FilterScript::onScriptAdd ()
{
	QString const qItem = m_ui->scriptNameLineEdit->text();

	if (!qItem.length())
		return;
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addTriRow(qItem, Qt::Checked, true);
		root->appendRow(row_items);
		appendToScriptFilters(qItem, true, true);
		row_items[0]->setCheckState(Qt::Checked);
		recompile();
	}
}

void FilterScript::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}

//////// delegate
void ScriptDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
	painter->save();
	QStyleOptionViewItemV4 option4 = option;
	initStyleOption(&option4, index);

	if (index.column() == 1)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();

			E_FilterMode const mode = stringToFltMod(qs.at(0).toLatin1());
			QString const verbose = fltmodsStr[mode];
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}
	}
	else
		QStyledItemDelegate::paint(painter, option4, index);
	painter->restore();
}





