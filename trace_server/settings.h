#pragma once
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QObject>
#include "types.h"
#include <tlv_parser/tlv_parser.h>

E_Align const default_aligns[tlv::tag_max_value] = {
	  e_AlignLeft		// invalid
	, e_AlignLeft		// app
	, e_AlignLeft		// pid
	, e_AlignRight		// time
	, e_AlignRight		// tid
	, e_AlignRight		// file
	, e_AlignRight		// line
	, e_AlignRight		// func
	, e_AlignLeft		// msg
	, e_AlignRight		// lvl
	, e_AlignRight		// ctx
	, e_AlignLeft		// bool
	, e_AlignRight		// int
	, e_AlignLeft		// str
	, e_AlignLeft		// flt
	, e_AlignLeft		// x
	, e_AlignLeft		// y
	, e_AlignLeft		// z
};
E_Elide const default_elides[tlv::tag_max_value] = {
	  e_ElideNone		// invalid
	, e_ElideNone		// app
	, e_ElideNone		// pid
	, e_ElideLeft		// time
	, e_ElideNone		// tid
	, e_ElideLeft		// file
	, e_ElideNone		// line
	, e_ElideLeft		// func
	, e_ElideRight		// msg
	, e_ElideLeft		// lvl
	, e_ElideLeft		// ctx
	, e_ElideNone		// bool
	, e_ElideNone		// int
	, e_ElideNone		// str
	, e_ElideNone		// flt
	, e_ElideNone		// x
	, e_ElideNone		// y
	, e_ElideNone		// z
};

int const default_sizes[tlv::tag_max_value] = {
	  0		// invalid
	, 0		// app
	, 0		// pid
	, 64	// time
	, 16	// tid
	, 192	// file
	, 32	// line
	, 128	// func
	, 512	// msg
	, 16	// lvl
	, 16	// ctx
	, 0		// bool
	, 0		// int
	, 0		// str
	, 0		// flt
	, 0		// x
	, 0		// y
	, 0		// z
};



