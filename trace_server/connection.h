#ifndef CONNECTION_H
#define CONNECTION_H

#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTime>
#include <QTimer>
#include <QTableView>
#include <QStandardItemModel>
#include "mainwindow.h"
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include "tls.h"
#include <boost/circular_buffer.hpp>

class Server;
class QFile;
class QDataStream;

struct DecodedCommand : tlv::StringCommand
{
	std::vector<char> orig_message;
	bool written_hdr;
	bool written_payload;

	DecodedCommand () : StringCommand() { Reset(); }

	void Reset ()
	{
		orig_message.clear();
		orig_message.resize(2038);
		written_hdr = written_payload = false;
		hdr.Reset();
		tvs.clear();
	}
};

class Connection : public QTcpSocket
{
	Q_OBJECT
public:
	explicit Connection(QObject *parent = 0);
	~Connection ();
	void setTabWidget (int n) { m_tab_idx = n; }
	void setTableViewWidget (QTableView * w) { m_table_view_widget = w; }
	void setMainWindow (MainWindow * w) { m_main_window = w; }
	void setupColumns (QList<QString> * cs);
	void setupThreadColors (QList<QColor> const & tc);
	void setupModelFile ();
	QList<QString> const * getColumnsSetup() const { return m_columns_setup; }
	QList<QString> * getColumnsSetup() { return m_columns_setup; }
	int findColumn4Tag (tlv::tag_t tag) const;
	void insertColumn4Tag (tlv::tag_t tag, int column_idx);
	void insertColumn ();
	QList<QColor> const & getThreadColors () const { return m_thread_colors; }
	ThreadSpecific & getTLS () { return m_tls; }
	ThreadSpecific const & getTLS () const { return m_tls; }

	typedef std::pair<std::string, std::string> fileline_t;
	typedef file_filter file_filters_t;
	file_filters_t const & getFileFilters () const { return m_file_filters; }
	void appendFileFilter(fileline_t const & item);
	void removeFileFilter(fileline_t const & item);
	bool isFileLineExcluded (fileline_t const & p);
	
signals:
	void readyForUse();
	void newMessage (QString const & from, QString const & message);
	
public slots:
	void onTabTraceFocus (int i);
	void onLevelValueChanged (int i);
	void onApplyFilterClicked ();
private slots:
	void processReadyRead ();
	void onDisconnected ();

private:
	friend class Server;
	bool tryHandleCommand (DecodedCommand const & cmd);
	bool handleLogCommand (DecodedCommand const & cmd);
	bool handleSetupCommand (DecodedCommand const & cmd);
	bool appendToFilters (DecodedCommand const & cmd);
	//int findTreeChild(QTreeView * parent, QString const & text);
	bool setupStorage (QString const & name);
	QString createStorageName () const;
	void processDataStream (QDataStream & stream);
	void exportStorageTo (QString const & filename);
	void closeStorage ();
	template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
	void processStream (T *, T_Ret (T::*read_member_t)(T_Arg0, T_Arg1));


	MainWindow * m_main_window;
	int m_app_idx;
	int m_tab_idx;
	int m_from_file;
	QTableView * m_table_view_widget;
	file_filters_t m_file_filters;
	QStandardItemModel * m_tree_view_file_model;
	QStandardItemModel * m_tree_view_func_model;

	QList<QColor> m_thread_colors;
	QList<QString> * m_columns_setup;
	QMap<tlv::tag_t, int> m_tags2columns;
	//QMap<int, tlv::tag_t> m_columns2tags;
	ThreadSpecific m_tls;
	enum { e_ringbuff_size = 16 * 1024 };
	boost::circular_buffer<char> m_buffer;
	DecodedCommand m_current_cmd;
	tlv::TVDecoder m_decoder;
	QString m_name;
	QFile * m_storage;
	QDataStream * m_datastream;
};

#endif // CONNECTION_H
