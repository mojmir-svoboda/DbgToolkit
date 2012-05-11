#pragma once
#include <QList>
#include <QString>

enum E_FilterMode { e_Include, e_Exclude };

inline E_FilterMode invert (E_FilterMode m) { return m == e_Include ? e_Include : e_Exclude; }
enum E_ColorRole { e_Bg, e_Fg };

typedef QList<QString>			filter_regexs_t;
typedef QList<QString>			filter_preset_t;
typedef QList<filter_preset_t>	filter_presets_t;
typedef QList<QString>			columns_setup_t;
typedef QList<int>				columns_sizes_t;


