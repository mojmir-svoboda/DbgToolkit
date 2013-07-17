#pragma once

struct AppData {

	void addCtxDict (QList<QString> const & names, QList<QString> const & strvalues)
	{
		m_dict_ctx.m_names = names;
		m_dict_ctx.m_strvalues = strvalues;
		
		foreach (QString const & item, m_dict_ctx.m_strvalues)
			m_dict_ctx.m_values.append(item.toInt());
	}

	Dict const & getDictCtx () const { return m_dict_ctx; }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("dict_ctx", m_dict_ctx);
	}

	Dict		m_dict_ctx;
};

