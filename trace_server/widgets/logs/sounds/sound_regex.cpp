#include "sound_regex.h"
#include "constants.h"
#include <serialize/serialize.h>
#include <QPainter>
#include <utils/utils_qstandarditem.h>
#include <utils/utils_widgets.h>
#include <wavetable.h>

SoundRegex::SoundRegex (QWidget * parent)
	: FilterBase(parent)
	, m_ui(new Ui_SoundRegex)
	, m_data()
	, m_model(nullptr)
	, m_wavetable(nullptr)
{
	initUI();
	setupModel();
}

SoundRegex::~SoundRegex ()
{
	destroyModel();
	doneUI();
}

void SoundRegex::initUI ()
{
	m_ui->setupUi(this);
}

void SoundRegex::doneUI ()
{
}

//@TODO: dedup
void SoundRegex::actionNotify (QModelIndex const & sourceIndex, SoundNotif const & ct) const
{
	if (!m_wavetable) return;

	QAbstractItemModel const * const src_model = sourceIndex.model();
	int const col_count = src_model->columnCount();

	for (int c = 0; c < col_count; ++c)
	{
		if (!ct.m_where.m_states[c])
			continue;
		if (!ct.m_is_enabled)
			continue;

		QModelIndex const idx = src_model->index(sourceIndex.row(), c, QModelIndex());
		if (!idx.isValid())
			continue;
		QVariant data = src_model->data(idx, Qt::DisplayRole);

		if (bool const is_match = ct.accept(data.toString()))
		{
			m_wavetable->play(ct.m_waveconfig.m_name, 1.0f, 1);
		}
	}
}

bool SoundRegex::action (QModelIndex const & sourceIndex)
{
	for (size_t i = 0, ie = m_data.size(); i < ie; ++i)
	{
		SoundNotif const & ct = m_data[i];
		actionNotify(sourceIndex, ct);
	}
	return false;
}
// 
// bool SoundRegex::accept (QModelIndex const & sourceIndex) const
// {
// 	return true;
// }

void SoundRegex::defaultConfig ()
{
	m_data.clear();
}

void SoundRegex::loadConfig (QString const & path)
{
	QString const fname = path + "/" + g_soundTag + "/" + typeName();
	if (!::loadConfigTemplate(*this, fname))
		defaultConfig();
}

void SoundRegex::saveConfig (QString const & path)
{
	QString const fname = path + "/" + g_soundTag + "/" + typeName();
	::saveConfigTemplate(*this, fname);
}

void SoundRegex::setConfigToUI ()
{
	m_model->clear();
	m_ui->waveComboBox->clear();
	WaveConfig tmp;
	setValuesToUI(m_ui->waveComboBox, m_wavetable->m_config, tmp);
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		addRowToUI(m_data[i]);
		setupModelHeader();
	}
}

void SoundRegex::applyConfig ()
{
	FilterBase::applyConfig();
	setConfigToUI();
	recompileNotifRegexps();
}


void SoundRegex::clear ()
{
	onSelectNone();
	//m_data.clear();
}


///////// sound

enum E_ColOrder
{
	e_ColOrder_Data = 0,
	e_ColOrder_aA,
	e_ColOrder_ww,
	e_ColOrder_useRegExp,
	e_ColOrder_wav,
	e_ColOrder_where,
	e_ColOrder_status
};

void SoundRegex::setupModelHeader ()
{
	if (m_model->rowCount() == 1)
	{
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_Data, Qt::Horizontal, "RegExp");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_aA, Qt::Horizontal, "aA");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_ww, Qt::Horizontal, "ww");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_useRegExp, Qt::Horizontal, ".*");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_wav, Qt::Horizontal, "wav");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_where, Qt::Horizontal, "where");
		m_ui->view->model()->setHeaderData(E_ColOrder::e_ColOrder_status, Qt::Horizontal, "state");
		m_model->setColumnCount(7);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_Data, 192);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_aA, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_ww, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_useRegExp, 32);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_wav, 128);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_where, 128);
		m_ui->view->setColumnWidth(E_ColOrder::e_ColOrder_status, 64);
	}
}
void SoundRegex::setupModel ()
{
	if (!m_model)
	{
		m_model = new QStandardItemModel;
	}
	m_ui->view->setModel(m_model);
	m_ui->view->expandAll();

	m_ui->view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(m_ui->addButton, SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(m_ui->comboBox->lineEdit(), SIGNAL(returnPressed()), this, SLOT(onAdd()));
	connect(m_ui->rmButton, SIGNAL(clicked()), this, SLOT(onRm()));
	connect(m_ui->view, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAt(QModelIndex)));
	connect(m_ui->allButton, SIGNAL(clicked()), this, SLOT(onSelectAll()));
	connect(m_ui->noneButton, SIGNAL(clicked()), this, SLOT(onSelectNone()));
	connect(m_model, SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)), this, SLOT(onDataChanged(QModelIndex const &, QModelIndex const &)));

	setupModelHeader();
}

void SoundRegex::destroyModel ()
{
	if (m_ui->view->model() == m_model)
		m_ui->view->setModel(0);
	delete m_model;
	m_model = nullptr;
}

SoundNotif const * SoundRegex::findMatch (QString const & item) const
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data.at(i).m_regex_str == item)
		{
			return &m_data.at(i);
		}
	return 0;
}

// void SoundRegex::append (QString const & item)
// {
// 	for (int i = 0, ie = m_data.size(); i < ie; ++i)
// 		if (m_data[i].m_regex_str == item)
// 		{
// 			SoundNotif & ct = m_data[i];
// 			ct.m_is_enabled = true;
// 			return;
// 		}
// 	m_data.push_back(SoundNotif(false, true, false, item, m_cccfg, WaveConfig()));
// }
void SoundRegex::remove (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			SoundNotif & item = m_data[i];
			item.m_is_enabled = false;
			return;
		}
}
SoundNotif & SoundRegex::findOrCreateSoundNotif (QString const & item)
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
		if (m_data[i].m_regex_str == item)
		{
			SoundNotif & ct = m_data[i];
			return ct;
		}
	m_data.push_back(SoundNotif(false, true, false, item, m_cccfg, WaveConfig()));
	return m_data.back();
}

//////// slots
void SoundRegex::onSelectAll ()
{
	//auto fn = &SoundRegex::append;
	//applyFnOnAllChildren(fn, this, m_model, Qt::Checked);
	//emitFilterChangedSignal();
}

void SoundRegex::onSelectNone ()
{
	//auto fn = &SoundRegex::remove;
	//applyFnOnAllChildren(fn, this, m_model, Qt::Unchecked);
	//emitFilterChangedSignal();
}

void SoundRegex::onClickedAt (QModelIndex idx)
{

}

void SoundRegex::onDataChanged (QModelIndex const & idx, QModelIndex const & parent)
{
	QAbstractItemModel const * const m = idx.model();
	int const r = idx.row();
	switch (idx.column())
	{
		case E_ColOrder::e_ColOrder_Data:
		{
			QStandardItem * item = m_model->itemFromIndex(idx);
			Q_ASSERT(item);

			QString const & str = m_model->data(idx, Qt::DisplayRole).toString();
			bool const checked = (item->checkState() == Qt::Checked);
			SoundNotif & ct = findOrCreateSoundNotif(item->text());
			if (checked)
			{
				recompileNotifRegex(ct);
				//updateNotif(ct);
			}
			else
			{
				recompileNotifRegex(ct);
				//updateColor(ct);
				remove(str);
			}

			emitFilterChangedSignal();
			break;
		}
		case E_ColOrder::e_ColOrder_aA:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_case_sensitive = checked;
			break;
		}
		case E_ColOrder::e_ColOrder_ww:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_whole_word = checked;
			break;
		}
		case E_ColOrder::e_ColOrder_useRegExp:
		{
			QVariant val = m->data(idx, Qt::CheckStateRole);
			bool const checked = val == Qt::Checked;
			m_data[r].m_is_regex = checked;
			SoundNotif & ct = m_data[r];
			recompileNotifRegex(ct);
			break;
		}
		case E_ColOrder::e_ColOrder_wav:
			break;
		case E_ColOrder::e_ColOrder_where:
			break;
		case E_ColOrder::e_ColOrder_status:
			break;
	}
}

void SoundRegex::recompile ()
{ }


/*void SoundRegex::locateItem (QString const & item, bool scrollto, bool expand)
{
	QModelIndexList indexList = m_model->match(m_model->index(0, 0), Qt::DisplayRole, item);
	if (!indexList.empty())
	{
		QModelIndex const selectedIndex(indexList.first());
		getWidget()->setCurrentIndex(selectedIndex);
	}
}*/

void SoundRegex::onActivate (int)
{
}

void SoundRegex::recompileNotifRegex (SoundNotif & ct)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QString const str = ct.m_regex_str;
	QStandardItem * child = findChildByText(root, str);
	if (ct.m_is_regex)
	{
		QModelIndex const idx = m_model->indexFromItem(child);
		ct.m_is_enabled = false;
		if (!child)
			return;

		QRegularExpression regex(str);
		QString reason;
		if (regex.isValid())
		{
			ct.m_regex = regex;

			bool const checked = (child->checkState() == Qt::Checked);
			if (child && checked)
			{
				child->setData(QBrush(Qt::green), Qt::BackgroundRole);
				reason = "ok";
				ct.m_is_enabled = true;
			}
			else if (child && !checked)
			{
				child->setData(QBrush(Qt::yellow), Qt::BackgroundRole);
				reason = "not checked";
			}
		}
		else
		{
			if (child)
			{
				child->setData(QBrush(Qt::red), Qt::BackgroundRole);
				reason = regex.errorString();
			}
		}

		child->setToolTip(reason);
		QStandardItem * item = m_model->item(child->row(), e_ColOrder_status);
		item->setText(reason);
	}
	else
	{
		//child->setData(QBrush(Qt::green), Qt::BackgroundRole);
		//QString const reason = "ok";

		//child->setToolTip(reason);
		//QStandardItem * item = m_model->item(child->row(), e_ColOrder_status);
		//item->setText(reason);

		ct.m_is_enabled = true;
	}
}

void SoundRegex::recompileNotifRegexps ()
{
	for (int i = 0, ie = m_data.size(); i < ie; ++i)
	{
		SoundNotif & ct = m_data[i];
		recompileNotifRegex(ct);
		//updateColor(ct);
	}
}
//void SoundRegex::onDoubleClickedAtColorRegexList (QModelIndex idx) { }

void SoundRegex::addRowToUI (SoundNotif const & cfg)
{
	int const rows = m_model->rowCount();
	int const row = rows;

	QStandardItem * const name_item = new QStandardItem(cfg.m_regex_str);
	name_item->setCheckable(true);
	name_item->setCheckState(true ? Qt::Checked : Qt::Unchecked);
	QStandardItem * aAitem = new QStandardItem;
	aAitem->setCheckable(true);
	aAitem->setCheckState(cfg.m_case_sensitive ? Qt::Checked : Qt::Unchecked);
	QStandardItem * wwitem = new QStandardItem;
	wwitem->setCheckable(true);
	wwitem->setCheckState(cfg.m_whole_word ? Qt::Checked : Qt::Unchecked);
	QStandardItem * regexitem = new QStandardItem;
	regexitem->setCheckable(true);
	regexitem->setCheckState(cfg.m_is_regex ? Qt::Checked : Qt::Unchecked);
	QStandardItem * wavitem = new QStandardItem;
	QStandardItem * whereitem = new QStandardItem;
	QStandardItem * stitem = new QStandardItem;
	stitem->setCheckable(false);

	m_model->setItem(row, E_ColOrder::e_ColOrder_Data, name_item);
	m_model->setItem(row, E_ColOrder::e_ColOrder_aA, aAitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_ww, wwitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_useRegExp, regexitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_wav, wavitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_where, whereitem);
	m_model->setItem(row, E_ColOrder::e_ColOrder_status, stitem);

	QModelIndex const idx = m_model->indexFromItem(whereitem);
	QComboBox * cb = new QComboBox;
	m_ui->view->setIndexWidget(idx, cb);

	QModelIndex const wavidx = m_model->indexFromItem(wavitem);
	QComboBox * wavcb = new QComboBox;
	m_ui->view->setIndexWidget(wavidx, wavcb);
	WaveConfig wc;
	setUIValuesToConfig(m_ui->waveComboBox, m_wavetable->m_config, wc);
	setValuesToUI(wavcb, m_wavetable->m_config, wc);
	setValuesToUI(cb, m_cccfg);
}

SoundNotif & SoundRegex::add (QString const & regex)
{
	QStandardItem * root = m_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, regex);
	if (child == 0)
	{
		SoundNotif & cfg = findOrCreateSoundNotif(regex);

		cfg.m_is_enabled = true;
		cfg.m_case_sensitive = m_ui->aAButton->isChecked();
		cfg.m_whole_word = m_ui->wwButton->isChecked();
		cfg.m_is_regex = m_ui->regexButton->isChecked();
		setUIValuesToConfig(m_ui->waveComboBox, m_wavetable->m_config, cfg.m_waveconfig);
		setUIValuesToConfig(m_ui->whereComboBox, cfg.m_where);
		recompileNotifRegex(cfg);

		addRowToUI(cfg);
		return cfg;
	}
	else
	{
		SoundNotif & ct = findOrCreateSoundNotif(regex);
		return ct;
	}
}

void SoundRegex::onAdd ()
{
	QString const qItem = m_ui->comboBox->currentText();
	if (!qItem.length())
		return;

	SoundNotif & ct = add(qItem);
	setupModelHeader();
}

void SoundRegex::onRm ()
{
	QModelIndex const idx_curr = m_ui->view->currentIndex();
	int const row = idx_curr.row();

	QModelIndex const idx = m_model->index(row, e_ColOrder_Data, QModelIndex());
	QStandardItem * item = m_model->itemFromIndex(idx);
	if (!item)
		return;

	QString const & val = m_model->data(idx, Qt::DisplayRole).toString();
	m_model->removeRow(idx.row());

	m_data.erase(m_data.begin() + row);
}

void SoundRegex::setDefaultSearchConfig (CheckedComboBoxConfig const & cccfg)
{
	m_cccfg = cccfg;
	setValuesToUI(m_ui->whereComboBox, cccfg);
}

