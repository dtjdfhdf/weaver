/********************************************************************************
** Form generated from reading UI file 'weavermanualalign.ui'
**
** Created: Tue Feb 22 12:08:25 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEAVERMANUALALIGN_H
#define UI_WEAVERMANUALALIGN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WeaverManualAlign
{
public:
    QWidget *centralWidget;
    QFrame *glFbFrame;
    QGroupBox *mainUiGroupBox;
    QTextEdit *yPosTextEdit;
    QSlider *brightnessSlider;
    QPushButton *submitPushButton;
    QTextEdit *xPosTextEdit;
    QLabel *label_4;
    QLabel *label_2;
    QPushButton *autoPosPushButton;
    QSlider *contrastSlider;
    QPushButton *pushButton;
    QTextBrowser *statusTextBrowser;
    QLabel *label_3;
    QLabel *label;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *WeaverManualAlign)
    {
        if (WeaverManualAlign->objectName().isEmpty())
            WeaverManualAlign->setObjectName(QString::fromUtf8("WeaverManualAlign"));
        WeaverManualAlign->resize(1151, 922);
        centralWidget = new QWidget(WeaverManualAlign);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        glFbFrame = new QFrame(centralWidget);
        glFbFrame->setObjectName(QString::fromUtf8("glFbFrame"));
        glFbFrame->setGeometry(QRect(10, 10, 1131, 781));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(glFbFrame->sizePolicy().hasHeightForWidth());
        glFbFrame->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(9);
        glFbFrame->setFont(font);
        glFbFrame->setFrameShape(QFrame::StyledPanel);
        glFbFrame->setFrameShadow(QFrame::Sunken);
        mainUiGroupBox = new QGroupBox(centralWidget);
        mainUiGroupBox->setObjectName(QString::fromUtf8("mainUiGroupBox"));
        mainUiGroupBox->setGeometry(QRect(10, 800, 1131, 101));
        yPosTextEdit = new QTextEdit(mainUiGroupBox);
        yPosTextEdit->setObjectName(QString::fromUtf8("yPosTextEdit"));
        yPosTextEdit->setGeometry(QRect(810, 70, 51, 27));
        yPosTextEdit->setFont(font);
        yPosTextEdit->setAcceptDrops(false);
        yPosTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
        brightnessSlider = new QSlider(mainUiGroupBox);
        brightnessSlider->setObjectName(QString::fromUtf8("brightnessSlider"));
        brightnessSlider->setGeometry(QRect(630, 0, 181, 21));
        brightnessSlider->setFont(font);
        brightnessSlider->setValue(0);
        brightnessSlider->setSliderPosition(0);
        brightnessSlider->setOrientation(Qt::Horizontal);
        submitPushButton = new QPushButton(mainUiGroupBox);
        submitPushButton->setObjectName(QString::fromUtf8("submitPushButton"));
        submitPushButton->setGeometry(QRect(1020, 20, 111, 51));
        submitPushButton->setFont(font);
        xPosTextEdit = new QTextEdit(mainUiGroupBox);
        xPosTextEdit->setObjectName(QString::fromUtf8("xPosTextEdit"));
        xPosTextEdit->setGeometry(QRect(730, 70, 51, 27));
        xPosTextEdit->setFont(font);
        xPosTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
        label_4 = new QLabel(mainUiGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(540, 30, 71, 16));
        label_4->setFont(font);
        label_2 = new QLabel(mainUiGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(790, 73, 16, 16));
        label_2->setFont(font);
        autoPosPushButton = new QPushButton(mainUiGroupBox);
        autoPosPushButton->setObjectName(QString::fromUtf8("autoPosPushButton"));
        autoPosPushButton->setGeometry(QRect(900, 20, 111, 51));
        autoPosPushButton->setFont(font);
        contrastSlider = new QSlider(mainUiGroupBox);
        contrastSlider->setObjectName(QString::fromUtf8("contrastSlider"));
        contrastSlider->setGeometry(QRect(630, 30, 181, 21));
        contrastSlider->setFont(font);
        contrastSlider->setValue(50);
        contrastSlider->setSliderPosition(50);
        contrastSlider->setOrientation(Qt::Horizontal);
        pushButton = new QPushButton(mainUiGroupBox);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(820, 2, 51, 41));
        pushButton->setFont(font);
        statusTextBrowser = new QTextBrowser(mainUiGroupBox);
        statusTextBrowser->setObjectName(QString::fromUtf8("statusTextBrowser"));
        statusTextBrowser->setGeometry(QRect(0, 0, 511, 91));
        statusTextBrowser->setFont(font);
        label_3 = new QLabel(mainUiGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(540, 0, 81, 16));
        label_3->setFont(font);
        label = new QLabel(mainUiGroupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(540, 72, 191, 20));
        label->setFont(font);
        WeaverManualAlign->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(WeaverManualAlign);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        WeaverManualAlign->setStatusBar(statusBar);

        retranslateUi(WeaverManualAlign);

        QMetaObject::connectSlotsByName(WeaverManualAlign);
    } // setupUi

    void retranslateUi(QMainWindow *WeaverManualAlign)
    {
        WeaverManualAlign->setWindowTitle(QApplication::translate("WeaverManualAlign", "Weaver Manual Alignment Tool", 0, QApplication::UnicodeUTF8));
        mainUiGroupBox->setTitle(QString());
#ifndef QT_NO_TOOLTIP
        brightnessSlider->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        submitPushButton->setText(QApplication::translate("WeaverManualAlign", "Submit", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_4->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("WeaverManualAlign", "Contrast", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("WeaverManualAlign", "X", 0, QApplication::UnicodeUTF8));
        autoPosPushButton->setText(QApplication::translate("WeaverManualAlign", "Position in grid", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        contrastSlider->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        pushButton->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        pushButton->setText(QApplication::translate("WeaverManualAlign", "Reset", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_3->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("WeaverManualAlign", "Brightness", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("WeaverManualAlign", "Relative position of Image 2:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class WeaverManualAlign: public Ui_WeaverManualAlign {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEAVERMANUALALIGN_H
