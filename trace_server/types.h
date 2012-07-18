#pragma once
#include <QList>
#include <QString>

enum E_NodeStates {
	  e_Unchecked
	, e_PartialCheck
	, e_Checked
};

enum E_FilterMode {
	  e_Include
	, e_Exclude
};

enum E_LevelMode {
	  e_LvlInclude = 0
	, e_LvlForceInclude = 1
	, e_max_lvlmod_enum_value
};

static char lvlmods[e_max_lvlmod_enum_value] = { 'I', 'F' };
inline char lvlModToString (E_LevelMode l) { return lvlmods[l]; }
inline E_LevelMode stringToLvlMod (char c) {
	for (size_t i = 0; i < e_max_lvlmod_enum_value; ++i)
		if (lvlmods[i] == c)
			return static_cast<E_LevelMode>(i);
	return e_LvlInclude;
}


inline E_FilterMode invert (E_FilterMode m) { return m == e_Include ? e_Include : e_Exclude; }
enum E_ColorRole { e_Bg, e_Fg };

typedef QList<QString>			columns_setup_t;
typedef QList<int>				columns_sizes_t;

enum E_Align { e_AlignLeft, e_AlignRight, E_AlignMid, e_max_align_enum_value };
typedef char T_Aligns[e_max_align_enum_value];
static T_Aligns aligns = { 'L', 'R', 'M' };
enum E_Elide { e_ElideLeft = 0, e_ElideRight, e_ElideMiddle, e_ElideNone, e_max_elide_enum_value }; // synchronized with Qt::TextElideMode
typedef char T_Elides[e_max_elide_enum_value];
static T_Elides elides = { 'L', 'R', 'M', '-' };

inline char alignToString (E_Align a) { return aligns[a]; }
inline E_Align stringToAlign (char c) {
	for (size_t i = 0; i < e_max_align_enum_value; ++i)
		if (aligns[i] == c)
			return static_cast<E_Align>(i);
	return e_AlignLeft; // default
}

inline char elideToString (E_Elide a) { return elides[a]; }
inline E_Elide stringToElide (char c) {
	for (size_t i = 0; i < e_max_elide_enum_value; ++i)
		if (elides[i] == c)
			return static_cast<E_Elide>(i);
	return e_ElideLeft; // default
}

typedef QList<QString>			columns_align_t;
typedef QList<QString>			columns_elide_t;

typedef std::pair<std::string, std::string> fileline_t;
typedef unsigned long long context_t;

typedef std::vector<std::string> strings_t;

