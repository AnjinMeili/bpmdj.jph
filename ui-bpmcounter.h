/********************************************************************************
** Form generated from reading UI file 'ui-bpmcounter.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_2D_BPMCOUNTER_H
#define UI_2D_BPMCOUNTER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CountDialog
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QLabel *BpmPix;
    QProgressBar *ReadingBar;
    QHBoxLayout *hboxLayout;
    QPushButton *StartStop;
    QSpacerItem *spacerItem;
    QLabel *TextLabel1_2;
    QSpinBox *SkipBox;
    QPushButton *resetButton;
    QLCDNumber *TapLcd;
    QPushButton *tapButton;
    QHBoxLayout *horizontalLayout;
    QLabel *StatusLabel;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QLabel *label_3;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *FromBpmEdit;
    QLineEdit *ToBpmEdit;
    QLabel *label;
    QGroupBox *algorithm;
    QGridLayout *gridLayout1;
    QRadioButton *weightedEnvCor;
    QRadioButton *fullAutoCorrelation;
    QRadioButton *enveloppeSpectrum;
    QRadioButton *resamplingScan;
    QRadioButton *experimentalAlg;

    void setupUi(QWidget *CountDialog)
    {
        if (CountDialog->objectName().isEmpty())
            CountDialog->setObjectName(QString::fromUtf8("CountDialog"));
        CountDialog->resize(480, 305);
        gridLayout = new QGridLayout(CountDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        BpmPix = new QLabel(CountDialog);
        BpmPix->setObjectName(QString::fromUtf8("BpmPix"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(BpmPix->sizePolicy().hasHeightForWidth());
        BpmPix->setSizePolicy(sizePolicy);
        BpmPix->setMinimumSize(QSize(0, 200));
        BpmPix->setCursor(QCursor(Qt::ArrowCursor));
        BpmPix->setFrameShape(QFrame::Panel);
        BpmPix->setFrameShadow(QFrame::Sunken);
        BpmPix->setScaledContents(true);
        BpmPix->setWordWrap(false);

        verticalLayout->addWidget(BpmPix);

        ReadingBar = new QProgressBar(CountDialog);
        ReadingBar->setObjectName(QString::fromUtf8("ReadingBar"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ReadingBar->sizePolicy().hasHeightForWidth());
        ReadingBar->setSizePolicy(sizePolicy1);
        ReadingBar->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(ReadingBar);


        gridLayout->addLayout(verticalLayout, 2, 1, 5, 1);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        StartStop = new QPushButton(CountDialog);
        StartStop->setObjectName(QString::fromUtf8("StartStop"));
        StartStop->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(StartStop->sizePolicy().hasHeightForWidth());
        StartStop->setSizePolicy(sizePolicy2);

        hboxLayout->addWidget(StartStop);

        spacerItem = new QSpacerItem(91, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        TextLabel1_2 = new QLabel(CountDialog);
        TextLabel1_2->setObjectName(QString::fromUtf8("TextLabel1_2"));
        TextLabel1_2->setWordWrap(false);

        hboxLayout->addWidget(TextLabel1_2);

        SkipBox = new QSpinBox(CountDialog);
        SkipBox->setObjectName(QString::fromUtf8("SkipBox"));
        SkipBox->setMinimum(1);
        SkipBox->setMaximum(32);
        SkipBox->setValue(4);

        hboxLayout->addWidget(SkipBox);

        resetButton = new QPushButton(CountDialog);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));

        hboxLayout->addWidget(resetButton);

        TapLcd = new QLCDNumber(CountDialog);
        TapLcd->setObjectName(QString::fromUtf8("TapLcd"));
        TapLcd->setNumDigits(4);

        hboxLayout->addWidget(TapLcd);

        tapButton = new QPushButton(CountDialog);
        tapButton->setObjectName(QString::fromUtf8("tapButton"));

        hboxLayout->addWidget(tapButton);


        gridLayout->addLayout(hboxLayout, 7, 0, 1, 2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        StatusLabel = new QLabel(CountDialog);
        StatusLabel->setObjectName(QString::fromUtf8("StatusLabel"));
        StatusLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(StatusLabel);


        gridLayout->addLayout(horizontalLayout, 8, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 3, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_2 = new QLabel(CountDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_3->addWidget(label_2);

        label_3 = new QLabel(CountDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_3->addWidget(label_3);


        horizontalLayout_2->addLayout(verticalLayout_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        FromBpmEdit = new QLineEdit(CountDialog);
        FromBpmEdit->setObjectName(QString::fromUtf8("FromBpmEdit"));
        FromBpmEdit->setEnabled(true);
        sizePolicy2.setHeightForWidth(FromBpmEdit->sizePolicy().hasHeightForWidth());
        FromBpmEdit->setSizePolicy(sizePolicy2);
        FromBpmEdit->setMinimumSize(QSize(50, 0));
        FromBpmEdit->setFrame(false);

        verticalLayout_2->addWidget(FromBpmEdit);

        ToBpmEdit = new QLineEdit(CountDialog);
        ToBpmEdit->setObjectName(QString::fromUtf8("ToBpmEdit"));
        ToBpmEdit->setEnabled(true);
        sizePolicy2.setHeightForWidth(ToBpmEdit->sizePolicy().hasHeightForWidth());
        ToBpmEdit->setSizePolicy(sizePolicy2);
        ToBpmEdit->setFrame(false);

        verticalLayout_2->addWidget(ToBpmEdit);


        horizontalLayout_2->addLayout(verticalLayout_2);


        gridLayout->addLayout(horizontalLayout_2, 5, 0, 1, 1);

        label = new QLabel(CountDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 4, 0, 1, 1);

        algorithm = new QGroupBox(CountDialog);
        algorithm->setObjectName(QString::fromUtf8("algorithm"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(algorithm->sizePolicy().hasHeightForWidth());
        algorithm->setSizePolicy(sizePolicy3);
        gridLayout1 = new QGridLayout(algorithm);
        gridLayout1->setSpacing(0);
        gridLayout1->setContentsMargins(0, 0, 0, 0);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        weightedEnvCor = new QRadioButton(algorithm);
        weightedEnvCor->setObjectName(QString::fromUtf8("weightedEnvCor"));
        weightedEnvCor->setChecked(false);

        gridLayout1->addWidget(weightedEnvCor, 3, 0, 1, 1);

        fullAutoCorrelation = new QRadioButton(algorithm);
        fullAutoCorrelation->setObjectName(QString::fromUtf8("fullAutoCorrelation"));

        gridLayout1->addWidget(fullAutoCorrelation, 2, 0, 1, 1);

        enveloppeSpectrum = new QRadioButton(algorithm);
        enveloppeSpectrum->setObjectName(QString::fromUtf8("enveloppeSpectrum"));

        gridLayout1->addWidget(enveloppeSpectrum, 1, 0, 1, 1);

        resamplingScan = new QRadioButton(algorithm);
        resamplingScan->setObjectName(QString::fromUtf8("resamplingScan"));
        resamplingScan->setChecked(true);

        gridLayout1->addWidget(resamplingScan, 0, 0, 1, 1);

        experimentalAlg = new QRadioButton(algorithm);
        experimentalAlg->setObjectName(QString::fromUtf8("experimentalAlg"));

        gridLayout1->addWidget(experimentalAlg, 4, 0, 1, 1);


        gridLayout->addWidget(algorithm, 6, 0, 1, 1);


        retranslateUi(CountDialog);
        QObject::connect(StartStop, SIGNAL(clicked()), CountDialog, SLOT(startStopButton()));
        QObject::connect(tapButton, SIGNAL(pressed()), CountDialog, SLOT(tap()));
        QObject::connect(resetButton, SIGNAL(pressed()), CountDialog, SLOT(reset()));

        QMetaObject::connectSlotsByName(CountDialog);
    } // setupUi

    void retranslateUi(QWidget *CountDialog)
    {
        CountDialog->setWindowTitle(QApplication::translate("CountDialog", "Form", 0, QApplication::UnicodeUTF8));
        StartStop->setText(QApplication::translate("CountDialog", "&Calculate", 0, QApplication::UnicodeUTF8));
        StartStop->setShortcut(QApplication::translate("CountDialog", "Alt+S", 0, QApplication::UnicodeUTF8));
        TextLabel1_2->setText(QApplication::translate("CountDialog", "Beats/tap", 0, QApplication::UnicodeUTF8));
        resetButton->setText(QApplication::translate("CountDialog", "&Reset", 0, QApplication::UnicodeUTF8));
        resetButton->setShortcut(QApplication::translate("CountDialog", "Alt+R", 0, QApplication::UnicodeUTF8));
        tapButton->setText(QApplication::translate("CountDialog", "&Tap", 0, QApplication::UnicodeUTF8));
        tapButton->setShortcut(QApplication::translate("CountDialog", "Alt+T", 0, QApplication::UnicodeUTF8));
        StatusLabel->setText(QApplication::translate("CountDialog", "Automatic count left, manual counter right.", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("CountDialog", "From:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("CountDialog", "To:", 0, QApplication::UnicodeUTF8));
        FromBpmEdit->setText(QApplication::translate("CountDialog", "80", 0, QApplication::UnicodeUTF8));
        ToBpmEdit->setText(QApplication::translate("CountDialog", "160", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("CountDialog", "Search bounds", 0, QApplication::UnicodeUTF8));
        algorithm->setTitle(QApplication::translate("CountDialog", "Algorithm", 0, QApplication::UnicodeUTF8));
        weightedEnvCor->setText(QApplication::translate("CountDialog", "Weighted", 0, QApplication::UnicodeUTF8));
        fullAutoCorrelation->setText(QApplication::translate("CountDialog", "Autocorrelation", 0, QApplication::UnicodeUTF8));
        enveloppeSpectrum->setText(QApplication::translate("CountDialog", "Enveloppe Spectrum", 0, QApplication::UnicodeUTF8));
        resamplingScan->setText(QApplication::translate("CountDialog", "Autodifference", 0, QApplication::UnicodeUTF8));
        experimentalAlg->setText(QApplication::translate("CountDialog", "Bpm2010", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CountDialog: public Ui_CountDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_2D_BPMCOUNTER_H
