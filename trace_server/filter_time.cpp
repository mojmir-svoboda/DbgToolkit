#include "filter_time.h"
#include "constants.h"
#include "serialize.h"
#include "utils_qstandarditem.h"
#include <boost/function.hpp>
#include <QPainter>

namespace {
    bool lesser (float lhs, float rhs) { return lhs < rhs; }
    bool lesser_or_eq (float lhs, float rhs) { return lhs <= rhs; }
    bool eq (float lhs, float rhs) { return lhs == rhs; }
    bool greater_or_eq (float lhs, float rhs) { return lhs >= rhs; }
    bool greater (float lhs, float rhs) { return lhs > rhs; }
    bool not_eq (float lhs, float rhs) { return lhs != rhs; }
}

typedef bool (*cmp_t)(float, float);

cmp_t comparators[e_max_cmpmod_enum_value] = { lesser, lesser_or_eq, eq, greater_or_eq, greater, not_eq };
QString comparatorsStr[e_max_cmpmod_enum_value] = { "<", "<=", "==", ">=", ">", "!=" };
inline QString cmpModToString (E_CmpMode l) { return comparatorsStr[l]; }
inline E_CmpMode stringToCmpMod (QString const & qstr) {
	for (size_t i = 0; i < e_max_cmpmod_enum_value; ++i)
		if (qstr == comparatorsStr[i])
			return static_cast<E_CmpMode>(i);
	return e_CmpL;
}

bool FilteredTime::match (float f) const
{
    bool const res = comparators[m_operator](f, m_value);
    return res;
}

FilteredTime::FilteredTime (QString const & op, QString const & rhs, QString const & units, bool enabled)
    : m_string(rhs)
    , m_time_units_str(units)
    , m_src_value(rhs.toULongLong())
    , m_value(static_cast<float>(m_src_value))
    , m_time_units(stringToUnitsValue(units))
    , m_is_enabled(enabled)
    , m_operator(stringToCmpMod(op))
{ }

FilteredTime::FilteredTime (QString const & op, QString const & rhs, QString const & units)
    : m_string(rhs)
    , m_time_units_str(units)
    , m_src_value(rhs.toULongLong())
    , m_value(static_cast<float>(m_src_value))
    , m_time_units(stringToUnitsValue(units))
    , m_is_enabled(true)
    , m_operator(stringToCmpMod(op))
{ }

FilterTime::FilterTime (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_FilterTime)
	, m_data()
	, m_model(0)
	, m_delegate(0)
{
	initUI();
	setupModel();
}

FilterTime::~FilterTime ()
{
	destroyModel();
	doneUI();
}

void FilterTime::initUI ()
{
	m_ui->setupUi(this);
}

void FilterTime::doneUI ()
{
}

bool FilterTime::accept (DecodedCommand const & cmd) const
{
	if (m_data.size() > 0)
	{
		unsigned long long t;
        QString qt;
		if (!cmd.getString(tlv::tag_time, qt))
			return true;
        t = qt.toULongLong();

        bool accepted = true;
		for (int i = 0, ie = m_data.size(); i < ie; ++i)
		{
			FilteredTime const & f = m_data.at(i);
            if (!f.m_is_enabled)
                continue;

			accepted &= f.match(t);
		}
		return accepted;
	}
	return true; // no filters at all
}

void FilterTime::defaultConfig ()
{
	m_data.clear();
}

void FilterTime::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void FilterTime::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_filterTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void FilterTime::applyConfig ()
{
	FilterBase::applyConfig();
}

void FilterTime::clear ()
{
	m_data.clear();
	// @TODO m_string_model.clear();
}


///////////////////
void FilterTime::setupModel ()
{
	if (!m_model)
		m_model = new QStandardItemModel;
	m_ui->view->setModel(m_model);
	connect(m_ui->qFilterLineEdit, SIGNAL(returnPressed()), this, SLOT(onAdd()));
	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->view->header()->hide();
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	//connect(m_ui->view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedAtStringList(QModelIndex)));
	//connect(ui->comboBoxString, SIGNAL(activated(int)), this, SLOT(onStringActivate(int)));
	connect(m_ui->buttonAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->buttonRm, SIGNAL(clicked()), this, SLOT(onRm()));

	//m_ui->view->setItemDelegate(m_delegates.get<e_delegate_String>());
}

void FilterTime::destroyModel ()
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


/*bool FilterTime::isMatchedExcluded (QString str) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredString const & fr = m_data.at(i);
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
void FilterTime::setOperator (QString const & s, int op)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredTime & f = m_data[i];
		if (f.m_string == s)
		{
			f.m_operator = op;
		}
	}
}
void FilterTime::setChecked (QString const & s, bool checked)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		FilteredTime & f = m_data[i];
		if (f.m_string == s)
		{
			f.m_is_enabled = checked;
		}
	}
}
void FilterTime::remove (QString const & op, QString const & s, QString const & u)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		if (m_data[i] == FilteredTime(op, s, u))
		{
			m_data.removeAt(i);
			return;
		}
	}
	emitFilterChangedSignal();
}
void FilterTime::append (QString const & op, QString const & s, QString const & units, bool enabled)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i] == FilteredTime(op, s, units, enabled))
			return;
	m_data.push_back(FilteredTime(op, s, units, enabled));
	emitFilterChangedSignal();
}

void FilterTime::appendToWidgets (FilteredTime const & flt)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, flt.m_string);
	if (child == 0)
	{
        QString const & m = cmpModToString(static_cast<E_CmpMode>(flt.m_operator));
		QList<QStandardItem *> row_items = add4Col(flt.m_is_enabled ? Qt::Checked : Qt::Unchecked
                , get_tag_name(tlv::tag_time)
                , m
                , flt.m_string
                , flt.m_time_units_str);
		root->appendRow(row_items);
	}
}


void FilterTime::onClickedAt (QModelIndex idx)
{
	if (!idx.isValid())
		return;

	if (idx.column() == 1)
	{
		QString const & filter_item = m_model->data(m_model->index(idx.row(), 0, QModelIndex()), Qt::DisplayRole).toString();
		QString const & mod = m_model->data(idx, Qt::DisplayRole).toString();
        
		E_CmpMode const curr = stringToCmpMod(mod);
		size_t const i = (curr + 1) % e_max_cmpmod_enum_value;
		E_CmpMode const new_mode = static_cast<E_CmpMode>(i);
		m_model->setData(idx, QString(cmpModToString(new_mode)));

		setOperator(filter_item, new_mode);
		recompile();
		emitFilterChangedSignal();
	}
	else
	{
		QStandardItem * item = m_model->itemFromIndex(idx);
		Q_ASSERT(item);
		bool const orig_checked = (item->checkState() == Qt::Checked);
		//Qt::CheckState const checked = orig_checked ? Qt::Unchecked : Qt::Checked;
		//item->setCheckState(checked);

		//QString const & mod = m_model->data(m_model->index(idx.row(), 1, QModelIndex()), Qt::DisplayRole).toString();
		//E_FilterMode const curr = stringToFltMod(mod.toStdString().c_str()[0]);
		//bool const is_inclusive = curr == e_Include;
		///String const & val = m_model->data(idx, Qt::DisplayRole).toString();
		// @TODO: if state really changed
		//setStringState(val, is_inclusive);
		//setStringChecked(val, checked);
		//recompile();
		//emitFilterChangedSignal();
	}
}

void FilterTime::recompile ()
{ }

void FilterTime::onRm ()
{
	QModelIndex const idx = m_ui->view->currentIndex();
    int const row = idx.row();

    QModelIndex const op_idx = m_model->index(row, 1, QModelIndex());
    QModelIndex const s_idx = m_model->index(row, 2, QModelIndex());
    QModelIndex const u_idx = m_model->index(row, 3, QModelIndex());

	QStandardItem * op_item = m_model->itemFromIndex(op_idx);
	if (!op_item)
		return;
	QStandardItem * s_item = m_model->itemFromIndex(s_idx);
	if (!s_item)
		return;
	QStandardItem * u_item = m_model->itemFromIndex(u_idx);
	if (!u_item)
		return;

	QString const & op_val = m_model->data(op_idx, Qt::DisplayRole).toString();
	QString const & s_val = m_model->data(s_idx, Qt::DisplayRole).toString();
	QString const & u_val = m_model->data(u_idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());
	remove(op_val, s_val, u_val);
	recompile();
	emitFilterChangedSignal();
}

void FilterTime::onAdd (QString const & op, QString const & rhs, QString const & units)
{
	if (!op.length() && !rhs.length())
		return;
	QStandardItem * root = m_model->invisibleRootItem();

    for (int i = 0, ie = m_model->rowCount(); i < ie; ++i)
    {
        QStandardItem * op_item = m_model->item(i, 1);
        if (!op_item)
            return;
        QStandardItem * s_item = m_model->item(i, 2);
        if (!s_item)
            return;
        QStandardItem * u_item = m_model->item(i, 3);
        if (!u_item)
            return;

        QString const & op_val = op_item->text();
        QString const & s_val = s_item->text();
        QString const & u_val = u_item->text();

        if (FilteredTime(op, rhs, units) == FilteredTime(op_val, s_val, u_val))
        {
            QList<QStandardItem *> row_items = add4Col(Qt::Checked, get_tag_name(tlv::tag_time), op_val, s_val, u_val);
            root->appendRow(row_items);
        }
    }
    append(op, rhs, units, true);
}
void FilterTime::onAdd ()
{
	QString const qOp = m_ui->opBox->currentText();
	QString const qItem = m_ui->qFilterLineEdit->text();
	QString const qUnits = m_ui->timeComboBox->comboBox()->currentText();
    onAdd(qOp, qItem, qUnits);
}

void FilterTime::locateItem (QString const & item, bool scrollto, bool expand)
{
	/*QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}*/
}

//////// delegate
void TimeDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
{
    painter->save();
    QStyleOptionViewItemV4 option4 = option;
    initStyleOption(&option4, index);

            // TODO: units
	/*if (index.column() == 0)
	{
		QVariant value = index.data(Qt::DisplayRole);
		if (value.isValid() && !value.isNull())
		{
			QString const qs = value.toString();
    
			E_CmpMode const mode = stringToCmpMod(qs);
			QString const verbose = cmpModToString(mode);
			option4.text = verbose;

			if (QWidget const * widget = option4.widget)
			{
				QStyle * style = widget->style();
				style->drawControl(QStyle::CE_ItemViewItem, &option4, painter, widget);
			}
		}

	}
	else
		QStyledItemDelegate::paint(painter, option4, index);*/
	painter->restore();
}





