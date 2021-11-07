#ifndef POLICYCONFIG_H
#define POLICYCONFIG_H

#include <QDialog>
#include <QtWidgets>

#include <QtSql>

namespace Ui {
class PolicyConfig;
}

class PolicyConfig : public QDialog
{
    Q_OBJECT

public:
    explicit PolicyConfig(QModelIndexList indexList, int policyId, int hostId, QSqlQuery* query, QWidget *parent = nullptr);
    ~PolicyConfig();

private slots:
    void on_sHourBox_currentIndexChanged(int index);

    void on_sMinBox_currentIndexChanged(int index);

    void on_eHourBox_currentIndexChanged(int index);

    void on_eMinBox_currentIndexChanged(int index);

    void on_applyButton_clicked();

    void on_deleteButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::PolicyConfig *ui;

    QList<bool> dayOfWeek_;

    QSqlQuery* query_;

    QTime sTime_;
    QTime eTime_;
    int sHIdx_;
    int sMIdx_;
    int eHIdx_;
    int eMIdx_;

    int hostId_;
    int policyId_;

    void setEndHourBox();
    void setEndMinBox();

};

#endif // POLICYCONFIG_H
