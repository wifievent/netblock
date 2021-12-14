#include "policyconfig.h"
#include "ui_policyconfig.h"

#include <glog/logging.h>

PolicyConfig::PolicyConfig(QModelIndexList indexList, int policyId, int hostId, DBConnect *connect, QWidget *parent) : QDialog(parent),
                                                                                                                                ui(new Ui::PolicyConfig)
{
    ui->setupUi(this);

    ui->sHourBox->setEditable(true);
    ui->sMinBox->setEditable(true);
    ui->eHourBox->setEditable(true);
    ui->eMinBox->setEditable(true);

    DLOG(INFO) << "PolicyId: " << policyId;

    if (policyId == 0)
    {
        this->setWindowTitle("New");
    }
    else
    {
        this->setWindowTitle(QString::number(policyId));
    }

    hostId_ = hostId;
    policyId_ = policyId;
    connect_ = connect;

    for (int i = 0; i < 7; ++i)
    {
        dayOfWeek_.append(false);
    }

    QTime now = QTime::currentTime();
    sTime_ = QTime(now.hour(), now.minute() - now.minute() % 10);
    eTime_ = sTime_.addSecs(600);
    if (!policyId)
    {
        for (QModelIndexList::iterator iter = indexList.begin(); iter != indexList.end(); ++iter)
        {
            dayOfWeek_[iter->column()] = true;
        }
        sTime_ = QTime(indexList.constFirst().row(), 0);
        eTime_ = QTime(indexList.constLast().row() + 1, 0);
        if (indexList.constLast().row() + 1 == 24)
        {
            eTime_ = QTime(23, 59);
        }
    }
    else
    {
        ui->deleteButton->setEnabled(true);

        std::string query("SELECT host_id, start_time, end_time, day_of_the_week FROM policy WHERE policy_id = :policy_id");
        query.replace(query.find(":policy_id"), std::string(":policy_id").length(), std::to_string(policyId));

        std::list<DataList> dl = connect_->selectQuery(query);

        DataList data = dl.front();

        int sHour = std::stoi(data.argv_[1].substr(0, 2));
        int sMin = std::stoi(data.argv_[1].substr(2));
        int eHour = std::stoi(data.argv_[2].substr(0, 2));
        int eMin = std::stoi(data.argv_[2].substr(2));

        sTime_ = QTime(sHour, sMin);
        eTime_ = QTime(eHour, eMin);
        dayOfWeek_[std::stoi(data.argv_[3])] = true;

        hostId_ = std::stoi(data.argv_[0]);

        DLOG(INFO) << "else in" << sTime_.hour() << sTime_.minute() << eTime_.hour() << eTime_.minute();
    }

    ui->sHourBox->setCurrentText(QString::number(sTime_.hour()));
    ui->sMinBox->setCurrentText(QString::number(sTime_.minute()));
    ui->eHourBox->setCurrentText(QString::number(eTime_.hour() < 0 ? 24 : eTime_.hour()));
    ui->eMinBox->setCurrentText(QString::number(eTime_.minute() < 0 ? 0 : eTime_.minute()));

    ui->dayOfTheWeekCheck_0->setChecked(dayOfWeek_[0]);
    ui->dayOfTheWeekCheck_1->setChecked(dayOfWeek_[1]);
    ui->dayOfTheWeekCheck_2->setChecked(dayOfWeek_[2]);
    ui->dayOfTheWeekCheck_3->setChecked(dayOfWeek_[3]);
    ui->dayOfTheWeekCheck_4->setChecked(dayOfWeek_[4]);
    ui->dayOfTheWeekCheck_5->setChecked(dayOfWeek_[5]);
    ui->dayOfTheWeekCheck_6->setChecked(dayOfWeek_[6]);

    qDebug() << "end " << sTime_.hour() << sTime_.minute() << eTime_.hour() << eTime_.minute();
}

PolicyConfig::~PolicyConfig()
{
    delete ui;
}

void PolicyConfig::on_sHourBox_currentIndexChanged(int index)
{
    (void)index;

    sTime_ = QTime(ui->sHourBox->currentText().toInt(), sTime_.minute());
}

void PolicyConfig::on_sMinBox_currentIndexChanged(int index)
{
    (void)index;

    sTime_ = QTime(sTime_.hour(), ui->sMinBox->currentText().toInt());
}

void PolicyConfig::on_eHourBox_currentIndexChanged(int index)
{
    (void)index;

    eTime_ = QTime(ui->eHourBox->currentText().toInt(), eTime_.minute() < 0 ? ui->eMinBox->currentText().toInt() : eTime_.minute());
}

void PolicyConfig::on_eMinBox_currentIndexChanged(int index)
{
    (void)index;

    eTime_ = QTime(eTime_.hour(), ui->eMinBox->currentText().toInt());
}

template<typename ... Args>
std::string format_string(const std::string& format, Args ... args)
{
size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
std::unique_ptr<char[]> buffer(new char[size]);
snprintf(buffer.get(), size, format.c_str(), args ...);
return std::string(buffer.get(), buffer.get() + size - 1);
}

void PolicyConfig::on_applyButton_clicked()
{
    DLOG(INFO) << sTime_.hour() << sTime_.minute() << eTime_.hour() << eTime_.minute();
    if (sTime_ < eTime_ || (ui->eHourBox->currentText().toInt() == 24 && ui->eMinBox->currentText().toInt() == 0))
    {
        dayOfWeek_[0] = ui->dayOfTheWeekCheck_0->isChecked();
        dayOfWeek_[1] = ui->dayOfTheWeekCheck_1->isChecked();
        dayOfWeek_[2] = ui->dayOfTheWeekCheck_2->isChecked();
        dayOfWeek_[3] = ui->dayOfTheWeekCheck_3->isChecked();
        dayOfWeek_[4] = ui->dayOfTheWeekCheck_4->isChecked();
        dayOfWeek_[5] = ui->dayOfTheWeekCheck_5->isChecked();
        dayOfWeek_[6] = ui->dayOfTheWeekCheck_6->isChecked();

        if(policyId_ && dayOfWeek_.count(true) == 1)
        {
            std::string query("UPDATE policy SET start_time=':start_time', end_time=':end_time', day_of_the_week=:day_of_the_week WHERE policy_id=:policy_id");
            query.replace(query.find(":start_time"), std::string(":start_time").length(), (sTime_.hour() >= 10 ? std::to_string(sTime_.hour()) : std::to_string(0) + std::to_string(sTime_.hour())) + (sTime_.minute() >= 10 ? std::to_string(sTime_.minute()) : std::to_string(0) + std::to_string(sTime_.minute())));
            query.replace(query.find(":end_time"), std::string(":end_time").length(), (eTime_.hour() < 0 ? std::to_string(24) : (eTime_.hour() >= 10 ? std::to_string(eTime_.hour()) : (std::to_string(0) + std::to_string(eTime_.hour())))) + (eTime_.minute() >= 10 ? std::to_string(eTime_.minute()) : (std::to_string(0) + std::to_string(eTime_.minute()))));
            query.replace(query.find(":day_of_the_week"), std::string(":day_of_the_week").length(), std::to_string((dayOfWeek_.indexOf(true))));
            query.replace(query.find(":policy_id"), std::string(":policy_id").length(), std::to_string(policyId_));
            connect_->sendQuery(query);
        }
        else
        {
            if (policyId_)
            {
                std::string query("DELETE FROM policy WHERE policy_id=:policy_id");
                query.replace(query.find(":policy_id"), std::string(":policy_id").length(), std::to_string(policyId_));
                connect_->sendQuery(query);
            }
            for (int i = 0; i < 7; ++i)
            {
                if (dayOfWeek_[i])
                {
                    std::string query("INSERT INTO policy(host_id, start_time, end_time, day_of_the_week) VALUES(:host_id, ':start_time', ':end_time', :day_of_the_week)");
                    query.replace(query.find(":host_id"), std::string(":host_id").length(), std::to_string(hostId_));
                    query.replace(query.find(":start_time"), std::string(":start_time").length(), (sTime_.hour() >= 10 ? std::to_string(sTime_.hour()) : std::to_string(0) + std::to_string(sTime_.hour())) + (sTime_.minute() >= 10 ? std::to_string(sTime_.minute()) : std::to_string(0) + std::to_string(sTime_.minute())));
                    query.replace(query.find(":end_time"), std::string(":end_time").length(), (eTime_.hour() < 0 ? std::to_string(24) : (eTime_.hour() >= 10 ? std::to_string(eTime_.hour()) : (std::to_string(0) + std::to_string(eTime_.hour())))) + (eTime_.minute() >= 10 ? std::to_string(eTime_.minute()) : (std::to_string(0) + std::to_string(eTime_.minute()))));
                    query.replace(query.find(":day_of_the_week"), std::string(":day_of_the_week").length(), std::to_string(i));
                }
            }
        }

        accept();
        close();
    }
    else if (eTime_.hour() < 0)
    {
        QMessageBox::warning(this, "Warning", "You Check EndTime Over than 24:00");
    }
    else
    {
        QMessageBox::warning(this, "Warning", "EndTime must be greater then StartTime");
    }
}

void PolicyConfig::on_deleteButton_clicked()
{
    std::string query("DELETE FROM policy WHERE policy_id = :policy_id");
    query.replace(query.find(":policy_id"), std::string(":policy_id").length(), std::to_string(policyId_));
    connect_->sendQuery(query);

    accept();
    close();
}

void PolicyConfig::on_cancelButton_clicked()
{
    close();
}
