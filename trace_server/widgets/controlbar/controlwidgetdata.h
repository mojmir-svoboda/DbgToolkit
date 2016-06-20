#ifndef CONTROLWIDGETDATA_H
#define CONTROLWIDGETDATA_H

#include <QWidget>

namespace Ui {
class ControlWidgetData;
}

struct ConnectionConfig;

// struct ControlWidgetDataConfig
// {
// 	int		  m_logs_recv_level;
// 	int		  m_plots_recv_level;
// 	int		  m_tables_recv_level;
// 	int		  m_gantts_recv_level;
// };

class ControlWidgetData : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWidgetData(QWidget *parent = 0);
    ~ControlWidgetData();

		void applyConfig (ConnectionConfig & config);

    Ui::ControlWidgetData *ui;
};

#endif // CONTROLWIDGETDATA_H
