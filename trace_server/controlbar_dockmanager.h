#pragma once
#include <QWidget>

namespace Ui {
	class ControlBarDockManager;
}

class ControlBarDockManager : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBarDockManager (QWidget *parent = 0);
    ~ControlBarDockManager ();

    Ui::ControlBarDockManager * ui;
};

#include <qtsln/flowlayout.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlBarDockManager
{
public:
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QSpinBox *levelSpinBox;
    QFrame *line_3;
    QCheckBox *buffCheckBox;
    QFrame *line_4;
    QLabel *label_5;
    QComboBox *presetComboBox;
    QPushButton *activatePresetButton;
    QPushButton *presetSaveButton;
    QPushButton *presetAddButton;
    QPushButton *presetRmButton;
    QPushButton *presetResetButton;
    QFrame *line_2;
    QLabel *label_6;
    QSlider *logSlider;
    QFrame *line_5;
    QLabel *label;
    QSlider *plotSlider;
    QFrame *line_7;
    QLabel *label_2;
    QSlider *tableSlider;
    QFrame *line_8;
    QLabel *label_4;
    QSlider *ganttSlider;

    void setupUi(QWidget *ControlBarDockManager)
    {
        if (ControlBarDockManager->objectName().isEmpty())
            ControlBarDockManager->setObjectName(QStringLiteral("ControlBarDockManager"));
        ControlBarDockManager->resize(582, 30);
        horizontalLayout_2 = new QHBoxLayout(ControlBarDockManager);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_3 = new QLabel(ControlBarDockManager);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setMaximumSize(QSize(32, 16777215));
        QFont font;
        font.setFamily(QStringLiteral("Verdana"));
        font.setPointSize(7);
        label_3->setFont(font);

        horizontalLayout->addWidget(label_3);

        levelSpinBox = new QSpinBox(ControlBarDockManager);
        levelSpinBox->setObjectName(QStringLiteral("levelSpinBox"));
        levelSpinBox->setMinimumSize(QSize(32, 0));
        levelSpinBox->setMaximumSize(QSize(32, 16777215));
        QFont font1;
        font1.setFamily(QStringLiteral("Verdana"));
        font1.setPointSize(7);
        font1.setBold(true);
        font1.setWeight(75);
        levelSpinBox->setFont(font1);
        levelSpinBox->setReadOnly(false);
        levelSpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        levelSpinBox->setValue(5);

        horizontalLayout->addWidget(levelSpinBox);

        line_3 = new QFrame(ControlBarDockManager);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setMinimumSize(QSize(3, 0));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_3);

        buffCheckBox = new QCheckBox(ControlBarDockManager);
        buffCheckBox->setObjectName(QStringLiteral("buffCheckBox"));
        buffCheckBox->setMaximumSize(QSize(48, 16777215));
        buffCheckBox->setFont(font);
        buffCheckBox->setChecked(true);

        horizontalLayout->addWidget(buffCheckBox);

        line_4 = new QFrame(ControlBarDockManager);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setMinimumSize(QSize(3, 0));
        line_4->setFrameShape(QFrame::VLine);
        line_4->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_4);

        label_5 = new QLabel(ControlBarDockManager);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setMaximumSize(QSize(36, 16777215));
        label_5->setFont(font);

        horizontalLayout->addWidget(label_5);

        presetComboBox = new QComboBox(ControlBarDockManager);
        presetComboBox->setObjectName(QStringLiteral("presetComboBox"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(presetComboBox->sizePolicy().hasHeightForWidth());
        presetComboBox->setSizePolicy(sizePolicy);
        presetComboBox->setMinimumSize(QSize(128, 0));
        presetComboBox->setMaximumSize(QSize(192, 16777215));
        presetComboBox->setFont(font);
        presetComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        presetComboBox->setFrame(true);

        horizontalLayout->addWidget(presetComboBox);

        activatePresetButton = new QPushButton(ControlBarDockManager);
        activatePresetButton->setObjectName(QStringLiteral("activatePresetButton"));
        activatePresetButton->setMinimumSize(QSize(32, 0));
        activatePresetButton->setMaximumSize(QSize(20, 16777215));
        QFont font2;
        font2.setFamily(QStringLiteral("Verdana"));
        font2.setPointSize(7);
        font2.setBold(false);
        font2.setItalic(false);
        font2.setWeight(50);
        activatePresetButton->setFont(font2);
        activatePresetButton->setFlat(false);

        horizontalLayout->addWidget(activatePresetButton);

        presetSaveButton = new QPushButton(ControlBarDockManager);
        presetSaveButton->setObjectName(QStringLiteral("presetSaveButton"));
        presetSaveButton->setMinimumSize(QSize(32, 0));
        presetSaveButton->setMaximumSize(QSize(13, 16777215));
        presetSaveButton->setFont(font);

        horizontalLayout->addWidget(presetSaveButton);

        presetAddButton = new QPushButton(ControlBarDockManager);
        presetAddButton->setObjectName(QStringLiteral("presetAddButton"));
        presetAddButton->setMaximumSize(QSize(13, 16777215));
        presetAddButton->setFont(font);

        horizontalLayout->addWidget(presetAddButton);

        presetRmButton = new QPushButton(ControlBarDockManager);
        presetRmButton->setObjectName(QStringLiteral("presetRmButton"));
        presetRmButton->setMaximumSize(QSize(13, 16777215));
        presetRmButton->setFont(font);

        horizontalLayout->addWidget(presetRmButton);

        presetResetButton = new QPushButton(ControlBarDockManager);
        presetResetButton->setObjectName(QStringLiteral("presetResetButton"));
        presetResetButton->setMaximumSize(QSize(24, 16777215));
        presetResetButton->setFont(font);

        horizontalLayout->addWidget(presetResetButton);

        line_2 = new QFrame(ControlBarDockManager);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setMinimumSize(QSize(8, 0));
        line_2->setFont(font);
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_2);

        label_6 = new QLabel(ControlBarDockManager);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setMaximumSize(QSize(24, 16777215));
        label_6->setFont(font);

        horizontalLayout->addWidget(label_6);

        logSlider = new QSlider(ControlBarDockManager);
        logSlider->setObjectName(QStringLiteral("logSlider"));
        logSlider->setMinimumSize(QSize(24, 0));
        logSlider->setMaximumSize(QSize(24, 16777215));
        logSlider->setMaximum(2);
        logSlider->setPageStep(1);
        logSlider->setOrientation(Qt::Horizontal);
        logSlider->setTickPosition(QSlider::TicksBothSides);
        logSlider->setTickInterval(0);

        horizontalLayout->addWidget(logSlider);

        line_5 = new QFrame(ControlBarDockManager);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setMinimumSize(QSize(3, 0));
        line_5->setFrameShape(QFrame::VLine);
        line_5->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_5);

        label = new QLabel(ControlBarDockManager);
        label->setObjectName(QStringLiteral("label"));
        label->setMinimumSize(QSize(0, 0));
        label->setMaximumSize(QSize(24, 16777215));
        label->setFont(font);

        horizontalLayout->addWidget(label);

        plotSlider = new QSlider(ControlBarDockManager);
        plotSlider->setObjectName(QStringLiteral("plotSlider"));
        plotSlider->setMinimumSize(QSize(24, 0));
        plotSlider->setMaximumSize(QSize(24, 16777215));
        plotSlider->setMaximum(2);
        plotSlider->setPageStep(1);
        plotSlider->setOrientation(Qt::Horizontal);
        plotSlider->setTickPosition(QSlider::TicksBothSides);
        plotSlider->setTickInterval(0);

        horizontalLayout->addWidget(plotSlider);

        line_7 = new QFrame(ControlBarDockManager);
        line_7->setObjectName(QStringLiteral("line_7"));
        line_7->setMinimumSize(QSize(3, 0));
        line_7->setFrameShape(QFrame::VLine);
        line_7->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_7);

        label_2 = new QLabel(ControlBarDockManager);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setMaximumSize(QSize(24, 16777215));
        label_2->setFont(font);

        horizontalLayout->addWidget(label_2);

        tableSlider = new QSlider(ControlBarDockManager);
        tableSlider->setObjectName(QStringLiteral("tableSlider"));
        tableSlider->setMinimumSize(QSize(24, 0));
        tableSlider->setMaximumSize(QSize(24, 16777215));
        tableSlider->setMaximum(2);
        tableSlider->setPageStep(1);
        tableSlider->setOrientation(Qt::Horizontal);
        tableSlider->setTickPosition(QSlider::TicksBothSides);

        horizontalLayout->addWidget(tableSlider);

        line_8 = new QFrame(ControlBarDockManager);
        line_8->setObjectName(QStringLiteral("line_8"));
        line_8->setMinimumSize(QSize(3, 0));
        line_8->setFrameShape(QFrame::VLine);
        line_8->setFrameShadow(QFrame::Sunken);

        horizontalLayout->addWidget(line_8);

        label_4 = new QLabel(ControlBarDockManager);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setMaximumSize(QSize(26, 16777215));
        label_4->setFont(font);

        horizontalLayout->addWidget(label_4);

        ganttSlider = new QSlider(ControlBarDockManager);
        ganttSlider->setObjectName(QStringLiteral("ganttSlider"));
        ganttSlider->setMinimumSize(QSize(24, 0));
        ganttSlider->setMaximumSize(QSize(24, 16777215));
        ganttSlider->setMaximum(2);
        ganttSlider->setOrientation(Qt::Horizontal);
        ganttSlider->setTickPosition(QSlider::TicksBothSides);

        horizontalLayout->addWidget(ganttSlider);


        horizontalLayout_2->addLayout(horizontalLayout);


        retranslateUi(ControlBarDockManager);

        QMetaObject::connectSlotsByName(ControlBarDockManager);
    } // setupUi

    void retranslateUi(QWidget *ControlBarDockManager)
    {
        ControlBarDockManager->setWindowTitle(QApplication::translate("ControlBarDockManager", "Form", 0));
        label_3->setText(QApplication::translate("ControlBarDockManager", "Level", 0));
#ifndef QT_NO_TOOLTIP
        levelSpinBox->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Adjusts debug level of client side.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Note that it means that messages with level &gt; level specified in the box will be dropped entirely and will not be sent to the server.</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        levelSpinBox->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
#ifndef QT_NO_TOOLTIP
        buffCheckBox->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Turns on/off buffering of messages on client side. </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">If buffering is turned off, all logged messages are sent synchronously. This forces the message to be sent immeadiately to the server. On the other side it affects negatively performance.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; tex"
                        "t-indent:0px;\"><span style=\" font-size:8pt;\">If unsure, leave buffering on.</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        buffCheckBox->setText(QApplication::translate("ControlBarDockManager", "buff", 0));
        label_5->setText(QApplication::translate("ControlBarDockManager", "Preset", 0));
#ifndef QT_NO_TOOLTIP
        presetComboBox->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Selects and applies saved preset file filter.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">If the name of the preset is preferably in the format Application/Name.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">In the special case where Name = default, the preset Application/default i"
                        "s loaded automaticaly on when the application Application connects to trace server.</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        activatePresetButton->setToolTip(QApplication::translate("ControlBarDockManager", "Applies currently selected preset", 0));
#endif // QT_NO_TOOLTIP
        activatePresetButton->setText(QApplication::translate("ControlBarDockManager", "Apply", 0));
#ifndef QT_NO_TOOLTIP
        presetSaveButton->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Saves current filter state as currently selected preset (or default if !exits)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        presetSaveButton->setText(QApplication::translate("ControlBarDockManager", "Save", 0));
#ifndef QT_NO_TOOLTIP
        presetAddButton->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Saves current state as new preset under new name.</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        presetAddButton->setText(QApplication::translate("ControlBarDockManager", "+", 0));
#ifndef QT_NO_TOOLTIP
        presetRmButton->setToolTip(QApplication::translate("ControlBarDockManager", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Removes currently selected preset</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        presetRmButton->setText(QApplication::translate("ControlBarDockManager", "-", 0));
#ifndef QT_NO_TOOLTIP
        presetResetButton->setToolTip(QApplication::translate("ControlBarDockManager", "clear current filter", 0));
#endif // QT_NO_TOOLTIP
        presetResetButton->setText(QApplication::translate("ControlBarDockManager", "Rst", 0));
        label_6->setText(QApplication::translate("ControlBarDockManager", "log", 0));
#ifndef QT_NO_TOOLTIP
        logSlider->setToolTip(QApplication::translate("ControlBarDockManager", "<html><head/><body><p><span style=\" font-weight:600;\">Left</span>   = data not received, no widget visible</p><p><span style=\" font-weight:600;\">Mid</span>    = data received, but no widget visible</p><p><span style=\" font-weight:600;\">Right</span> = data received, widget visible according to saved config</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("ControlBarDockManager", "plot", 0));
#ifndef QT_NO_TOOLTIP
        plotSlider->setToolTip(QApplication::translate("ControlBarDockManager", "<html><head/><body><p><span style=\" font-weight:600;\">Left</span>   = data not received, no widget visible</p><p><span style=\" font-weight:600;\">Mid</span>    = data received, but no widget visible</p><p><span style=\" font-weight:600;\">Right</span> = data received, widget visible according to saved config</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ControlBarDockManager", "table", 0));
#ifndef QT_NO_TOOLTIP
        tableSlider->setToolTip(QApplication::translate("ControlBarDockManager", "<html><head/><body><p><span style=\" font-weight:600;\">Left</span> = data not received, no widget visible</p><p><span style=\" font-weight:600;\">Mid</span> = data received, but no widget visible</p><p><span style=\" font-weight:600;\">Right</span> = data received, widget visible according to saved config</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("ControlBarDockManager", "gantt", 0));
    } // retranslateUi

};

namespace Ui {
    class ControlBarDockManager: public Ui_ControlBarDockManager {};
} // namespace Ui

QT_END_NAMESPACE


