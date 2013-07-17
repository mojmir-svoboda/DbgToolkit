#include "connection.h"
#include <QtNetwork>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include "utils.h"
#include "utils_qstandarditem.h"
#include "utils_boost.h"
#include "types.h"
//#include "statswindow.h"
#include "delegates.h"
#include "tableview.h"

Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(0)
	, m_src_stream(e_Stream_TCP)
	, m_src_protocol(e_Proto_TLV)
	, m_app_idx(-1)
	, m_storage_idx(-2)
	, m_recv_bytes(0)
	, m_app_name()

	, m_marked_for_close(false)
	, m_curr_preset()
	, m_tab_widget(0)
	, m_file_tlv_stream(0)
	, m_file_csv_stream(0)

	/*, m_column_setup_done(false)
	, m_last_search_row(0)
	, m_last_search_col(0)
	, m_table_view_widget(0)
	, m_file_model(0)
	, m_file_proxy(0)
	, m_ctx_model(0)
	, m_func_model(0)
	, m_tid_model(0)
	, m_color_regex_model(0)
	, m_regex_model(0)
	, m_lvl_model(0)
	, m_string_model(0)
	, m_table_view_proxy(0)
	, m_table_view_src(0)
	, m_last_clicked()*/

	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_tcp_dump_stream(0)
	, m_tcpstream(0)
	//, m_statswindow(0)
	, m_data_model(0)
{
	qDebug("Connection::Connection() this=0x%08x", this);

	static int counter = 0;
	m_storage_idx = counter;
	++counter;

	/*m_actions.resize(e_action_max_enum_value);
	m_actions[e_action_ToggleRef] = new QAction("Set as reference time", this);
	m_actions[e_action_HidePrev] = new QAction("Hide prev rows", this);
	m_actions[e_action_ExcludeFileLine] = new QAction("Exclude File:Line (x)", this);
	m_actions[e_action_Copy] = new QAction("Copy", this);
	m_actions[e_action_Find] = new QAction("Find File:Line in filters", this);
	m_actions[e_action_ColorTag] = new QAction("Tag row with color", this);
	m_actions[e_action_Setup] = new QAction("Setup", this);
    m_ctx_menu.addAction(m_actions[e_action_ExcludeFileLine]);
    m_ctx_menu.addAction(m_actions[e_action_Find]);
    m_ctx_menu.addAction(m_actions[e_action_Copy]);
    m_ctx_menu.addAction(m_actions[e_action_ToggleRef]);
    m_ctx_menu.addAction(m_actions[e_action_HidePrev]);
    m_ctx_menu.addAction(m_actions[e_action_ColorTag]);
    m_ctx_menu.addSeparator();
    m_ctx_menu.addAction(m_actions[e_action_Setup]);

	m_data_model = new TreeModel(this, &m_session_state.m_data_filters);
	
	m_delegates.get<e_delegate_Level>() = new LevelDelegate(m_session_state, this);
	m_delegates.get<e_delegate_Ctx>() = new CtxDelegate(m_session_state, this);
	m_delegates.get<e_delegate_String>() = new StringDelegate(m_session_state, this);
	m_delegates.get<e_delegate_Regex>() = new RegexDelegate(m_session_state, this);*/
}

namespace {

	template <class ContainerT>
	void destroyDockedWidgets (ContainerT & c, QMainWindow & mainwin)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			if (ptr->m_wd)
			{
				mainwin.removeDockWidget(ptr->m_wd);
				ptr->widget().setParent(0);
				ptr->m_wd->setWidget(0);
				delete ptr->m_wd;
			}
			delete ptr;
		}
		c.clear();
	}
}

struct DestroyDockedWidgets {
	QMainWindow & m_main_window;

	DestroyDockedWidgets (QMainWindow & mw) : m_main_window(mw) { }

	template <typename T>
	void operator() (T & t)
	{
		destroyDockedWidgets(t, m_main_window);
	}
};

Connection::~Connection ()
{
	qDebug("Connection::~Connection() this=0x%08x", this);
	/*if (m_statswindow)
	{
		delete m_statswindow;
		m_statswindow = 0;
	}*/

	recurse(m_data, DestroyDockedWidgets(*m_main_window));

	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();

	/*if (m_file_tlv_stream)
	{
		QDevice * const f = m_file_tlv_stream->device();
		f->close();
		delete m_file_tlv_stream;
		m_file_tlv_stream = 0;
		delete f;
	}*/

/*	if (m_file_csv_stream)
	{
		QIODevice * const f = m_file_csv_stream->device();
		f->close();
		delete m_file_csv_stream;
		m_file_csv_stream = 0;
		delete f;
	}

	destroyModelFile();

	if (m_main_window->getWidgetLvl()->itemDelegate() == m_delegates.get<e_delegate_Level>())
		m_main_window->getWidgetLvl()->setItemDelegate(0);
	if (m_main_window->getWidgetLvl()->model() == m_lvl_model)
		m_main_window->getWidgetLvl()->setModel(0);
	delete m_lvl_model;
	m_lvl_model = 0;
	delete m_delegates.get<e_delegate_Level>();
	m_delegates.get<e_delegate_Level>() = 0;

	if (m_main_window->getWidgetCtx()->itemDelegate() == m_delegates.get<e_delegate_Ctx>())
		m_main_window->getWidgetCtx()->setItemDelegate(0);
	if (m_main_window->getWidgetCtx()->model() == m_ctx_model)
		m_main_window->getWidgetCtx()->setModel(0);
	delete m_ctx_model;
	m_ctx_model = 0;
	delete m_delegates.get<e_delegate_Ctx>();
	m_delegates.get<e_delegate_Ctx>() = 0;

	if (m_main_window->getWidgetTID()->model() == m_tid_model)
		m_main_window->getWidgetTID()->setModel(0);
	delete m_tid_model;
	m_tid_model = 0;

	if (m_main_window->getWidgetColorRegex()->model() == m_color_regex_model)
		m_main_window->getWidgetColorRegex()->setModel(0);
	delete m_color_regex_model;
	m_color_regex_model = 0;

	if (m_main_window->getWidgetRegex()->itemDelegate() == m_delegates.get<e_delegate_Regex>())
		m_main_window->getWidgetRegex()->setItemDelegate(0);
	if (m_main_window->getWidgetRegex()->model() == m_regex_model)
		m_main_window->getWidgetRegex()->setModel(0);
	delete m_regex_model;
	m_regex_model = 0;
	delete m_delegates.get<e_delegate_Regex>();
	m_delegates.get<e_delegate_Regex>() = 0;

	if (m_main_window->getWidgetString()->itemDelegate() == m_delegates.get<e_delegate_String>())
		m_main_window->getWidgetString()->setItemDelegate(0);
	if (m_main_window->getWidgetString()->model() == m_string_model)
		m_main_window->getWidgetString()->setModel(0);
	delete m_string_model;
	m_string_model = 0;
	delete m_delegates.get<e_delegate_String>();
	m_delegates.get<e_delegate_String>() = 0;

	if (m_table_view_proxy)
	{
		m_table_view_proxy->setSourceModel(0);
		delete m_table_view_proxy;
		m_table_view_proxy = 0;
	}

	m_table_view_widget->setModel(0);
	delete m_table_view_src;
	m_table_view_src = 0;

	m_table_view_widget = 0;*/
}

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");
	//if (m_statswindow) m_statswindow->stopUpdate();

	if (m_main_window->dumpModeEnabled())
	{
		QString fname = tr("%1_%2.csv").arg(getAppName()).arg(m_pid);
		exportStorageToCSV(fname);

		Server * server = static_cast<Server *>(parent());
		m_marked_for_close = true;
		QTimer::singleShot(0, server, SLOT(onCloseMarkedTabs()));
	}

	for (dataplots_t::iterator it = m_data.get<e_data_plot>().begin(), ite = m_data.get<e_data_plot>().end(); it != ite; ++it)
	{
		DataPlot * dp = (*it);
		dp->widget().stopUpdate();
	}
}

void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_level, buff, 256);
		e.Encode(TLV(tag_lvl, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

void Connection::onBufferingStateChanged (int val)
{
	bool const buffering_enabled = (val == Qt::Checked) ? true : false;

	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", buffering_enabled);
#else
	int const result = _snprintf_s(tlv_buff, 16, "%u", buffering_enabled);
#endif

	if (result > 0)
	{
		qDebug("Connection::onBufferingStateChanged to_state=%i", buffering_enabled);
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_buffering, buff, 256);
		e.Encode(TLV(tag_bool, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

struct Save {

	QString const & m_path;
	Save (QString const & p) : m_path(p) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().saveConfig(m_path);
	}
};

struct Load {

	QString const & m_path;
	Load (QString const & p) : m_path(p) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->widget().loadConfig(m_path);
	}
};

void Connection::onSaveAll ()
{
	qDebug("%s", __FUNCTION__);
	// @TODO: v hhdr bude 0 !
	
	QString const preset_name = m_curr_preset.isEmpty() ? m_main_window->getValidCurrentPresetName() : m_curr_preset;
	//QString const fname = getDataTagFileName(getConfig().m_appdir, preset_name, g_presetTableTag, tag);
	QString const preset_path = getPresetPath(getConfig().m_appdir, preset_name);
	saveConfigs(preset_path);
}


void Connection::saveConfigs (QString const & path)
{
	recurse(m_data, Save(path));
}

void Connection::loadConfigs (QString const & path)
{
	recurse(m_data, Load(path));
}
