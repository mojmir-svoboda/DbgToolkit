#include "logconfig.h"
#include <serialize/serialize.h>

namespace logs {

	/*bool loadConfig (LogConfig & config, QString const & fname)
	{
		if (!::loadConfigTemplate(config, fname))
		{
			fillDefaultConfig(config);
			return false;
		}
		return true;
	}*/

	bool saveConfig (LogConfig const & config, QString const & fname)
	{
		return ::saveConfigTemplate(config, fname);
	}

	void fillDefaultConfig (LogConfig & config)
	{
		config = LogConfig();

		//@TODO: apply<log_format, addTagToConfig>
		config.addTagToConfig(proto::tag_stime);
		config.addTagToConfig(proto::tag_dt);
		config.addTagToConfig(proto::tag_ctime);
		config.addTagToConfig(proto::tag_lvl);
		config.addTagToConfig(proto::tag_ctx);
		config.addTagToConfig(proto::tag_tid);
		config.addTagToConfig(proto::tag_file);
		config.addTagToConfig(proto::tag_line);
		config.addTagToConfig(proto::tag_func);
		config.addTagToConfig(proto::tag_msg);
		// 		for (size_t i = 0, ie = m_src_model->columnCount(); i < ie; ++i)
		// 		{
		// 			proto::TagDesc const & td = m_tagconfig.findOrCreateTag(i);
		// 			addTagToConfig(config, m_tagconfig.findOrCreateTag(i));
		// 		}
	}

	bool validateConfig (logs::LogConfig const & cfg)
	{
		bool b = true;
		b &= cfg.m_columns_setup.size() != 0;
		b &= cfg.m_columns_sizes.size() != 0;
		b &= cfg.m_columns_align.size() != 0;
		b &= cfg.m_columns_elide.size() != 0;

		if (!b)
			return false;

		bool same = cfg.m_columns_setup.size() == cfg.m_columns_sizes.size();
		same &= cfg.m_columns_sizes.size() == cfg.m_columns_align.size();
		same &= cfg.m_columns_align.size() == cfg.m_columns_elide.size();

		if (!same)
			return false;

		for (int i = 0, ie = cfg.m_columns_setup.size(); i < ie; ++i)
		{
			QString const & item = cfg.m_columns_setup.at(i);
			if (item.isEmpty())
				return false;
			//@NOTE: this is not compatible with csv FIXME
			//if (tlv::tag_for_name(item.toStdString().c_str()) == 0)
			//	return false;
		}

		for (int i = 0, ie = cfg.m_columns_sizes.size(); i < ie; ++i)
		{
			int const item = cfg.m_columns_sizes.at(i);
			if (item < 0)
				return false;
		}

		for (int i = 0, ie = cfg.m_columns_align.size(); i < ie; ++i)
		{
			QString const & item = cfg.m_columns_align.at(i);
			if (item.isEmpty())
				return false;
			bool valid_align = false;
			for (size_t i = 0; i < e_max_align_enum_value; ++i)
				if (aligns[i] == item.at(0).toLatin1())
					valid_align |= true;
			if (!valid_align)
				return false;
		}

		for (int i = 0, ie = cfg.m_columns_elide.size(); i < ie; ++i)
		{
			QString const & item = cfg.m_columns_elide.at(i);
			if (item.isEmpty())
				return false;
			bool valid_elide = false;
			for (size_t i = 0; i < e_max_elide_enum_value; ++i)
				if (elides[i] == item.at(0).toLatin1())
					valid_elide |= true;
			if (!valid_elide)
				return false;
		}

		return true;
	}
}


