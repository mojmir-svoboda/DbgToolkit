#pragma once
#include <QFrame>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_barchart.h>
#include "frameviewconfig.h"
#include "frameviewctxmenu.h"
#include "action.h"
#include "dock.h"
#include "syncwidgets.h"

class Connection;

struct BarPlot : QwtPlotBarChart
{
	BarPlot ();

	virtual QwtColumnSymbol * specialSymbol (int index, QPointF const &) const;
	virtual QwtText barTitle (int idx) const;

	QVector<double> m_values;
	std::vector<QString> m_strvalues;
	std::vector<unsigned long long> m_begins;
	std::vector<unsigned long long> m_ends;
	std::vector<QColor> m_colors;
};

struct FrameView : QWidget, ActionAble
{
	enum { e_type = e_data_frame };
	FrameView (Connection * oparent, QWidget * wparent, FrameViewConfig & cfg, QString const & fname, QStringList const & path);

	void appendFrame (unsigned long long from, unsigned long long to);
	virtual bool handleAction (Action * a, E_ActionHandleType sync);

	void loadConfig (QString const & path);
	void saveConfig (QString const & path);
	void applyConfig (FrameViewConfig & cfg);
	void applyConfig ();

	void setDockedWidget (DockedWidgetBase * dwb) { m_dwb = dwb; }

signals:
	void requestSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

public slots:
	void performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source);

	void selected (QRectF const & r);
	void selected (QVector<QPointF> const & pa);
	void selected (QPointF const & pos);
	void appended (QPointF const & pos);

	void onShow ();
	void onHide ();
	void onHideContextMenu ();
	void onShowContextMenu (QPoint const & pos);
	void setConfigValuesToUI (FrameViewConfig const & cfg);
	void setUIValuesToConfig (FrameViewConfig & cfg);
	void onApplyButton ();
	void onSaveButton ();
	void onResetViewButton ();
	void onDefaultButton ();

private slots:
	void setNum (double v);

public:
	QwtPlot * m_plot;
	BarPlot * m_bars;
	FrameViewConfig & m_config;
	frameview::CtxFrameViewConfig m_config_ui;
	DockedWidgetBase * m_dwb;

	Q_OBJECT
};

