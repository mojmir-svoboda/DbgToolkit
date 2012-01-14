#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <QAbstractTableModel>
#include <QString>
#include <vector>
#include "../tlv_parser/tlv_parser.h"

class Connection;

class ModelView : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit ModelView(QObject *parent = 0, Connection * c = 0);
	int rowCount (const QModelIndex & parent = QModelIndex()) const ;
	int columnCount (const QModelIndex & parent = QModelIndex()) const;
	QVariant data (const QModelIndex & index, int role = Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role) const;

	void transactionStart ();
	void appendCommand (tlv::StringCommand const & cmd, bool & excluded);
	void transactionCommit ();

	bool checkExistence (QModelIndex const & index) const;
	bool checkColumnExistence (tlv::tag_t tag, QModelIndex const & index) const;

signals:
	
public slots:

private:

	Connection * m_connection;
	typedef std::vector<QString> columns_t;
	typedef std::vector<columns_t> rows_t;
	rows_t m_rows;
};

#endif // MODELVIEW_H
