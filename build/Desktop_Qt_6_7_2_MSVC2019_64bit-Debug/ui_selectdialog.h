/********************************************************************************
** Form generated from reading UI file 'selectdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTDIALOG_H
#define UI_SELECTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_selectDialog
{
public:
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QLabel *label;

    void setupUi(QDialog *selectDialog)
    {
        if (selectDialog->objectName().isEmpty())
            selectDialog->setObjectName("selectDialog");
        selectDialog->resize(463, 300);
        pushButton = new QPushButton(selectDialog);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(20, 190, 121, 51));
        pushButton_2 = new QPushButton(selectDialog);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(160, 190, 121, 51));
        pushButton_3 = new QPushButton(selectDialog);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(300, 190, 121, 51));
        label = new QLabel(selectDialog);
        label->setObjectName("label");
        label->setGeometry(QRect(20, 50, 401, 101));
        label->setTextFormat(Qt::TextFormat::AutoText);
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);

        retranslateUi(selectDialog);

        QMetaObject::connectSlotsByName(selectDialog);
    } // setupUi

    void retranslateUi(QDialog *selectDialog)
    {
        selectDialog->setWindowTitle(QCoreApplication::translate("selectDialog", "Dialog", nullptr));
        pushButton->setText(QCoreApplication::translate("selectDialog", "\347\251\272\346\227\267", nullptr));
        pushButton_2->setText(QCoreApplication::translate("selectDialog", "\346\255\243\345\270\270", nullptr));
        pushButton_3->setText(QCoreApplication::translate("selectDialog", "\346\213\245\345\240\265", nullptr));
        label->setText(QCoreApplication::translate("selectDialog", "\350\257\267\351\200\211\346\213\251\344\275\240\350\246\201\344\277\256\346\224\271\347\232\204\350\267\257\345\276\204\350\267\257\345\206\265", nullptr));
    } // retranslateUi

};

namespace Ui {
    class selectDialog: public Ui_selectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTDIALOG_H
