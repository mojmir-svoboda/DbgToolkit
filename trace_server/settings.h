#pragma once
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QObject>
#include "types.h"

/*struct SettingsModelView : QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SettingsModelView (QList<QString> const & app_names, QList<columns_setup_t> const & col_setup);
	virtual ~SettingsModelView ();
	int rowCount (const QModelIndex & parent = QModelIndex()) const;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;
	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags (QModelIndex const & index) const;
	bool setData (QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);

signals:
	
public slots:

public:
	QList<QString> const & m_app_names;
	std::vector<QString> m_rows;
};
*/
class SettingsEditDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	SettingsEditDelegate (QObject * parent = 0);
	QWidget * createEditor (QWidget * parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
	void setEditorData (QWidget * editor, const QModelIndex & index) const;
	void setModelData (QWidget * editor, QAbstractItemModel * model, const QModelIndex &index) const;

private slots:
	void commitAndCloseEditor ();
};

