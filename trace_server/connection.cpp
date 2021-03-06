#include "connection.h"
#include <QtNetwork>
#include <utils/utils.h>
#include <utils/utils_boost.h>
#include "types.h"
#include <server/server.h>
#include "mainwindow.h"
#include <serialize/serialize.h>
#include <widgets/controlbar/controlbarcommon.h>
#include <ui_controlbarcommon.h>
#include <widgets/mixer.h>
#include "ui_mixer.h"
#include <widgets/controlbar/controlwidgetdata.h>
#include <ui_controlwidgetdata.h>
#include "wavetable.h"

GlobalConfig const & Connection::getGlobalConfig () const { return m_main_window->getConfig(); }

Connection::Connection (QString const & app_name, QObject * parent)
	: QThread(parent)
	, ActionAble(QStringList(qobject_cast<MainWindow *>(parent)->dockManager().path()) << app_name)
	, m_app_name(app_name)
	, m_main_window(qobject_cast<MainWindow *>(parent))
	, m_src_stream(e_Stream_TCP)
	, m_src_protocol(e_Proto_ASN1)
	, m_config()
	, m_app_data()
	, m_storage_idx(-2)
	, m_marked_for_close(false)
	, m_curr_preset()
	, m_control_bar(new ControlBarCommon)
	, m_mixer(new Mixer(nullptr, m_control_bar->ui->mixerButton, 0, 0))
	, m_data_control(new ControlWidgetData)
	, m_file_tlv_stream(nullptr)
	, m_file_csv_stream(nullptr)
	, m_file_size(0)
	, m_current_cmd()
	, m_storage(nullptr)
// 	, m_availableAudioOutputDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
// 	, m_audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice())
// 	, m_audioOutput(nullptr)
	, m_wavetable(new WaveTable)
	, m_tcp_dump_stream(nullptr)
	, m_tcpstream(nullptr)
{
	qDebug("Connection::Connection() this=0x%08x", this);

	m_asn1_allocator.resizeStorage(64 * 1024);

	initSound();

	m_config.m_auto_scroll = getGlobalConfig().m_auto_scroll;
	m_config.m_mixer = getGlobalConfig().m_mixer;
	m_config.m_buffered = getGlobalConfig().m_buffered;
	m_config.m_time_units_str = getGlobalConfig().m_time_units_str;
	m_config.m_time_units = getGlobalConfig().m_time_units;
	m_config.m_font = getGlobalConfig().m_font;
	m_config.m_fontsize = getGlobalConfig().m_fontsize;
	m_config.m_logs_recv_level = getGlobalConfig().m_logs_recv_level;
	m_config.m_plots_recv_level = getGlobalConfig().m_plots_recv_level;
	m_config.m_tables_recv_level = getGlobalConfig().m_tables_recv_level;
	m_config.m_gantts_recv_level = getGlobalConfig().m_gantts_recv_level;

	static int counter = 0;
	m_storage_idx = counter;
	++counter;

	setConfigValuesToUI(m_config);

	connect(m_control_bar->ui->mixerButton, SIGNAL(clicked()), this, SLOT(onMixerButton()));
	connect(m_control_bar->ui->dataControlButton, SIGNAL(clicked()), this, SLOT(onDataControlButton()));
	//connect(m_control_bar->ui->levelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLevelValueChanged(int)));
	connect(m_control_bar->ui->buffCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onBufferingStateChanged(int)));
	connect(m_control_bar->ui->presetComboBox, SIGNAL(activated(int)), this, SLOT(onPresetChanged(int)));
	connect(m_control_bar->ui->activatePresetButton, SIGNAL(clicked()), this, SLOT(onPresetApply()));
	connect(m_control_bar->ui->presetSaveButton, SIGNAL(clicked()), this, SLOT(onPresetSave()));
	connect(m_control_bar->ui->presetAddButton, SIGNAL(clicked()), this, SLOT(onPresetAdd()));
	connect(m_control_bar->ui->presetRmButton, SIGNAL(clicked()), this, SLOT(onPresetRm()));
	//connect(m_control_bar->ui->presetResetButton, SIGNAL(clicked()), this, SLOT(onPresetReset()));
	connect(m_data_control->ui->logSlider, SIGNAL(valueChanged(int)), this, SLOT(onLogsStateChanged(int)));
	connect(m_data_control->ui->plotSlider, SIGNAL(valueChanged(int)), this, SLOT(onPlotsStateChanged(int)));
	//connect(m_control_bar->ui->tableSlider, SIGNAL(valueChanged(int)), this, SLOT(onTablesStateChanged(int)));
	connect(m_data_control->ui->ganttSlider, SIGNAL(valueChanged(int)), this, SLOT(onGanttsStateChanged(int)));
	connect(m_control_bar->ui->clrDataButton, SIGNAL(clicked()), this, SLOT(onClearAllData()));

	m_mixer->setupMixer(m_config.m_mixer);
	connect(m_mixer.get(), SIGNAL(mixerChanged()), this, SLOT(onMixerApplied()));
	connect(this, SIGNAL(dictionaryArrived(int, Dict const &)), m_mixer.get(), SLOT(onDictionaryArrived(int, Dict const &)));
}

namespace {

	template <class ContainerT>
	void unregisterDockedWidgets (ContainerT & c, MainWindow & mainwin)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			mainwin.dockManager().removeActionAble(*ptr);
		}
	}
}

struct UnregisterDockedWidgets {
	MainWindow & m_main_window;

	UnregisterDockedWidgets (MainWindow & mw) : m_main_window(mw) { }

	template <typename T>
	void operator() (T & t)
	{
		m_main_window.dockManager().removeActionAble(t);
		unregisterDockedWidgets(t, m_main_window);
	}
};

void Connection::destroyDockedWidget (DockedWidgetBase * dwb)
{
	m_main_window->dockManager().removeActionAble(*dwb);
	switch (dwb->type())
	{
		case e_data_log: removeDockedWidget<e_data_log>(dwb); break;
		case e_data_plot: removeDockedWidget<e_data_plot>(dwb); break;
		case e_data_table: removeDockedWidget<e_data_table>(dwb); break;
		case e_data_gantt: removeDockedWidget<e_data_gantt>(dwb); break;
		case e_data_frame: removeDockedWidget<e_data_frame>(dwb); break;
		default: break;
	}
}

namespace {
	template <class ContainerT>
	void destroyDockedWidgets (ContainerT & c, MainWindow & mw, Connection & conn)
	{
		for (typename ContainerT::iterator it = c.begin(), ite = c.end(); it != ite; ++it)
		{
			typename ContainerT::iterator::value_type ptr = *it;
			ptr->setParent(0);
			delete ptr;
		}
		c.clear();
	}
}

struct DestroyDockedWidgets {
	MainWindow & m_main_window;
	Connection & m_conn;

	DestroyDockedWidgets (MainWindow & mw, Connection & conn) : m_main_window(mw), m_conn(conn) { }

	template <typename T>
	void operator() (T & t)
	{
		destroyDockedWidgets(t, m_main_window, m_conn);
	}
};

Connection::~Connection ()
{
	qDebug("Connection::~Connection() this=0x%08x", this);

	m_main_window->dockManager().removeActionAble(*this);
	recurse(m_data_widgets, UnregisterDockedWidgets(*m_main_window));

	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();

	qDebug("destroying docked widgets");
	recurse(m_data_widgets, DestroyDockedWidgets(*m_main_window, *this));

	if (m_file_tlv_stream)
	{
		QIODevice * const f = m_file_tlv_stream->device();
		f->close();
		delete m_file_tlv_stream;
		m_file_tlv_stream = 0;
		delete f;
	}

	if (m_file_csv_stream)
	{
		QIODevice * const f = m_file_csv_stream->device();
		f->close();
		delete m_file_csv_stream;
		m_file_csv_stream = 0;
		delete f;
	}

	delete m_tcpstream;
}

void Connection::mkWidgetPath (E_DataWidgetType type, QString const tag, QStringList & path)
{
	char const * widget_prefix = g_fileTags[type];
	QString const & name0 = m_main_window->dockedName();
	QString const & name1 = getAppName();
	QString const & name2 = widget_prefix;
	QString const & name3 = tag;
	path.append(name0);
	path.append(name1);
	path.append(name2);
	path.append(name3);
}

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");

	if (m_main_window->dumpModeEnabled())
	{
		QString const path = tr("%1_%2").arg(getAppName()).arg(m_pid);

		if (mkPath(path))
			onExportDataToCSV(path);

		m_main_window->markConnectionForClose(this);
	}

	for (dataplots_t::iterator it = m_data_widgets.get<e_data_plot>().begin(), ite = m_data_widgets.get<e_data_plot>().end(); it != ite; ++it)
		(*it)->stopUpdate();
}


struct Save {

	QString const & m_path;
	Save (QString const & p) : m_path(p) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->saveConfig(m_path);
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
			(*it)->loadConfig(m_path);
	}
};

struct Apply {

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->applyConfig();
	}
};

struct ExportAsCSV {

	QString const & m_path;
	ExportAsCSV (QString const & dir) : m_path(dir) { }

	template <class T>
	void operator() (T const & t)
	{
		typedef typename T::const_iterator it_t;
		for (it_t it = t.begin(), ite = t.end(); it != ite; ++it)
			(*it)->exportStorageToCSV(m_path);
	}
};

struct HandleAction {

	Action * m_action;
	E_ActionHandleType m_sync;
	HandleAction (Action * a, E_ActionHandleType sync) : m_action(a), m_sync(sync) { }

	template <class T>
	void operator() (T & t)
	{
	  t.handleAction(m_action, m_sync);
	}
};


void Connection::saveConfigs (QString const & path)
{
	qDebug("%s", __FUNCTION__);

	saveConfig(m_curr_preset);
	recurse(m_data_widgets, Save(path));
}

void Connection::loadConfigs (QString const & path)
{
	qDebug("%s", __FUNCTION__);

	loadConfig(m_curr_preset);

	recurse(m_data_widgets, Load(path));
	loadWaveTable(m_config.m_wavetable);
}

void Connection::loadConfig (QString const & preset_name)
{
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, getAppName(), preset_name);
	QString const fname = path + "/" + getAppName() + ".xml";
	bool const loaded = loadConfigTemplate(m_config, fname);
	if (!loaded)
	{
		//m_config = ConnectionConfig();
		//m_config.m_tag = tag_backup; // defaultConfigFor destroys tag
	}
	setConfigValuesToUI(m_config);
}

void Connection::saveConfig (QString const & preset_name)
{
	QString const path = mkAppPresetPath(getGlobalConfig().m_appdir, getAppName(), preset_name);
	mkPath(path);
	QString const fname = path + "/" + getAppName() + ".xml";
	saveConfigTemplate(m_config, fname);
}

void Connection::applyConfigs ()
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data_widgets, Apply());
	m_wavetable->apply(m_config.m_wavetable);
}

void Connection::onSaveData (QString const & dir)
{
	copyStorageTo(dir + "/" + "raw_data." + g_traceFileExtTLV);
	//recurse(m_data_widgets, SaveData(dir));
}

void Connection::onExportDataToCSV (QString const & dir)
{
	qDebug("%s", __FUNCTION__);
	recurse(m_data_widgets, ExportAsCSV(dir));
}

bool Connection::handleAction (Action * a, E_ActionHandleType sync)
{
	if (a->type() == e_Close)
	{
		m_main_window->markConnectionForClose(this);
		return true;
	}

	recurse(m_data_widgets, HandleAction(a, sync));
	return true;
}

QWidget * Connection::controlWidget ()
{
	return m_control_bar.get();
}
