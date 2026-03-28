/********************************************************************************
** Form generated from reading UI file 'minewindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MINEWINDOW_H
#define UI_MINEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MineWindow
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MineWindow)
    {
        if (MineWindow->objectName().isEmpty())
            MineWindow->setObjectName("MineWindow");
        MineWindow->resize(400, 400);
        centralWidget = new QWidget(MineWindow);
        centralWidget->setObjectName("centralWidget");
        MineWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MineWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 400, 21));
        MineWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MineWindow);
        statusBar->setObjectName("statusBar");
        MineWindow->setStatusBar(statusBar);

        retranslateUi(MineWindow);

        QMetaObject::connectSlotsByName(MineWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MineWindow)
    {
        MineWindow->setWindowTitle(QCoreApplication::translate("MineWindow", "\346\211\253\351\233\267", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MineWindow: public Ui_MineWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MINEWINDOW_H
