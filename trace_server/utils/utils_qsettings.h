#pragma once

inline void write_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> const & lst)
{
	settings.beginWriteArray(groupname);
	for (int i = 0, ie = lst.size(); i < ie; ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue(groupvaluename, lst.at(i));
		qDebug("store to registry %i/%i: %s", i,ie, lst.at(i).toStdString().c_str());
	}
	settings.endArray();
}

inline void read_list_of_strings (QSettings & settings, char const * groupname, char const * groupvaluename, QList<QString> & lst)
{
	int const size = settings.beginReadArray(groupname);
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex(i);
		QString val = settings.value(groupvaluename).toString();
		//qDebug("read from registry: %s", val.toStdString().c_str());
		lst.push_back(val);
	}
	settings.endArray();
}


