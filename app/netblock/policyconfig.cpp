#include "policyconfig.h"
#include "ui_policyconfig.h"

PolicyConfig::PolicyConfig(QModelIndexList indexList, int policyId, int hostId, QSqlQuery *query, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PolicyConfig)
{
    ui->setupUi(this);

    qDebug() <<"PolicyId: " << policyId;

    if(policyId == 0) {
        this->setWindowTitle("New");
    }
    else {
        this->setWindowTitle(QString::number(policyId));
    }

    hostId_ = hostId;
    policyId_ = policyId;
    for(int i = 0; i < 7; ++i) {
        dayOfWeek_.append(false);
    }

    query_ = query;

    QTime now = QTime::currentTime();
    sTime_ = QTime(now.hour(), now.minute() - now.minute() % 10);
    eTime_ = sTime_.addSecs(600);
    if(!policyId) {
        dayOfWeek_[indexList.constFirst().column()] = true;
        sTime_ = QTime(indexList.constFirst().row(), 0);
        eTime_ = QTime(indexList.constLast().row(), 0);
        if(indexList.constLast().row() + 1 == 24) {
            eTime_ = QTime(23, 50);
        }
    } else {
        ui->deleteButton->setEnabled(true);
        query_->prepare("SELECT host_id, start_time, end_time, day_of_the_week FROM policy WHERE policy_id = :policy_id");
        query_->bindValue(":policy_id", policyId);
        query_->exec();
        query_->next();
        int sHour = query_->value(1).toString().leftRef(2).toInt();
        int sMin = query_->value(1).toString().rightRef(2).toInt();
        int eHour = query_->value(2).toString().leftRef(2).toInt();
        int eMin = query_->value(2).toString().rightRef(2).toInt();
        sTime_ = QTime(sHour, sMin);
        eTime_ = QTime(eHour, eMin);
        dayOfWeek_[query_->value(3).toInt()] = true;

        hostId_ = query->value(0).toInt();

    }

    ui->sHourBox->setCurrentIndex(ui->sHourBox->findText(QString::number(sTime_.hour())));
    ui->sMinBox->setCurrentIndex(ui->sMinBox->findText(QString::number(sTime_.minute())));
    ui->eHourBox->setCurrentIndex(ui->eHourBox->findText(QString::number(eTime_.hour())));
    ui->eMinBox->setCurrentIndex(ui->eMinBox->findText(QString::number(eTime_.minute())));


    ui->dayOfTheWeekCheck_0->setChecked(dayOfWeek_[0]);
    ui->dayOfTheWeekCheck_1->setChecked(dayOfWeek_[1]);
    ui->dayOfTheWeekCheck_2->setChecked(dayOfWeek_[2]);
    ui->dayOfTheWeekCheck_3->setChecked(dayOfWeek_[3]);
    ui->dayOfTheWeekCheck_4->setChecked(dayOfWeek_[4]);
    ui->dayOfTheWeekCheck_5->setChecked(dayOfWeek_[5]);
    ui->dayOfTheWeekCheck_6->setChecked(dayOfWeek_[6]);

    QObject::connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

PolicyConfig::~PolicyConfig()
{
    delete ui;
}

void PolicyConfig::on_sHourBox_currentIndexChanged(int index)
{
	(void)index;

    sTime_ = QTime(ui->sHourBox->currentText().toInt(), ui->sMinBox->currentText().toInt());
}


void PolicyConfig::on_sMinBox_currentIndexChanged(int index)
{
	(void)index;

    sTime_ = QTime(sTime_.hour(), ui->sMinBox->currentText().toInt());
}


void PolicyConfig::on_eHourBox_currentIndexChanged(int index)
{
	(void)index;

    eTime_ = QTime(ui->eHourBox->currentText().toInt(), ui->eMinBox->currentText().toInt());
}

void PolicyConfig::on_eMinBox_currentIndexChanged(int index)
{
	(void)index;

    eTime_ = QTime(ui->eHourBox->currentText().toInt(), ui->eMinBox->currentText().toInt());
}

void PolicyConfig::on_applyButton_clicked()
{
    if(sTime_ < eTime_ || (ui->eHourBox->currentText().toInt() == 24 && ui->eMinBox->currentText().toInt() == 0)) {
        dayOfWeek_[0] = ui->dayOfTheWeekCheck_0->isChecked();
        dayOfWeek_[1] = ui->dayOfTheWeekCheck_1->isChecked();
        dayOfWeek_[2] = ui->dayOfTheWeekCheck_2->isChecked();
        dayOfWeek_[3] = ui->dayOfTheWeekCheck_3->isChecked();
        dayOfWeek_[4] = ui->dayOfTheWeekCheck_4->isChecked();
        dayOfWeek_[5] = ui->dayOfTheWeekCheck_5->isChecked();
        dayOfWeek_[6] = ui->dayOfTheWeekCheck_6->isChecked();

        if(policyId_ && dayOfWeek_.count(true) == 1) {
                query_->prepare("UPDATE policy SET start_time=:start_time, end_time=:end_time, day_of_week=:day_of_week WHERE policy_id=:policy_id");
                query_->bindValue(":start_time", QString("%1%2").arg(sTime_.hour(), 2, 10, QLatin1Char('0')).arg(sTime_.minute(), 2, 10, QLatin1Char('0')));
                query_->bindValue(":end_time", QString("%1%2").arg(eTime_.hour() < 0 ? 24 : eTime_.hour(), 2, 10, QLatin1Char('0')).arg(eTime_.minute() < 0 ? 0 : eTime_.minute(), 2, 10, QLatin1Char('0')));
                query_->bindValue(":day_of_week", QString::number(dayOfWeek_.indexOf(true)));
                query_->bindValue(":policy_id", policyId_);
                query_->exec();
        } else {
            if(policyId_) {
                query_->prepare("DELETE FROM policy WHERE policy_id=:policy_id");
                query_->bindValue(":policy_id", policyId_);
                query_->exec();
            }
            for(int i = 0; i < 7; ++i) {
                if(dayOfWeek_[i]) {
                    query_->prepare("INSERT INTO policy(host_id, start_time, end_time, day_of_the_week) VALUES(:host_id, :start_time, :end_time, :day_of_the_week)");
                    query_->bindValue(":host_id", hostId_);
                    query_->bindValue(":start_time", QString("%1%2").arg(sTime_.hour(), 2, 10, QLatin1Char('0')).arg(sTime_.minute(), 2, 10, QLatin1Char('0')));
                    query_->bindValue(":end_time", QString("%1%2").arg(eTime_.hour() < 0 ? 24 : eTime_.hour(), 2, 10, QLatin1Char('0')).arg(eTime_.minute() < 0 ? 0 : eTime_.minute(), 2, 10, QLatin1Char('0')));
                    query_->bindValue(":day_of_the_week", i);
                    query_->exec();
                }
            }
        }
        close();
    } else if(eTime_.hour() < 0) {
        QMessageBox::warning(this, "Warning", "You Check EndTime Over than 24:00");
    } else {
        QMessageBox::warning(this, "Warning", "EndTime must be greater then StartTime");
    }

}

void PolicyConfig::on_deleteButton_clicked()
{
    query_->prepare("DELETE FROM policy WHERE policy_id = :policy_id");
    query_->bindValue(":policy_id", policyId_);
    query_->exec();
    close();
}

void PolicyConfig::on_cancelButton_clicked()
{
    close();
}


