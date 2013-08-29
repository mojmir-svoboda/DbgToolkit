#pragma  once

struct DockedWidgetBase;

struct DockedInfo
{
	QStringList m_path;

	/*@member	state
	 * duplicates qt enum
	 *	Qt::Unchecked	0	The item is unchecked.
	 *	Qt::PartiallyChecked	1	The item is partially checked. Items in hierarchical models may be partially checked if some, but not all, of their children are checked.
	 *	Qt::Checked	2	The item is checked.
	 */
	int m_state;
	int m_collapsed;
	int m_valid_widget;
	int m_centralwidget;
	int m_childwidget;

	DockedInfo () : m_state(e_Unchecked), m_collapsed(true), m_valid_widget(false), m_centralwidget(false), m_childwidget(false) { }

	template <class ArchiveT>
	void serialize (ArchiveT & ar, unsigned const version)
	{
		ar & boost::serialization::make_nvp("path", m_path);
		ar & boost::serialization::make_nvp("state", m_state);
		ar & boost::serialization::make_nvp("collapsed", m_collapsed);
		ar & boost::serialization::make_nvp("centralwidget", m_centralwidget);
	}
};


