#pragma once
#include <QList>
#include <QVector>
#include <QString>
#include <QColor>
#include <vector>

using level_t = uint32_t;
using context_t = uint32_t; // see default_config.h // @TODO @FIXME

using mixervalues_t = std::array<level_t, sizeof(context_t) * CHAR_BIT>;

// available widget types @TODO: frameview
enum E_DataWidgetType {
	  e_data_log
	, e_data_plot
	, e_data_table
	, e_data_gantt
	, e_data_frame
	, e_data_widget_max_value
};

enum E_ApiErrors { e_InvalidItem = -1 };

enum E_FeatureStates {
	  e_FtrDisabled		= 0
	, e_FtrDataOnly		= 1
	, e_FtrEnabled		= 2
};

enum E_ReceiveMode {
	  e_RecvSync
	, e_RecvBatched
};

enum E_NodeStates {
	  e_Unchecked
	, e_PartialCheck
	, e_Checked
};

enum E_ExprState {
	  e_ExprInvalid
	, e_ExprValid
};

enum E_TimeUnits {
	  e_Time_ms
	, e_Time_us
	, e_Time_s
	, e_Time_m
	, e_max_timeunits_enum_value
};

QString const timeUnitsStr[] = { "ms", "us", "s", "m" };
float const timeUnitsVal[] = { 0.001f, 0.000001f, 1.0f, 60.0f };

inline QString const & unitsToString (int i) { return timeUnitsStr[i]; }
inline float stringToUnitsValue (QString const & unit_str)
{
	for (size_t i = 0; i < e_max_timeunits_enum_value; ++i)
		if (timeUnitsStr[i] == unit_str)
			return timeUnitsVal[i];
	return 0.001f;
}
inline E_TimeUnits stringToUnits (QString const & unit_str)
{
	for (size_t i = 0; i < e_max_timeunits_enum_value; ++i)
		if (timeUnitsStr[i] == unit_str)
			return static_cast<E_TimeUnits>(i);
	return e_Time_ms;
}

enum class E_FilterMode : unsigned {
	e_Default
	, e_Negate
	, e_max_enum_value
};
QString const fltmodsStr[E_FilterMode::e_max_enum_value] = { "default", "Negate" };
inline QString fltModToString (E_FilterMode l) { return fltmodsStr[static_cast<unsigned>(l)]; }
inline E_FilterMode stringToFltMod (QString const & s) {
	for (size_t i = 0; i < static_cast<unsigned>(E_FilterMode::e_max_enum_value); ++i)
		if (fltmodsStr[i] == s)
			return static_cast<E_FilterMode>(i);
	return E_FilterMode::e_Default;
}

// level modes
enum class E_LevelMode : unsigned {
	  e_LvlInclude = 0
	, e_LvlForceInclude = 1
	, e_max_enum_value
};
QString const lvlmodsStr[E_LevelMode::e_max_enum_value] = { "Include", "Force" };
inline QString lvlModToString (E_LevelMode l) { return lvlmodsStr[static_cast<unsigned>(l)]; }
inline E_LevelMode stringToLvlMod (QString const & s) {
	for (size_t i = 0; i < static_cast<unsigned>(E_LevelMode::e_max_enum_value); ++i)
		if (lvlmodsStr[i] == s)
			return static_cast<E_LevelMode>(i);
	return E_LevelMode::e_LvlInclude;
}

// row modes
enum class E_RowMode : unsigned {
	  e_RowInclude = 0
	, e_RowForceInclude = 1
	, e_max_enum_value
};
QString const rowmodsStr[E_RowMode::e_max_enum_value] = { "Include", "Force" };
inline QString rowModToString (E_RowMode l) { return rowmodsStr[static_cast<unsigned>(l)]; }
inline E_RowMode stringToRowMod (QString const & s) {
	for (size_t i = 0; i < static_cast<unsigned>(E_RowMode::e_max_enum_value); ++i)
		if (rowmodsStr[i] == s)
			return static_cast<E_RowMode>(i);
	return E_RowMode::e_RowInclude;
}


enum E_ColorRole { e_Bg, e_Fg };

typedef QList<QString>			columns_setup_t;
typedef QList<int>				columns_sizes_t;
typedef QList<QString>			columns_align_t;
typedef QList<QString>			columns_elide_t;

typedef std::pair<QString, QString> fileline_t;

typedef std::vector<std::string> strings_t;

enum E_SrcStream {
	e_Stream_TCP,
	e_Stream_File,
};

enum E_SrcProtocol {
	e_Proto_ASN1,
	e_Proto_TraceFile,
	e_Proto_CSV
};


enum E_Align { e_AlignLeft, e_AlignRight, E_AlignMid, e_max_align_enum_value };
typedef char T_Aligns[e_max_align_enum_value];
static T_Aligns aligns = { 'L', 'R', 'M' };
static char const * alignsStr[e_max_align_enum_value] = { "Left", "Right", "Middle" };
inline char alignToString(E_Align a)
{
	return a < e_max_align_enum_value ? aligns[a] : aligns[e_AlignLeft];
}
inline E_Align stringToAlign(char c) {
	for (size_t i = 0; i < e_max_align_enum_value; ++i)
		if (aligns[i] == c)
			return static_cast<E_Align>(i);
	return e_AlignLeft; // default
}

enum E_Elide { e_ElideLeft = 0, e_ElideRight, e_ElideMiddle, e_ElideNone, e_max_elide_enum_value }; // synchronized with Qt::TextElideMode
typedef char T_Elides[e_max_elide_enum_value];
static T_Elides elides = { 'L', 'R', 'M', '-' };
static char const * elidesStr[e_max_elide_enum_value] = { "Left", "Right", "Middle", "-" };

inline char elideToString(E_Elide e)
{
	return e < e_max_elide_enum_value ? elides[e] : elides[e_ElideRight];
}
inline E_Elide stringToElide(char c) {
	for (size_t i = 0; i < e_max_elide_enum_value; ++i)
		if (elides[i] == c)
			return static_cast<E_Elide>(i);
	return e_ElideLeft; // default
}


