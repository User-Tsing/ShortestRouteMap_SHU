#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include <QDialog>

namespace Ui {
class selectDialog;
}

class selectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit selectDialog(QWidget *parent = nullptr);
    ~selectDialog();
    int toReturn;
    int getReturner();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::selectDialog *ui;
};

#endif // SELECTDIALOG_H
