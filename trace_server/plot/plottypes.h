#pragma once
#include <sysfn/enum_factory.h>
#include <cstdlib>

namespace plot {

#	define PLOTSYMBOL_ENUM(XX)						\
		XX(NoSymbol,)								\
		XX(Ellipse,)								\
		XX(Rect,)									\
		XX(Diamond,)								\
		XX(Triangle,)								\
		XX(DTriangle,)								\
		XX(UTriangle,)								\
		XX(LTriangle,)								\
		XX(RTriangle,)								\
		XX(Cross,)									\
		XX(XCross,)									\
		XX(HLine,)									\
		XX(VLine,)									\
		XX(Star1,)									\
		XX(Star2,)									\
		XX(Hexagon,)


	FACT_DECLARE_ENUM(E_PlotSymbol,PLOTSYMBOL_ENUM);
	FACT_DECLARE_ENUM_STR(E_PlotSymbol);
	FACT_DECLARE_ENUM_TO_STRING(E_PlotSymbol,PLOTSYMBOL_ENUM);
	//FACT_DECLARE_ENUM_FROM_STR(E_PlotSymbol,PLOTSYMBOL_ENUM);

#	define CURVESTYLE_ENUM(XX)						\
		XX(NoCurve,)								\
		XX(Lines,)									\
		XX(Sticks,)									\
		XX(Steps,)									\
		XX(Dots,)									\

	FACT_DECLARE_ENUM(E_CurveStyle,CURVESTYLE_ENUM);
	FACT_DECLARE_ENUM_STR(E_CurveStyle);
	FACT_DECLARE_ENUM_TO_STRING(E_CurveStyle,CURVESTYLE_ENUM);
	//FACT_DECLARE_ENUM_FROM_STR(E_CurveStyle,CURVESTYLE_ENUM);
}
