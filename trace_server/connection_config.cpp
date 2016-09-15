#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
//#include "utils.h"
//#include "utils_qstandarditem.h"
#include <utils/utils_boost.h>
#include "constants.h"
#include "mainwindow.h"
#include <trace_proto/encode_config.h>
#include <QTcpSocket>
#include <widgets/mixer.h>

bool Connection::dumpModeEnabled () const { return m_main_window->dumpModeEnabled(); }

namespace {
	struct RegisterCommand {
		MainWindow & m_main_window;
		QString const & m_app_name;
		RegisterCommand (MainWindow & mw, QString const & app_name) : m_main_window(mw), m_app_name(app_name) { }
		
		template <typename T>
		void operator() (T & t)
		{
			m_main_window.dockManager().removeActionAble(t);
			QStringList p = m_main_window.dockManager().path();
			p << m_app_name;
			p << g_fileTags[t.e_type];
			t.m_path = p;
			t.m_joined_path = p.join("/");
			m_main_window.dockManager().addActionAble(t, true);
		}
	};
}
void Connection::registerDataMaps ()
{
	recurse(m_data_widgets, RegisterCommand(*m_main_window, getAppName()));
}

bool Connection::sendConfigCommand (ConnectionConfig const & config)
{
	enum : size_t { max_msg_size = 512 };
	char msg[max_msg_size];
	asn1::Header & hdr = asn1::encode_header(msg, max_msg_size);
	char const * mixer_ptr = reinterpret_cast<char const *>(config.m_mixer.m_state.data());
	QString const & appName = m_app_name;
	size_t const mixer_sz = config.m_mixer.m_state.size() * sizeof(level_t);
	const size_t n = asn1::encode_config(msg + sizeof(asn1::Header), max_msg_size - sizeof(asn1::Header), appName.toStdString().c_str(), mixer_ptr, mixer_sz, config.m_buffered, m_pid.toUInt());
	hdr.m_len = n;

	if (m_tcpstream && n > 0)
	{
		m_tcpstream->write(msg, n + sizeof(asn1::Header));
		m_tcpstream->flush();
		return true;
	}
	return false;
}

bool Connection::handleConfigCommand (DecodedCommand const & cmd)
{
	qDebug("Connection::handleSetupCommand() this=0x%08x", this);

	if (cmd.choice.config.pid)
		m_pid = QString::number(cmd.choice.config.pid);

	OCTET_STRING const & ostr = cmd.choice.config.app;
	QString const app_name = QString::fromLatin1(reinterpret_cast<char const *>(ostr.buf), ostr.size);

	Connection * conn = m_main_window->findConnectionByName(app_name);
	if (conn)
	{
		qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
		qDebug("deleting old instance of %s at @ 0x%08x", conn->getAppName().toStdString().c_str(), conn);
		QString const curr_preset = conn->getCurrentPresetName();
		//m_main_window->markConnectionForClose(this);
		m_main_window->onCloseConnection(conn); // deletes it immeadiately

		// @TODO: delete persistent storage for the tab

		m_curr_preset = curr_preset;
	}
	else
	{
		qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
		m_curr_preset = getClosestPresetName();
	}

	if (!app_name.isEmpty())
		m_app_name = app_name;

	QString storage_name = createStorageName();
	setupStorage(storage_name);

	m_main_window->dockManager().removeActionAble(*this);
	m_path.last() = m_app_name;
	m_joined_path = m_path.join("/");
	m_main_window->dockManager().addActionAble(*this, true); // TODO: m_config.m_show

	loadConfig(m_curr_preset);
	onPresetApply(m_curr_preset);

	registerDataMaps();

	bool send_config_back_to_client = false;
	// mixer
	if (m_config.m_mixer.m_default)
	{
		OCTET_STRING const & omixer = cmd.choice.config.mixer;
		char const * const ptr = reinterpret_cast<char const *>(omixer.buf);
		level_t const * levels = reinterpret_cast<level_t const *>(ptr);
		Q_ASSERT(m_config.m_mixer.m_state.size() == omixer.size / sizeof(level_t));

		for (level_t & l : m_config.m_mixer.m_state)
			l = *levels++;
		m_mixer->applyConfig(m_config.m_mixer);
	}
	else
	{
		send_config_back_to_client = true;
	}

	if (m_config.m_buffered != cmd.choice.config.buffered)
	{
		m_config.m_buffered = cmd.choice.config.buffered;
		send_config_back_to_client = true;
	}

	if (send_config_back_to_client)
		sendConfigCommand(m_config);

	//cmd.choice.config.buffered
	//qDebug("Server::incomingConnection buffering not enabled, notifying client");
	//onBufferingStateChanged(m_config.m_buffered);

	return true;
}

bool Connection::handleDictionary (DecodedCommand const & cmd)
{
	int64_t ** const ptr_val = cmd.choice.dict.value.list.array;
	OCTET_STRING_t ** const ptr_name = cmd.choice.dict.name.list.array;
	size_t const count = cmd.choice.dict.value.list.count;
	int const type = cmd.choice.dict.type;

	for (size_t i = 0; i < count; ++i)
	{
		int64_t const val_i = *ptr_val[i];
		OCTET_STRING_t const & name_i = *ptr_name[i];
		char const * raw_name = reinterpret_cast<char const *>(name_i.buf);
		QString const name = QString::fromLatin1(raw_name, name_i.size);
		switch (type)
		{
			case 0:	m_app_data.m_dict_lvl.add(val_i, name); break;
			case 1: m_app_data.m_dict_ctx.add(val_i, name); break;
		}
	}

	emit dictionaryArrived(type, type == 0 ? m_app_data.m_dict_lvl : m_app_data.m_dict_ctx);
	return true;
}

void Connection::handleCSVSetup (QString const & fname)
{
	qDebug("Connection::handleCSVSetup() this=0x%08x", this);

	QString const app_name = fname;

	// @TODO: dedup with tcp
	Connection * conn = m_main_window->findConnectionByName(app_name);
	if (conn)
	{
		qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
		qDebug("deleting old instance of %s at @ 0x%08x", conn->getAppName().toStdString().c_str(), conn);
		QString const curr_preset = conn->getCurrentPresetName();
		//m_main_window->markConnectionForClose(this);
		m_main_window->onCloseConnection(conn); // deletes it immeadiately

		// @TODO: delete persistent storage for the tab
		m_curr_preset = curr_preset;
	}
	else
	{
		qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
		m_curr_preset = getClosestPresetName();
	}

	m_app_name = app_name;
	//m_pid = pid;

	m_main_window->dockManager().removeActionAble(*this);
	m_path.last() = m_app_name;
	m_joined_path = m_path.join("/");
	m_main_window->dockManager().addActionAble(*this, true); // TODO: m_config.m_show

	loadConfig(m_curr_preset);
	onPresetApply(m_curr_preset);

	registerDataMaps();
}

void Connection::onMixerApplied ()
{
	if (m_config.m_mixer.m_state != m_mixer->m_config.m_state)
	{
		onMixerChanged(m_mixer->m_config);
	}
}
