#include "logwidget.h"
#include "logconfig.h"
#include <mainwindow.h>
#include <connection.h>
#include <QObject>

// legacy configs

namespace {
	template <class SrcT, class DstT>
	void assignSrcToDst (SrcT & src, DstT & dst)
	{
		dst.reserve(src.size());
		for (int i = 0, ie = src.size(); i < ie; ++i)
			dst.push_back(src.at(i));
	}
}

namespace logs {

bool LogWidget::convertBloodyBollockyBuggeryRegistry (logs::LogConfig & cfg)
{
	qDebug("%s", __FUNCTION__);
	int const i = m_connection->getMainWindow()->findAppName(m_connection->getAppName());
	if (i != e_InvalidItem && m_connection->getGlobalConfig().m_columns_setup.size() > i)
	{
		assignSrcToDst(m_connection->getGlobalConfig().m_columns_setup[i], cfg.m_columns_setup);
		assignSrcToDst(m_connection->getGlobalConfig().m_columns_sizes[i], cfg.m_columns_sizes);
		assignSrcToDst(m_connection->getGlobalConfig().m_columns_align[i], cfg.m_columns_align);
		assignSrcToDst(m_connection->getGlobalConfig().m_columns_elide[i], cfg.m_columns_elide);
		return true;
	}
	return false;
}

void addTagToConfig (logs::LogConfig & config, TagDesc const & td)
{
	config.m_columns_setup.push_back(tlv::get_tag_name(td.m_tag));
	config.m_columns_sizes.push_back(td.m_size);
	config.m_columns_align.push_back(td.m_align_str);
	config.m_columns_elide.push_back(td.m_elide_str);
}

void LogWidget::reconfigureConfig (logs::LogConfig & config)
{
	fillDefaultConfig(config);

	tlv::tag_t const tags[] = {
		  tlv::tag_stime
		, tlv::tag_ctime
		, tlv::tag_tid
		, tlv::tag_lvl
		, tlv::tag_ctx
		, tlv::tag_file
		, tlv::tag_line
		, tlv::tag_func
		, tlv::tag_msg
	};

	for (size_t i = 0, ie = sizeof(tags)/sizeof(*tags); i < ie; ++i)
	{
		TagDesc const & td = m_tagconfig.findOrCreateTag(tags[i]);
		addTagToConfig(config, td);
	}
}

void LogWidget::defaultConfigFor (logs::LogConfig & config)
{
	QString const & appname = m_connection->getAppName();
	int const idx = m_connection->getMainWindow()->findAppName(appname);
	if (idx != e_InvalidItem)
	{
		bool const converted = convertBloodyBollockyBuggeryRegistry(config);
		if (converted)
		{
			if (!validateConfig(config))
				reconfigureConfig(config);
		}
		else
			reconfigureConfig(config);
		return;
	}
}

}

