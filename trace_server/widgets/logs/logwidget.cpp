#include "logwidget.h"
#include <QClipboard>
#include <connection.h>
#include <utils/utils.h>
#include <utils/utils_qstandarditem.h>
#include "logdelegate.h"
#include <syncwidgets.h>
#include "logtablemodel.h"
#include "findproxymodel.h"
#include <widgets/warnimage.h>
#include <serialize/serialize.h>
#include <QInputDialog>
#include <QFontDialog>
#include <QScrollBar>
#include <QShortcut>
#include <widgets/controlbar/controlbarlog.h>
#include <ui_controlbarlog.h>
#include "mainwindow.h"
#include <widgets/colorizewidget.h>
#include <utils/hsv.h>
#include <utils/set_with_blocked_signals.h>
#include "logfilterproxymodel.h"
#include "loghheaderctxmenu.h"

namespace logs {

	LogWidget::LogWidget (Connection * conn, LogConfig const & cfg, QString const & fname, QStringList const & path)
		: DockedWidgetBase(conn->getMainWindow(), path)
		, m_connection(conn)
		, m_tableview(0)
		, m_cacheLayout(new ButtonCache)
		, m_gotoPrevErrButton(nullptr) , m_gotoNextErrButton(nullptr) , m_gotoPrevWarnButton(nullptr) , m_gotoNextWarnButton(nullptr)
		, m_openFileLineButton(nullptr)
		, m_excludeFileButton(nullptr) , m_excludeFileLineButton(nullptr) , m_excludeRowButton(nullptr) , m_locateRowButton(nullptr), m_syncGroup(nullptr)
		, m_clrDataButton(nullptr), m_setRefCTimeButton(nullptr), m_setRefSTimeButton(nullptr), m_hidePrevButton(nullptr), m_hideNextButton(nullptr)
		, m_colorRowButton(nullptr) , m_colorFileLineButton(nullptr) , m_uncolorRowButton(nullptr) , m_gotoPrevColorButton(nullptr) , m_gotoNextColorButton(nullptr)
		, m_control_bar(nullptr)
		, m_config_ui(nullptr)
		, m_config_hheader_ui(nullptr)
		, m_config(cfg)
		, m_fname(fname)
		, m_warnimage(0)
/*		, m_tls()*/
		, m_ctime_ref_value(0)
		, m_stime_ref_value(0)
		, m_proxy_model(0)
		, m_find_proxy_model(0)
		, m_src_model(0)
		, m_selection(0)
		, m_ksrc_selection(0)
		, m_kproxy_selection(0)
		, m_src_selection(0)
		, m_proxy_selection(0)
		, m_find_proxy_selection(0)
		, m_kfind_proxy_selection(0)
		, m_color_regex_model(0)
		, m_find_widget(0)
		, m_colorize_widget(0)
		, m_window_action(0)
		, m_linked_parent(0)
		, m_file_csv_stream(0)
		//, m_file_tlv_stream(0)
	{
		m_queue.reserve(4096);
		m_tableview = new LogTableView(conn, *this, m_config);

		QVBoxLayout * vLayout = new QVBoxLayout();
		setLayout(vLayout);
		vLayout->setSpacing(1);
		vLayout->setContentsMargins(0, 0, 0, 0);
		vLayout->setObjectName(QString::fromUtf8("verticalLayout"));
		QWidget * cacheWidget = new QFrame();
		m_cacheLayout->setSpacing(1);
		m_cacheLayout->setContentsMargins(0, 0, 0, 0);
		m_cacheLayout->setObjectName(QString::fromUtf8("cacheLayout"));
		cacheWidget->setLayout(m_cacheLayout);
		vLayout->addWidget(cacheWidget);
		fillButtonCache(this);
		vLayout->addWidget(m_tableview);

		m_config_ui = new LogCtxMenu(*this, this);

		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		m_control_bar = new ControlBarLog(0);

		setConfigValuesToUI(m_config);

		connect(&getSyncWidgets(), SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 this, SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));
		connect(this, SIGNAL( requestSynchronization(E_SyncMode, int, unsigned long long, void *) ),
							 &getSyncWidgets(), SLOT( performSynchronization(E_SyncMode, int, unsigned long long, void *) ));


		setObjectName(QString::fromUtf8("LogWidget"));
		//setupThreadColors(connection->getMainWindow()->getThreadColors());

		QStyle const * const style = QApplication::style();
		m_config_ui->ui()->findWidget->setMainWindow(m_connection->getMainWindow());
		//connect(m_config_ui->ui()->gotoNextButton, SIGNAL(clicked()), this, SLOT(onNextToView()));
		//m_config_ui->ui()->gotoNextButton->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
		connect(m_config_ui->ui()->autoScrollCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuAutoScrollStateChanged(int)));
		connect(m_config_ui->ui()->timeComboBox, SIGNAL(activated(int)), this, SLOT(onChangeTimeUnits(int)));
		connect(m_config_ui->ui()->scopesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuShowScopesChanged(int)));
		connect(m_config_ui->ui()->dtScopesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuShowdtScopesChanged(int)));
		connect(m_config_ui->ui()->indentSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuIndentChanged(int)));
		connect(m_config_ui->ui()->cutPathCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuCutPathChanged(int)));
		connect(m_config_ui->ui()->cutPathSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuCutPathLevelChanged(int)));
		connect(m_config_ui->ui()->cutNamespaceCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCtxMenuCutNamespaceChanged(int)));
		connect(m_config_ui->ui()->cutNamespaceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuCutNamespaceLevelChanged(int)));
		connect(m_config_ui->ui()->tableRowSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuTableRowSizeChanged(int)));
		connect(m_config_ui->ui()->syncGroupSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onCtxMenuSyncGroupChanged(int)));
		//connect(ui->inViewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onInViewStateChanged(int)));
		//connect(m_config_ui->ui()->filterFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterFile(int)));
		connect(m_tableview, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
		connect(m_tableview, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));

		QObject::connect(m_tableview->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(onSectionResized(int, int, int)));
		QObject::connect(m_tableview->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(onSectionMoved(int, int, int)));
		m_tableview->setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));

		m_warnimage = new WarnImage(m_tableview);
		m_find_widget = new FindWidget(m_connection->getMainWindow(), this);
		m_find_widget->setActionAbleWidget(this);
		m_find_widget->setParent(m_tableview);
		m_colorize_widget = new ColorizeWidget(m_connection->getMainWindow(), this);
		m_colorize_widget->setActionAbleWidget(this);
		m_colorize_widget->setParent(m_tableview);

		m_window_action = new QAction("Tool widget for " + joinedPath(), this);
		connect(m_window_action, SIGNAL(triggered()), this, SLOT(onWindowActionTriggered()));
		m_main_window->addWindowAction(m_window_action);

		m_config_hheader_ui = new LogHHeaderCtxMenu(*this, nullptr);
		m_tableview->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_tableview->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowHHeaderContextMenu(QPoint const &)));
	}

	void LogWidget::fillButtonCache (QWidget * parent_widget)
	{
		QFrame * line = 0;
		QFrame * line_3 = 0;
		QFrame * line_2 = 0;
		QSpacerItem * horizontalSpacer_3 = 0;
		ButtonCache * cacheLayout = m_cacheLayout;

		m_gotoPrevErrButton = new QToolButton(parent_widget);
		m_gotoPrevErrButton->setObjectName(QStringLiteral("gotoPrevErrButton"));
		m_gotoPrevErrButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevErrButton->setCheckable(false);
		m_gotoPrevErrButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevErrButton->setArrowType(Qt::UpArrow);
		m_gotoPrevErrButton->setStyleSheet("QToolButton{color: red;}");
		m_gotoPrevErrButton->setAutoRepeat(true);
		cacheLayout->addWidget(m_gotoPrevErrButton);
		m_gotoNextErrButton = new QToolButton(parent_widget);
		m_gotoNextErrButton->setObjectName(QStringLiteral("gotoNextErrButton"));
		m_gotoNextErrButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextErrButton->setCheckable(false);
		m_gotoNextErrButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextErrButton->setArrowType(Qt::DownArrow);
		m_gotoNextErrButton->setStyleSheet("QToolButton{color: red;}");
		m_gotoNextErrButton->setAutoRepeat(true);
		cacheLayout->addWidget(m_gotoNextErrButton);

		m_gotoPrevWarnButton = new QToolButton(parent_widget);
		m_gotoPrevWarnButton->setObjectName(QStringLiteral("gotoPrevWarnButton"));
		m_gotoPrevWarnButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevWarnButton->setCheckable(false);
		m_gotoPrevWarnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevWarnButton->setArrowType(Qt::UpArrow);
		m_gotoPrevWarnButton->setStyleSheet("QToolButton{color: yellow;}");
		m_gotoPrevWarnButton->setAutoRepeat(true);
		cacheLayout->addWidget(m_gotoPrevWarnButton);
		m_gotoNextWarnButton = new QToolButton(parent_widget);
		m_gotoNextWarnButton->setObjectName(QStringLiteral("gotoNextWarnButton"));
		m_gotoNextWarnButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextWarnButton->setCheckable(false);
		m_gotoNextWarnButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextWarnButton->setArrowType(Qt::DownArrow);\
		m_gotoNextWarnButton->setStyleSheet("QToolButton{color: yellow;}");
		m_gotoNextWarnButton->setAutoRepeat(true);
		cacheLayout->addWidget(m_gotoNextWarnButton);

		m_openFileLineButton = new QToolButton(parent_widget);
		m_openFileLineButton->setObjectName(QStringLiteral("openFileLineButton"));
		m_openFileLineButton->setMaximumSize(QSize(16777215, 16));
		m_openFileLineButton->setCheckable(false);
		//m_openFileLineButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		//m_openFileLineButton->setArrowType(Qt::DownArrow);
		QIcon icon(":images/open_file.png");
		m_openFileLineButton->setIcon(icon);
		cacheLayout->addWidget(m_openFileLineButton);

		QFrame * line4 = new QFrame(parent_widget);
		line4->setObjectName(QStringLiteral("line4"));
		line4->setMinimumSize(QSize(3, 0));
		line4->setFrameShape(QFrame::VLine);
		//line4->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line4);

		m_excludeFileButton = new QToolButton(parent_widget);
		m_excludeFileButton->setObjectName(QStringLiteral("excludeFileButton"));
		m_excludeFileButton->setMinimumSize(QSize(16, 0));
		m_excludeFileButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeFileButton);

		m_excludeFileLineButton = new QToolButton(parent_widget);
		m_excludeFileLineButton->setObjectName(QStringLiteral("excludeFileLineButton"));
		m_excludeFileLineButton->setMinimumSize(QSize(16, 0));
		m_excludeFileLineButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeFileLineButton);

		m_excludeRowButton = new QToolButton(parent_widget);
		m_excludeRowButton->setObjectName(QStringLiteral("excludeRowButton"));
		QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(m_excludeRowButton->sizePolicy().hasHeightForWidth());
		m_excludeRowButton->setSizePolicy(sizePolicy);
		m_excludeRowButton->setMinimumSize(QSize(0, 0));
		m_excludeRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_excludeRowButton);

		m_locateRowButton = new QToolButton(parent_widget);
		m_locateRowButton->setObjectName(QStringLiteral("locateRowButton"));
		m_locateRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_locateRowButton);

		m_syncGroup = new QSpinBox(parent_widget);
		m_syncGroup->setObjectName(QStringLiteral("syncGroupSpingBox"));
		//m_syncGroup->setFont(font5);
		m_syncGroup->setMinimum(0);
		m_syncGroup->setMaximum(8);
		m_syncGroup->setValue(1);
		cacheLayout->addWidget(m_syncGroup);

		m_syncGroupRowButton = new QToolButton(parent_widget);
		m_syncGroupRowButton->setObjectName(QStringLiteral("syncGroupRowButton"));
		m_syncGroupRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_syncGroupRowButton);



		m_timeComboBox = new TimeComboBox(parent_widget);
		m_timeComboBox->setObjectName(QStringLiteral("timeComboBox"));
		m_timeComboBox->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_timeComboBox);

		line = new QFrame(parent_widget);
		line->setObjectName(QStringLiteral("line"));
		line->setMinimumSize(QSize(5, 0));
		line->setMaximumSize(QSize(10, 0));
		line->setFrameShape(QFrame::VLine);
		//line->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line);

		m_clrDataButton = new QToolButton(parent_widget);
		m_clrDataButton->setObjectName(QStringLiteral("clrData"));
		m_clrDataButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_clrDataButton);


		line = new QFrame(parent_widget);
		line->setObjectName(QStringLiteral("line"));
		line->setMinimumSize(QSize(5, 0));
		line->setFrameShape(QFrame::VLine);
		//line->setFrameShadow(QFrame::Sunken);
		cacheLayout->addWidget(line);

		m_setRefCTimeButton = new QToolButton(parent_widget);
		m_setRefCTimeButton->setObjectName(QStringLiteral("setRefCTimeButton"));
		m_setRefCTimeButton->setMaximumSize(QSize(16777215, 16));
		m_setRefCTimeButton->setCheckable(true);
		cacheLayout->addWidget(m_setRefCTimeButton);

		m_setRefSTimeButton = new QToolButton(parent_widget);
		m_setRefSTimeButton->setObjectName(QStringLiteral("setRefSTimeButton"));
		m_setRefSTimeButton->setMaximumSize(QSize(16777215, 16));
		m_setRefSTimeButton->setCheckable(true);
		cacheLayout->addWidget(m_setRefSTimeButton);

		m_hidePrevButton = new QToolButton(parent_widget);
		m_hidePrevButton->setObjectName(QStringLiteral("hidePrevButton"));
		m_hidePrevButton->setMaximumSize(QSize(16777215, 16));
/*		m_hidePrevButton->setCheckable(true);*/
		m_hidePrevButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_hidePrevButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_hidePrevButton);

		m_hideNextButton = new QToolButton(parent_widget);
		m_hideNextButton->setObjectName(QStringLiteral("hideNextButton"));
		m_hideNextButton->setMaximumSize(QSize(16777215, 16));
/*		m_hideNextButton->setCheckable(true);*/
		m_hideNextButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_hideNextButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_hideNextButton);

		//line_2 = new QLine(parent_widget);
		//line_2->setObjectName(QStringLiteral("line_2"));
		//line_2->setMinimumSize(QSize(3, 0));
		//line_2->setFrameShape(QFrame::VLine);
		//line_2->setFrameShadow(QFrame::Sunken);
		//cacheLayout->addWidget(line_2);

		m_colorRowButton = new QToolButton(parent_widget);
		m_colorRowButton->setObjectName(QStringLiteral("colorRowButton"));
		m_colorRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_colorRowButton);

		m_gotoPrevColorButton = new QToolButton(parent_widget);
		m_gotoPrevColorButton->setObjectName(QStringLiteral("gotoPrevColorButton"));
		m_gotoPrevColorButton->setMaximumSize(QSize(16777215, 16));
		m_gotoPrevColorButton->setCheckable(false);
		m_gotoPrevColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoPrevColorButton->setArrowType(Qt::UpArrow);
		cacheLayout->addWidget(m_gotoPrevColorButton);
		m_gotoNextColorButton = new QToolButton(parent_widget);
		m_gotoNextColorButton->setObjectName(QStringLiteral("gotoNextColorButton"));
		m_gotoNextColorButton->setMaximumSize(QSize(16777215, 16));
		m_gotoNextColorButton->setCheckable(false);
		m_gotoNextColorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_gotoNextColorButton->setArrowType(Qt::DownArrow);
		cacheLayout->addWidget(m_gotoNextColorButton);

		//m_colorFileLineButton = new QToolButton(parent_widget);
		//m_colorFileLineButton->setObjectName(QStringLiteral("colorFileLineButton"));
		//m_colorFileLineButton->setMaximumSize(QSize(16777215, 16));
		//cacheLayout->addWidget(m_colorFileLineButton);

		m_uncolorRowButton = new QToolButton(parent_widget);
		m_uncolorRowButton->setObjectName(QStringLiteral("uncolorRowButton"));
		m_uncolorRowButton->setMaximumSize(QSize(16777215, 16));
		cacheLayout->addWidget(m_uncolorRowButton);

		horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		m_gotoPrevErrButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev error</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">Shift + e</span></p></body></html>", 0));
		m_gotoPrevErrButton->setText(QApplication::translate("SettingsLog", "E", 0));
		m_gotoNextErrButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next error</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">e</span></p></body></html>", 0));
		m_gotoNextErrButton->setText(QApplication::translate("SettingsLog", "E", 0));

		m_gotoPrevWarnButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev warning</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">Shift + w</span></p></body></html>", 0));
		m_gotoPrevWarnButton->setText(QApplication::translate("SettingsLog", "W", 0));
		m_gotoNextWarnButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next warning</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">x</span></p></body></html>", 0));
		m_gotoNextWarnButton->setText(QApplication::translate("SettingsLog", "W", 0));

		m_excludeFileButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes File from current selection from table. This is shortcut for going into Filter/File:Line and click on File tree item</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">h</span></p></body></html>", 0));
		m_excludeFileButton->setText(QApplication::translate("SettingsLog", "x F", 0));

		m_excludeFileLineButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes File:Line combination from current selection from table. This is shortcut for going into Filter/File:Line and click on item</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\">x</span></p></body></html>", 0));
		m_excludeFileLineButton->setText(QApplication::translate("SettingsLog", "x F:L", 0));

		m_excludeRowButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Excludes selected row via Filter/Row. This one does not use File:Line information, so it can be used to exclude specific lines while keeping the rest.</p><p>Hotkey = <span style=\" font-weight:600;\">r</span></p></body></html>", 0));
		m_excludeRowButton->setText(QApplication::translate("SettingsLog", "x ==", 0));

		m_timeComboBox->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Change time units</p><p>Hotkey = <span style=\" font-weight:600;\">?</span></p></body></html>", 0));

		m_clrDataButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Delete table data.</p><p>Hotkey = <span style=\" font-weight:600;\">Shift+DEL</span></p></body></html>", 0));
		m_clrDataButton->setText(QApplication::translate("SettingsLog", "DEL", 0));

		m_locateRowButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Locates currently selected row in Filters/File:Line</p><p>Hotkey = <span style=\" font-weight:600;\">?</span></p></body></html>", 0));
		m_locateRowButton->setText(QApplication::translate("SettingsLog", "? ==", 0));

		m_syncGroup->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Sets current sync group. Double click on row synchronizes all members of sync group with same number.</p></body></html>", 0));
		//m_syncGroup->setText(QApplication::translate("SettingsLog", "? ==", 0));

		m_syncGroupRowButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>synchronizes all members of sync group with same number</p></body></html>", 0));
		m_syncGroupRowButton->setText(QApplication::translate("SettingsLog", "=|=", 0));

		m_setRefCTimeButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Set/Unset reference client time (= 0) to currently selected line</p></body></html>", 0));
		m_setRefCTimeButton->setText(QApplication::translate("SettingsLog", "Ref CT", 0));

		m_setRefSTimeButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Set/Unset reference server time (= 0) to currently selected line</p></body></html>", 0));
		m_setRefSTimeButton->setText(QApplication::translate("SettingsLog", "Ref ST", 0));

		m_hidePrevButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Hide rows preceeding current selection</p></body></html>", 0));
		m_hidePrevButton->setText(QApplication::translate("SettingsLog", "x ^", 0));
		m_hideNextButton->setToolTip(QApplication::translate("SettingsLog", "Hide lines following current selection", 0));
		m_hideNextButton->setText(QApplication::translate("SettingsLog", "x v", 0));
		m_colorRowButton->setText(QApplication::translate("SettingsLog", "Col", 0));
		//m_colorFileLineButton->setText(QApplication::translate("SettingsLog", "Color F:L", 0));
		m_uncolorRowButton->setText(QApplication::translate("SettingsLog", "Uncol", 0));

		m_gotoPrevColorButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto prev color tag</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\"></span></p></body></html>", 0));
		m_gotoPrevColorButton->setText(QApplication::translate("SettingsLog", "", 0));
		m_gotoNextColorButton->setToolTip(QApplication::translate("SettingsLog", "<html><head/><body><p>Goto next color tag</p><p><br/></p><p>Hotkey = <span style=\" font-weight:600;\"></span></p></body></html>", 0));
		m_gotoNextColorButton->setText(QApplication::translate("SettingsLog", "", 0));

		//cacheLayout->addItem(horizontalSpacer_3);

		connect(m_gotoPrevErrButton, SIGNAL(clicked()), this, SLOT(onGotoPrevErr()));
		connect(m_gotoNextErrButton, SIGNAL(clicked()), this, SLOT(onGotoNextErr()));
		connect(m_gotoPrevWarnButton, SIGNAL(clicked()), this, SLOT(onGotoPrevWarn()));
		connect(m_gotoNextWarnButton, SIGNAL(clicked()), this, SLOT(onGotoNextWarn()));
		connect(m_openFileLineButton, SIGNAL(clicked()), this, SLOT(onOpenFileLine()));
		connect(m_excludeFileButton, SIGNAL(clicked()), this, SLOT(onExcludeFile()));
		connect(m_excludeFileLineButton, SIGNAL(clicked()), this, SLOT(onExcludeFileLine()));
		connect(m_excludeRowButton, SIGNAL(clicked()), this, SLOT(onExcludeRow()));
		connect(m_locateRowButton, SIGNAL(clicked()), this, SLOT(onLocateRow()));

		connect(m_syncGroup, SIGNAL(valueChanged(int)), this, SLOT(onSyncGroupValueChanged(int)));
		connect(m_syncGroupRowButton, SIGNAL(clicked()), this, SLOT(onSyncGroupRowButton()));

		connect(m_clrDataButton, SIGNAL(clicked()), this, SLOT(onClearAllDataButton()));
		connect(m_timeComboBox->comboBox(), SIGNAL(activated(int)), this, SLOT(onChangeTimeUnits(int)));
		//connect(m_colorFileLineButton, SIGNAL(clicked()), this, SLOT(onColorFileLine()));
		connect(m_colorRowButton, SIGNAL(clicked()), this, SLOT(onColorRow()));
		connect(m_uncolorRowButton, SIGNAL(clicked()), this, SLOT(onUncolorRow()));
		connect(m_setRefCTimeButton, SIGNAL(clicked()), this, SLOT(onSetRefCTime()));
		connect(m_setRefSTimeButton, SIGNAL(clicked()), this, SLOT(onSetRefSTime()));
		connect(m_hidePrevButton, SIGNAL(clicked()), this, SLOT(onHidePrev()));
		connect(m_hideNextButton, SIGNAL(clicked()), this, SLOT(onHideNext()));
		connect(m_gotoPrevColorButton, SIGNAL(clicked()), this, SLOT(onGotoPrevColor()));
		connect(m_gotoNextColorButton, SIGNAL(clicked()), this, SLOT(onGotoNextColor()));
	}

	void LogWidget::setVisible (bool visible)
	{
		m_dockwidget->setVisible(visible);
		QFrame::setVisible(visible);
	}

	QModelIndex LogWidget::currentSourceIndex () const
	{
		QModelIndex current = m_tableview->currentIndex();
		if (isModelProxy())
		{
			current = m_proxy_model->mapToSource(current);
		}
		return current;
	}

	QModelIndex LogWidget::mapToSourceIndexIfProxy (QModelIndex const & idx) const
	{
		if (m_tableview->model() == m_src_model)
			return idx;

		if (m_tableview->model() == logProxy())
		{
			QModelIndex const src = m_proxy_model->mapToSource(idx);
			return src;
		}
		if (m_tableview->model() == findProxy())
		{
			QModelIndex const src = m_find_proxy_model->mapToSource(idx);
			return src;
		}
		return QModelIndex();
	}

	void LogWidget::setupRefsProxyModel (LogTableModel * linked_model, BaseProxyModel * linked_proxy)
	{
		m_src_model = linked_model;
		m_proxy_model = new ExtLogFilterProxyModel(this, filterMgr(), m_src_model);
		m_proxy_model->setSourceModel(linked_proxy);

		m_find_proxy_model = new FindProxyModel(this, m_config.m_find_config, filterMgr(), m_src_model, linked_proxy);
		m_find_proxy_model->setSourceModel(linked_proxy);
	}


	void LogWidget::setupRefsModel (LogTableModel * linked_model)
	{
		m_src_model = linked_model;
		m_proxy_model = new ExtLogFilterProxyModel(this, filterMgr(), m_src_model);
		m_proxy_model->setSourceModel(m_src_model);

		m_find_proxy_model = new FindProxyModel(this, m_config.m_find_config, filterMgr(), m_src_model, 0);
		m_find_proxy_model->setSourceModel(m_src_model);

		QItemSelectionModel * src_selection = linked_model->m_log_widget.m_tableview->selectionModel();
		QItemSelectionModel * pxy_selection = linked_model->m_log_widget.m_proxy_selection;
		//m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		//m_ksrc_selection = new KLinkItemSelectionModel(m_src_model, src_selection);
		//m_kproxy_selection = new KLinkItemSelectionModel(m_proxy_model, m_proxy_selection);
		//m_src_model = linked_model;
	}

	void LogWidget::setupCloneModel (LogTableModel * src_model)
	{
		if (src_model)
			m_src_model = src_model;
		else
		{
			m_src_model = new LogTableModel(this, *this);
			QObject::disconnect(m_src_model, SIGNAL(rowsInserted(QModelIndex,int,int)), m_tableview->verticalHeader(), SLOT(sectionsInserted(QModelIndex,int,int)));
		}

		m_proxy_model = new ExtLogFilterProxyModel(this, filterMgr(), m_src_model);
		m_proxy_model->setSourceModel(m_src_model);

		m_find_proxy_model = new FindProxyModel(this, m_config.m_find_config, filterMgr(), m_src_model, 0);
		m_find_proxy_model->setSourceModel(m_src_model);

		setupLogSelectionProxy();
	}

	void LogWidget::setupLogModel ()
	{
		m_src_model = new LogTableModel(this, *this);
		setupCloneModel(m_src_model);
	}

	void LogWidget::setupLogSelectionProxy ()
	{
		m_src_selection = new QItemSelectionModel(m_src_model);
		m_proxy_selection = new QItemSelectionModel(m_proxy_model);
		//m_ksrc_selection = new KLinkItemSelectionModel(m_src_model, m_src_selection);
		m_kproxy_selection = new KLinkItemSelectionModel(m_proxy_model, m_proxy_selection);
		//setSelectionModel(m_src_selection);
		//m_selection = new LogSelectionProxyModel(m_src_model, m_src_selection);
	}
}

// #include "c:\devel\qt5\qtbase\src\widgets\styles\qcommonstyle_p.h"
// 
// 	struct lemmein {};
// 	template<>
// 	inline lemmein QObject::findChild<lemmein>(const QString & aName, Qt::FindChildOptions options) const
// 	{
// 		QStylePrivate const * spc = static_cast<QStylePrivate const *>(d_func());
// 		QStylePrivate * sp = const_cast<QStylePrivate *>(spc);
// 		QCommonStylePrivate * csp = static_cast<QCommonStylePrivate *>(sp);
// 		if (csp->cachedOption)
// 		{
// 			delete csp->cachedOption;
// 			csp->cachedOption = nullptr;
// 		}
// 		return lemmein();
// 	}

namespace logs {
	LogWidget::~LogWidget ()
	{
		if (m_config_ui)
		{
			delete m_config_ui;
			m_config_ui = nullptr;
		}

		m_main_window->rmWindowAction(m_window_action);


		delete m_control_bar;
		m_control_bar = 0;

// 		QStyle * s_m = m_main_window->dockManager().style();
// 		s_m->findChild<lemmein>(QString(), Qt::FindChildrenRecursively);
// 
// 		QStyle * s_a = qApp->style();
// 		s_a->findChild<lemmein>(QString(), Qt::FindChildrenRecursively);
// 
// 		QStyle * s_t = m_tableview->style();
// 		s_t->findChild<lemmein>(QString(), Qt::FindChildrenRecursively);
// 		qDebug("~~~ s dm=0x%08x, app=0x%08x tab=0x%08x\n", s_m, s_a, s_t);
// 
// 		QAbstractItemDelegate * delegate = m_tableview->itemDelegate();
// 		delete delegate;
// 		m_tableview->setItemDelegate(nullptr);

		qDebug("%s this=0x%08x tag=%s queue=%u", __FUNCTION__, this, m_config.m_tag.toStdString().c_str(), m_queue.capacity());
		disconnect(this, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

		if (m_linked_parent)
		{
			LogWidget * parent = qobject_cast<LogWidget *>(m_linked_parent);
			if (parent)
			{
				parent->unregisterLinkedWidget(this);
			}
		}

		for (linked_widgets_t::iterator it = m_linked_widgets.begin(), ite = m_linked_widgets.end(); it != ite; ++it)
		{
			DockedWidgetBase * child = *it;
			m_connection->destroyDockedWidget(child);
		}
		m_linked_widgets.clear();

		delete m_tableview;
		m_tableview = nullptr;

	/*	if (m_file_csv_stream)
		{
			QIODevice * const f = m_file_csv_stream->device();
			f->close();
			delete m_file_csv_stream;
			m_file_csv_stream = 0;
			delete f;
		}*/
	}

	void LogWidget::onShow ()
	{
		show();
	}

	void LogWidget::onHide ()
	{
		hide();
	}

	void LogWidget::onHideContextMenu ()
	{
		::Ui::SettingsLog * ui = m_config_ui->ui();
		m_config_ui->onHideContextMenu();
	}

	void LogWidget::onWindowActionTriggered ()
	{
		m_config_ui->onShowContextMenu(QCursor::pos());
		::Ui::SettingsLog * ui = m_config_ui->ui();

		setConfigValuesToUI(m_config);
	}
	void LogWidget::onShowContextMenu (QPoint const & pos)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		m_config_ui->onShowContextMenu(QCursor::pos());
		::Ui::SettingsLog * ui = m_config_ui->ui();

		setConfigValuesToUI(m_config);
		//connect(ui->logViewComboBox, SIGNAL(activated(int)), this, SLOT(onLogViewActivate(int)));
	}

	void LogWidget::onShowHHeaderContextMenu (QPoint const & pos)
	{
		m_config_hheader_ui->onShowContextMenu(QCursor::pos());
		//Ui::SettingsLogHHeader * ui = m_config_hheader_ui.ui();

		//setConfigValuesToUI(m_config);
	}

	void fillColorsForThreads (int i, std::vector<std::pair<QColor, QColor>> & cols)
	{
		// pick colors for unique clusters
		cols.reserve(i);
		for (size_t hi = 0; hi < 360; hi += 360 / i)
		{
			HSV hsv;
			hsv.h = hi / 360.0f;
			hsv.s = 0.15f + tmp_randf() * 0.2f - 0.05f;
			hsv.v = 0.85f + tmp_randf() * 0.2f - 0.05f;
			QColor qcolor;
			qcolor.setHsvF(hsv.h, hsv.s, hsv.v);
			cols.push_back(std::make_pair(Qt::black, qcolor));
		}
		std::random_shuffle(cols.begin(), cols.end());
	}

	void LogWidget::applyConfig ()
	{
		CheckedComboBoxConfig cfg;
		fillDefaultConfigWithLogTags(cfg);

		if (m_config.m_thread_colors.size() == 0)
			fillColorsForThreads(8, m_config.m_thread_colors);
		filterMgr()->disconnectFiltersTo(this);
		colorizerMgr()->disconnectFiltersTo(this);
		soundMgr()->disconnectFiltersTo(this);

		setModelAndConfig(m_config);

		filterMgr()->applyConfig();
		colorizerMgr()->applyConfig();
		if (soundMgr()->getSoundRegex())
		{
			soundMgr()->getSoundRegex()->setDefaultSearchConfig(cfg);
			soundMgr()->getSoundRegex()->setWaveTable(m_connection->waveTable()); // must happen before applyConfig (needs wavetable)
		}
		soundMgr()->applyConfig();
		m_proxy_model->m_scopes_enabled = m_config.m_scopes_enabled;

		filterMgr()->connectFiltersTo(this);
		colorizerMgr()->connectFiltersTo(this);
		soundMgr()->connectFiltersTo(this);

		connect(filterMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(filterMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));
		// @TODO: nesel by mensi brutus nez je invalidate filter?
		connect(colorizerMgr(), SIGNAL(filterEnabledChanged()), this, SLOT(onFilterEnabledChanged()));
		connect(colorizerMgr(), SIGNAL(filterChangedSignal()), this, SLOT(onInvalidateFilter()));
		connect(soundMgr(), SIGNAL(soundEnabledChanged()), this, SLOT(onSoundEnabledChanged()));
		//connect(soundMgr(), SIGNAL(soundChangedSignal()), this, SLOT(onInvalidateFilter()));

		if (filterMgr()->getFilterCtx())
			filterMgr()->getFilterCtx()->setAppData(&m_connection->appData());
		if (filterMgr()->getFilterLvl())
			filterMgr()->getFilterLvl()->setAppData(&m_connection->appData());

		if (filterMgr()->getFilterString())
			filterMgr()->getFilterString()->setDefaultSearchConfig(cfg);
		if (colorizerMgr()->getColorizerString())
			colorizerMgr()->getColorizerString()->setSrcModel(m_src_model);
		if (colorizerMgr()->getColorizerRegex())
			colorizerMgr()->getColorizerRegex()->setSrcModel(m_src_model);
			//colMgr()->getFilterRegex()->setDefaultSearchConfig(cfg);
		if (colorizerMgr()->getColorizerRow())
			colorizerMgr()->getColorizerRow()->setSrcModel(m_src_model);

		setConfigValuesToUI(m_config);

		if (!m_config.m_header_setup_stream.isEmpty())
			m_tableview->horizontalHeader()->restoreState(m_config.m_header_setup_stream);
		else
			resizeSections();
		
		QString const curr_units = m_config.m_time_units_str;
		int const units_idx = m_timeComboBox->comboBox()->findText(curr_units);
		m_timeComboBox->comboBox()->setCurrentIndex(units_idx);

		m_syncGroup->setValue(m_config.m_sync_group);
	}

	void LogWidget::resizeSections ()
	{
		bool const old = blockSignals(true);
		for (int c = 0, ce = m_src_model->columnCount(); c < ce; ++c)
		{
			proto::tags const tag = proto::getTagForCol(c);
			size_t const sz = m_config.m_tag_config.findTag(tag).m_size;
			m_tableview->horizontalHeader()->resizeSection(c, sz);
		}
		blockSignals(old);
	}

	void LogWidget::setModelAndConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		if (!isLinkedWidget())
			setFilteringProxy(filterMgr()->enabled());
	}

	void LogWidget::setConfigValuesToUI (LogConfig const & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		::Ui::SettingsLog * ui = m_config_ui->ui();
		//setCheckedWithBlockedSignals(ui->dtCheckBox, cfg.m_dt_enabled);
		setCheckedWithBlockedSignals(ui->autoScrollCheckBox, cfg.m_auto_scroll);
		setCheckedWithBlockedSignals(ui->cutNamespaceCheckBox, m_config.m_cut_namespaces);
		setCheckedWithBlockedSignals(ui->cutPathCheckBox, m_config.m_cut_path);
		setCheckedWithBlockedSignals(ui->indentCheckBox, m_config.m_indent);
		setCheckedWithBlockedSignals(ui->scopesCheckBox, m_config.m_scopes_enabled);
		setCheckedWithBlockedSignals(ui->dtScopesCheckBox, m_config.m_dt_scopes_enabled);
		setValueWithBlockedSignals(ui->cutNamespaceSpinBox, m_config.m_cut_namespace_level);
		setValueWithBlockedSignals(ui->cutPathSpinBox, m_config.m_cut_path_level);
		setValueWithBlockedSignals(ui->indentSpinBox, m_config.m_indent_level);
		setValueWithBlockedSignals(ui->tableRowSizeSpinBox, m_config.m_row_width);
		setValueWithBlockedSignals(ui->syncGroupSpinBox, m_config.m_sync_group);
		setCurrentIndexWithBlockedSignals(ui->groupingWidget, m_config.m_curr_tooltab);
		// TODO: block signals
		//bool const old = m_table_view_widget->blockSignals(true);
		//m_table_view_widget->blockSignals(old);
		int const time_idx = ui->timeComboBox->comboBox()->findText(m_config.m_time_units_str);
		setCurrentIndexWithBlockedSignals(ui->timeComboBox->comboBox(), time_idx);
	}

	void LogWidget::setUIValuesToConfig (LogConfig & cfg)
	{
		//qDebug("%s this=0x%08x", __FUNCTION__, this);
		::Ui::SettingsLog * ui = m_config_ui->ui();
		//m_config.m_show = ui->globalShowCheckBox->checkState() == Qt::Checked;
		cfg.m_curr_tooltab = ui->groupingWidget->currentIndex();

		cfg.m_auto_scroll = ui->autoScrollCheckBox->isChecked();
		m_config.m_cut_namespaces = ui->cutNamespaceCheckBox->isChecked();
		m_config.m_cut_namespace_level = ui->cutNamespaceSpinBox->value();
		m_config.m_cut_path = ui->cutPathCheckBox->isChecked();
		m_config.m_cut_path_level = ui->cutPathSpinBox->value();
		m_config.m_indent = ui->indentCheckBox->isChecked();
		m_config.m_indent_level = ui->indentSpinBox->value();
		m_config.m_scopes_enabled = ui->scopesCheckBox->isChecked();
		m_config.m_dt_scopes_enabled = ui->dtScopesCheckBox->isChecked();
		//ui->tableFontComboBox->
		//ui->tableFontToolButton->
		m_config.m_row_width = ui->tableRowSizeSpinBox->value();
		m_config.m_time_units_str = ui->timeComboBox->comboBox()->currentText();
	}

	void LogWidget::onApplyButton ()
	{
		applyConfig();
		setUIValuesToConfig(m_config);
		//applyConfig();
	}

	QString LogWidget::getCurrentWidgetPath () const
	{
		QString const appdir = m_connection->getMainWindow()->getAppDir();
		QString const logpath = appdir + "/" + m_connection->getAppName() + "/" + m_connection->getCurrPreset() + "/" + g_LogTag + "/" + m_config.m_tag;
		return logpath;
	}

	void LogWidget::reconfigureConfig (logs::LogConfig & config)
	{
		fillDefaultConfig(config);
	}
	void LogWidget::defaultConfigFor (logs::LogConfig & config)
	{
		if (m_connection->protocol() == e_Proto_ASN1)
		{
			QString const & appname = m_connection->getAppName();
			if (!validateConfig(config))
				reconfigureConfig(config);
		}
		else if (m_connection->protocol() == e_Proto_CSV)
		{
		}
	}

	E_SrcProtocol LogWidget::protocol () const { return m_connection->protocol(); }

	void LogWidget::loadConfig (QString const & preset_dir)
	{
		QString const tag_backup = m_config.m_tag;
		QString const logpath = preset_dir + "/" + g_LogTag + "/" + m_config.m_tag + "/";
		m_config.clear();
		bool const loaded = loadConfigTemplate(m_config, logpath + g_LogFile);
		if (!loaded)
		{
			defaultConfigFor(m_config);
			m_config.m_tag = tag_backup; // defaultConfigFor destroys tag

			filterMgr()->m_filter_order.clear();
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_String]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Tid]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Ctx]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Lvl]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_FileLine]);
			filterMgr()->m_filter_order.push_back(g_filterNames[e_Filter_Row]);
		}

		loadAuxConfigs();
	}
	void LogWidget::loadAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		m_config.m_find_config.clear();
		loadConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		m_config.m_colorize_config.clear();
		loadConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
		filterMgr()->loadConfig(logpath);
		colorizerMgr()->loadConfig(logpath);
		soundMgr()->loadConfig(logpath);
	}
	void LogWidget::saveAuxConfigs ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
		saveConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
		filterMgr()->saveConfig(logpath);
		colorizerMgr()->saveConfig(logpath);
		soundMgr()->saveConfig(logpath);
	}
	void LogWidget::saveFindConfig ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_find_config, logpath + "/" + g_findTag);
	}
	void LogWidget::saveColorizeConfig ()
	{
		QString const logpath = getCurrentWidgetPath();
		saveConfigTemplate(m_config.m_colorize_config, logpath + "/" + g_colorizeTag);
	}

	void LogWidget::saveConfig (QString const & path)
	{
		QString const logpath = getCurrentWidgetPath();
		mkPath(logpath);

		m_config.m_header_setup_stream = m_tableview->horizontalHeader()->saveState();
		logs::saveConfig(m_config, logpath + "/" + g_LogFile);
		saveAuxConfigs();
	}

	void LogWidget::onSaveButton ()
	{
		/*m_config.m_hsize.clear();
		m_config.m_hsize.resize(m_modelView->columnCount());
		for (int i = 0, ie = m_modelView->columnCount(); i < ie; ++i)
			m_config.m_hsize[i] = horizontalHeader()->sectionSize(i);*/
		//saveConfig();
		//m_pers_filter.saveConfig(
	}
	void LogWidget::onResetButton () { setConfigValuesToUI(m_config); }
	void LogWidget::onDefaultButton ()
	{
		LogConfig defaults;
		//defaults.partialLoadFrom(m_config);
		setConfigValuesToUI(defaults);
	}

	void LogWidget::clearAllData ()
	{
		m_proxy_model->clearModelData();
		m_src_model->clearModelData();

		if (filterMgr()->getFilterRow())
			filterMgr()->getFilterRow()->clear();
	}

	void LogWidget::onClearAllDataButton ()
	{
		clearAllData();
	}

	void LogWidget::performSynchronization (E_SyncMode mode, int sync_group, unsigned long long time, void * source)
	{
		if (this == source)
			return;

		qDebug("%s syncgrp=%i time=%i", __FUNCTION__, sync_group, time);

		switch (mode)
		{
			case e_SyncClientTime:
				findNearestRow4Time(true, time);
				break;
			case e_SyncServerTime:
				findNearestRow4Time(false, time);
				break;
			case e_SyncRefSTime:
				setSTimeRefValue(time);
				onInvalidateFilter();
				break;
			case e_SyncFrame:
			case e_SyncSourceRow:
			case e_SyncProxyRow:
			default:
				break;
		}
	}

	/*void BaseLog::performFrameSynchronization (int sync_group, unsigned long long frame, void * source)
	{
		qDebug("%s syncgrp=%i frame=%i", __FUNCTION__, sync_group, frame);
	}*/

	void LogWidget::onFilterEnabledChanged ()
	{
		qDebug("%s", __FUNCTION__);
		setModelAndConfig(m_config);
		//setupUi
		//applyConfig();
	}

void LogWidget::onDumpFilters ()
{ }

void LogWidget::commitCommands (E_ReceiveMode mode)
{
	m_src_model->commitCommands(m_queue, mode);

	if (m_queue.size())
	{
		if (m_config.m_auto_scroll)
		{
			if (!m_tableview->verticalScrollBar()->isSliderDown())
				m_tableview->scrollToBottom();
		}
		m_queue.clear();
	}
}

void LogWidget::handleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
// 	if (mode == e_RecvSync)
// 		m_src_model->handleCommand(cmd, mode);
// 	else
		m_queue.push_back(cmd);

}

void LogWidget::reloadModelAccordingTo (LogConfig & config)
{
	//setItemDelegate(new LogDelegate(*this, m_connection->appData(), this));
	//setItemDelegate(0); // @FIXME TMP
	m_src_model->clearModel();
	m_proxy_model->clearModel();
	m_src_model->reloadModelAccordingTo(config);
}


// void LogWidget::commitBatchToLinkedWidgets (int src_from, int src_to, BatchCmd const & batch)
// {
// 	for (linked_widgets_t::iterator it = m_linked_widgets.begin(), ite = m_linked_widgets.end(); it != ite; ++it)
// 	{
// 		DockedWidgetBase * child = *it;
// 		if (child->type() == e_data_log)
// 		{
// 			LogWidget * lw = static_cast<LogWidget *>(child);
// 			lw->commitBatchToLinkedModel(src_from, src_to, batch);
// 		}
// 	}
// }

// void LogWidget::commitBatchToLinkedModel (int src_from, int src_to, BatchCmd const & batch)
// {
// 	FilterProxyModel * flt_pxy = m_proxy_model;
// 	if (m_tableview->model() == flt_pxy)
// 		flt_pxy->commitBatchToModel(src_from, src_to, batch);
// 
// 	FindProxyModel * fnd_pxy = m_find_proxy_model;
// 	if (m_tableview->model() == fnd_pxy)
// 		fnd_pxy->commitBatchToModel(src_from, src_to, batch);
// }

LogTableModel * LogWidget::cloneToNewModelFromProxy (LogWidget * parent, BaseProxyModel * proxy, FindConfig const & fc)
{
return nullptr;
}

LogTableModel * LogWidget::cloneToNewModel (LogWidget * parent, FindConfig const & fc)
{
	if (m_tableview->model() == m_src_model)
	{
		return m_src_model->cloneToNewModel(parent, fc);
	}
	else if (m_tableview->model() == m_proxy_model)
	{
		return cloneToNewModelFromProxy(parent, m_proxy_model, fc);
	}
	else if (m_tableview->model() == m_find_proxy_model)
	{
		return cloneToNewModelFromProxy(parent, m_find_proxy_model, fc);
	}

	Q_ASSERT(0); // apparently forgot something
	return 0;
}



/*void LogWidget::applyConfig ()
{
	settings.setValue("autoScrollCheckBox", ui->autoScrollCheckBox->isChecked());
	settings.setValue("inViewCheckBox", ui->inViewCheckBox->isChecked());
	settings.setValue("filterFileCheckBox", ui->filterFileCheckBox->isChecked());
	settings.setValue("clrFiltersCheckBox", ui_settings->clrFiltersCheckBox->isChecked());
	//settings.setValue("filterModeComboBox", ui->filterModeComboBox->currentIndex());
	//settings.setValue("filterPaneComboBox", ui_settings->filterPaneComboBox->currentIndex());

	settings.setValue("scopesCheckBox1", ui_settings->scopesCheckBox->isChecked());
	settings.setValue("indentCheckBox", ui_settings->indentCheckBox->isChecked());
	settings.setValue("cutPathCheckBox", ui_settings->cutPathCheckBox->isChecked());
	settings.setValue("cutNamespaceCheckBox", ui_settings->cutNamespaceCheckBox->isChecked());
	settings.setValue("indentSpinBox", ui_settings->indentSpinBox->value());
	settings.setValue("tableRowSizeSpinBox", ui_settings->tableRowSizeSpinBox->value());
	settings.setValue("tableFontComboBox", ui_settings->tableFontComboBox->currentText());
	settings.setValue("cutPathSpinBox", ui_settings->cutPathSpinBox->value());
	settings.setValue("cutNamespaceSpinBox", ui_settings->cutNamespaceSpinBox->value());
}

*/

inline void simplify_keep_indent (QString const & src, QString & indent, QString & dst)
{
	const int n = src.size();
	for (int i = 0; i < n; ++i)
	{
		if (!src.at(i).isSpace())
		{
			dst = src.right(n - i);
			indent = src.left(i);
			return;
		}
	}
}

QString LogWidget::exportSelection ()
{
	QAbstractItemModel * m = m_tableview->model();
	QItemSelectionModel * selection = m_tableview->selectionModel();
	if (!selection)
		return QString();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	qSort(indexes.begin(), indexes.end());

	QString selected_text;
	selected_text.reserve(4096);
	for (int i = 0; i < indexes.size(); ++i)
	{
		QModelIndex const current = indexes.at(i);
		QString const str = m->data(current).toString();
		QString skipped_indent, to_simplify;
		simplify_keep_indent(str, skipped_indent, to_simplify);
		QString const simplified = to_simplify.simplified();
		selected_text.append(skipped_indent);
		selected_text.append(simplified);

		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n'); // switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}

void LogWidget::onCopyToClipboard ()
{
	QString const text = exportSelection();
	QClipboard * clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

void LogWidget::autoScrollOff ()
{
	m_config.m_auto_scroll = false;
}

void LogWidget::autoScrollOn ()
{
	m_config.m_auto_scroll = true;
}

//@TODO: should be in model probably
void LogWidget::findNearestRow4Time (bool ctime, unsigned long long t)
{
	//qDebug("%s this=0x%08x", __FUNCTION__, this);
	bool const is_proxy = isModelProxy();
	int closest_r = 0;
	long long closest_dist = std::numeric_limits<long long>::max();
	for (int r = 0; r < m_src_model->rowCount(); ++r)
	{
		uint64_t time = 0;
		if (!ctime)
		{
			QModelIndex const tag_idx = m_src_model->index(r, proto::tag2col<proto::int_<proto::tag_stime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			time = tag_var.toULongLong();
		}
		else
		{
			QModelIndex const tag_idx = m_src_model->index(r, proto::tag2col<proto::int_<proto::tag_ctime>>::value, QModelIndex());
			QVariant const tag_var = m_src_model->data(tag_idx, Qt::DisplayRole);
			time = tag_var.toULongLong();
		}

		long long const diff = time - t;
		long long const d = abs(diff);
		bool const row_exists = is_proxy ? m_proxy_model->rowInProxy(r) : true;
		if (row_exists && d < closest_dist)
		{
			closest_r = r;
			closest_dist = d;
			//qDebug("table: nearest index= %i/%i %llu", closest_i, m_src_model->rowCount(), d);
		}
	}

	if (is_proxy)
	{
		qDebug("table: pxy nearest index= %i/%i", closest_r, m_src_model->rowCount());
		QModelIndex const curr = m_tableview->currentIndex();
		QModelIndex const idx = m_src_model->index(closest_r, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
		//qDebug("table: pxy findNearestTime curr=(%i, %i)	new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

		QModelIndex const pxy_idx = m_proxy_model->mapFromSource(idx);
		QModelIndex valid_pxy_idx = pxy_idx;
		if (!pxy_idx.isValid())
		{
			valid_pxy_idx = m_proxy_model->mapNearestFromSource(idx);
		}
		//qDebug("table: pxy findNearestTime pxy_new=(%i, %i) valid_pxy_new=(%i, %i)", pxy_idx.column(), pxy_idx.row(), valid_pxy_idx.column(), valid_pxy_idx.row());
		m_tableview->scrollTo(valid_pxy_idx, QAbstractItemView::PositionAtCenter);
		m_tableview->selectionModel()->select(valid_pxy_idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
	else
	{
		qDebug("table: nearest index= %i/%i", closest_r, m_src_model->rowCount());
		QModelIndex const curr = m_tableview->currentIndex();
		QModelIndex const idx = m_src_model->index(closest_r, curr.column() < 0 ? 0 : curr.column(), QModelIndex());
		//qDebug("table: findNearestTime curr=(%i, %i)	new=(%i, %i)", curr.column(), curr.row(), idx.column(), idx.row());

		m_tableview->scrollTo(idx, QAbstractItemView::PositionAtCenter);
		m_tableview->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

bool LogWidget::isModelProxy () const
{
	if (0 == m_tableview->model())
		return false;
	return m_tableview->model() == m_proxy_model;
}

void LogWidget::onFilterChanged ()
{
	onInvalidateFilter();
}

void LogWidget::onSectionMoved (int logical, int old_visual, int new_visual)
{
	qDebug("log: section moved logical=%i old_visual=%i new_visual=%i", logical, old_visual, new_visual);
	m_config.m_header_setup_stream = m_tableview->horizontalHeader()->saveState();
}

void LogWidget::onSectionResized (int logical, int old_size, int new_size)
{
	m_config.m_header_setup_stream = m_tableview->horizontalHeader()->saveState();
}

void LogWidget::exportStorageToCSV (QString const & path)
{
	// " --> ""
	QRegularExpression regex("\"");
	QString to_string("\"\"");
	QFile csv(path + "/" + m_config.m_tag + ".csv");
	csv.open(QIODevice::WriteOnly);
	QTextStream str(&csv);

	for (int c = 0, ce = static_cast<int>(m_config.m_columns_setup.size()); c < ce; ++c)
	{
		str << "\"" << m_config.m_columns_setup.at(c) << "\"";
		if (c < ce - 1)
			str << ",\t";
	}
	str << "\n";

	for (int r = 0, re = m_tableview->model()->rowCount(); r < re; ++r)
	{
		for (int c = 0, ce = m_tableview->model()->columnCount(); c < ce; ++c)
		{
			QModelIndex current = m_tableview->model()->index(r, c, QModelIndex());
			// csv nedumpovat pres proxy
			QString txt = m_tableview->model()->data(current).toString();
			QString simplified = txt.simplified();
			QString const quoted_txt = simplified.replace(regex, to_string);
			str << "\"" << quoted_txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}

void LogWidget::onCtxMenuAutoScrollStateChanged(int state)
{
	m_config.m_auto_scroll = state == Qt::Checked;
}
void LogWidget::onCtxMenuShowScopesChanged(int state)
{
	m_config.m_scopes_enabled = state == Qt::Checked;
	m_proxy_model->m_scopes_enabled = m_config.m_scopes_enabled;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuShowdtScopesChanged(int state)
{
	m_config.m_dt_scopes_enabled = state == Qt::Checked;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuIndentChanged (int value)
{
	m_config.m_indent = value;
}
void LogWidget::onCtxMenuCutPathChanged (int state)
{
	m_config.m_cut_path = state == Qt::Checked;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuCutPathLevelChanged (int value)
{
	m_config.m_cut_path_level = value;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuCutNamespaceChanged (int state)
{
	m_config.m_cut_namespaces = state == Qt::Checked;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuCutNamespaceLevelChanged (int value)
{
	m_config.m_cut_namespace_level = value;
	onInvalidateFilter();
}
void LogWidget::onCtxMenuTableRowSizeChanged (int value)
{
	m_config.m_row_width = value;
}
void LogWidget::onCtxMenuSyncGroupChanged (int value)
{
	m_config.m_sync_group = value;
}

}


