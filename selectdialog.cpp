#include "selectdialog.h"
#include "ui_selectdialog.h"
#include<QMessageBox>

selectDialog::selectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::selectDialog)
{
    ui->setupUi(this);
    QFont font1("Microsoft YaHei", 15);
    ui->label->setFont(font1);
}

selectDialog::~selectDialog()
{
    delete ui;
}

int selectDialog::getReturner()
{
    //QMessageBox::information(this,"提示",QString::number(toReturn));

    return(toReturn);
}


void selectDialog::on_pushButton_clicked()
{
    toReturn=1;
    accept(); //正确返回，使得exec()有正确值
}


void selectDialog::on_pushButton_2_clicked()
{
    toReturn=2;
    accept();
}


void selectDialog::on_pushButton_3_clicked()
{
    toReturn=3;
    accept();
}

