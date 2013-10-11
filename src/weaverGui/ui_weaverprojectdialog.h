/********************************************************************************
** Form generated from reading UI file 'weaverprojectdialog.ui'
**
** Created: Tue Feb 22 12:08:25 2011
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEAVERPROJECTDIALOG_H
#define UI_WEAVERPROJECTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_weaverProjectDialog
{
public:
    QFrame *frame;
    QLabel *label_7;
    QPushButton *chooseOutputImageFileButton;
    QCheckBox *multiResTiffCheckBox;
    QLabel *label_8;
    QPushButton *chooseTempDirButton;
    QLabel *label_9;
    QPushButton *choosePiecelistFileButton;
    QLabel *label_10;
    QSpinBox *montageNumOfColsSpinBox;
    QLabel *label_11;
    QSpinBox *montageNumOfRowsSpinBox;
    QCheckBox *manualAlignCheckBox;
    QLineEdit *outTiffFileTextEdit;
    QLineEdit *tmpDirTextEdit;
    QLineEdit *piecelistTextEdit;
    QLabel *label_12;
    QLabel *label;
    QPushButton *exitButton;
    QPushButton *startStitchingButton;

    void setupUi(QDialog *weaverProjectDialog)
    {
        if (weaverProjectDialog->objectName().isEmpty())
            weaverProjectDialog->setObjectName(QString::fromUtf8("weaverProjectDialog"));
        weaverProjectDialog->resize(501, 416);
        frame = new QFrame(weaverProjectDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(10, 30, 481, 311));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        label_7 = new QLabel(frame);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 30, 131, 17));
        QFont font;
        font.setPointSize(9);
        label_7->setFont(font);
        chooseOutputImageFileButton = new QPushButton(frame);
        chooseOutputImageFileButton->setObjectName(QString::fromUtf8("chooseOutputImageFileButton"));
        chooseOutputImageFileButton->setGeometry(QRect(430, 20, 31, 31));
        multiResTiffCheckBox = new QCheckBox(frame);
        multiResTiffCheckBox->setObjectName(QString::fromUtf8("multiResTiffCheckBox"));
        multiResTiffCheckBox->setGeometry(QRect(160, 60, 311, 22));
        multiResTiffCheckBox->setFont(font);
        multiResTiffCheckBox->setChecked(true);
        label_8 = new QLabel(frame);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 130, 131, 17));
        label_8->setFont(font);
        chooseTempDirButton = new QPushButton(frame);
        chooseTempDirButton->setObjectName(QString::fromUtf8("chooseTempDirButton"));
        chooseTempDirButton->setGeometry(QRect(430, 120, 31, 31));
        label_9 = new QLabel(frame);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(20, 240, 131, 17));
        label_9->setFont(font);
        choosePiecelistFileButton = new QPushButton(frame);
        choosePiecelistFileButton->setObjectName(QString::fromUtf8("choosePiecelistFileButton"));
        choosePiecelistFileButton->setGeometry(QRect(430, 230, 31, 31));
        label_10 = new QLabel(frame);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(20, 200, 101, 17));
        label_10->setFont(font);
        montageNumOfColsSpinBox = new QSpinBox(frame);
        montageNumOfColsSpinBox->setObjectName(QString::fromUtf8("montageNumOfColsSpinBox"));
        montageNumOfColsSpinBox->setGeometry(QRect(230, 190, 71, 30));
        montageNumOfColsSpinBox->setFont(font);
        montageNumOfColsSpinBox->setMaximum(10000);
        montageNumOfColsSpinBox->setValue(1);
        label_11 = new QLabel(frame);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(310, 200, 41, 17));
        label_11->setFont(font);
        montageNumOfRowsSpinBox = new QSpinBox(frame);
        montageNumOfRowsSpinBox->setObjectName(QString::fromUtf8("montageNumOfRowsSpinBox"));
        montageNumOfRowsSpinBox->setGeometry(QRect(350, 190, 71, 30));
        montageNumOfRowsSpinBox->setFont(font);
        montageNumOfRowsSpinBox->setMaximum(10000);
        montageNumOfRowsSpinBox->setValue(1);
        manualAlignCheckBox = new QCheckBox(frame);
        manualAlignCheckBox->setObjectName(QString::fromUtf8("manualAlignCheckBox"));
        manualAlignCheckBox->setGeometry(QRect(20, 280, 451, 22));
        manualAlignCheckBox->setFont(font);
        manualAlignCheckBox->setChecked(true);
        outTiffFileTextEdit = new QLineEdit(frame);
        outTiffFileTextEdit->setObjectName(QString::fromUtf8("outTiffFileTextEdit"));
        outTiffFileTextEdit->setGeometry(QRect(160, 20, 261, 31));
        outTiffFileTextEdit->setFont(font);
        outTiffFileTextEdit->setReadOnly(true);
        tmpDirTextEdit = new QLineEdit(frame);
        tmpDirTextEdit->setObjectName(QString::fromUtf8("tmpDirTextEdit"));
        tmpDirTextEdit->setGeometry(QRect(160, 120, 261, 31));
        tmpDirTextEdit->setFont(font);
        tmpDirTextEdit->setReadOnly(true);
        piecelistTextEdit = new QLineEdit(frame);
        piecelistTextEdit->setObjectName(QString::fromUtf8("piecelistTextEdit"));
        piecelistTextEdit->setGeometry(QRect(160, 230, 261, 31));
        piecelistTextEdit->setFont(font);
        piecelistTextEdit->setReadOnly(true);
        label_12 = new QLabel(frame);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(160, 200, 71, 20));
        label_12->setFont(font);
        label = new QLabel(weaverProjectDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 20, 151, 21));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        label->setAutoFillBackground(true);
        label->setFrameShape(QFrame::Box);
        label->setFrameShadow(QFrame::Sunken);
        exitButton = new QPushButton(weaverProjectDialog);
        exitButton->setObjectName(QString::fromUtf8("exitButton"));
        exitButton->setGeometry(QRect(280, 350, 161, 41));
        startStitchingButton = new QPushButton(weaverProjectDialog);
        startStitchingButton->setObjectName(QString::fromUtf8("startStitchingButton"));
        startStitchingButton->setGeometry(QRect(60, 350, 171, 41));

        retranslateUi(weaverProjectDialog);

        QMetaObject::connectSlotsByName(weaverProjectDialog);
    } // setupUi

    void retranslateUi(QDialog *weaverProjectDialog)
    {
        weaverProjectDialog->setWindowTitle(QApplication::translate("weaverProjectDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("weaverProjectDialog", "Output Image File:", 0, QApplication::UnicodeUTF8));
        chooseOutputImageFileButton->setText(QApplication::translate("weaverProjectDialog", "...", 0, QApplication::UnicodeUTF8));
        multiResTiffCheckBox->setText(QApplication::translate("weaverProjectDialog", "Create Multi-resolution (Pyramidal)TIFF", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("weaverProjectDialog", "Temporary directory", 0, QApplication::UnicodeUTF8));
        chooseTempDirButton->setText(QApplication::translate("weaverProjectDialog", "...", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("weaverProjectDialog", "Piecelist file:", 0, QApplication::UnicodeUTF8));
        choosePiecelistFileButton->setText(QApplication::translate("weaverProjectDialog", "...", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("weaverProjectDialog", "Size of montage :", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("weaverProjectDialog", "Rows:", 0, QApplication::UnicodeUTF8));
        manualAlignCheckBox->setText(QApplication::translate("weaverProjectDialog", "Use manual alignment as backup for cross-correlation", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("weaverProjectDialog", "Columns:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("weaverProjectDialog", "Weaver Options", 0, QApplication::UnicodeUTF8));
        exitButton->setText(QApplication::translate("weaverProjectDialog", "Exit", 0, QApplication::UnicodeUTF8));
        startStitchingButton->setText(QApplication::translate("weaverProjectDialog", "Start Stitching", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class weaverProjectDialog: public Ui_weaverProjectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEAVERPROJECTDIALOG_H
