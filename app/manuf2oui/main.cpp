#include <QCoreApplication>
#include <QtSql>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSqlDatabase ouiDB = QSqlDatabase::addDatabase("QSQLITE", "oui.db");
    ouiDB.setDatabaseName("oui.db");
    if(!ouiDB.open()) {
        qWarning() << QString("ouiDB.open() return false %1").arg(ouiDB.lastError().text());
        ouiDB.close();
        return -1;
    }

    QSqlQuery ouiQuery(ouiDB);

    QMutex m;

    {
        QMutexLocker ml(&m);
        ouiQuery.exec("SELECT name FROM sqlite_master WHERE name = 'oui'");
    }

    ouiQuery.next();
    QString result = ouiQuery.value(0).toString();
    if(result.compare("oui")) {
        qDebug() << QString("Create oui table");

        {
            QMutexLocker ml(&m);
            ouiQuery.exec("CREATE TABLE oui (mac CHAR(20) NOT NULL PRIMARY KEY, organ VARCHAR(64) NOT NULL)");
        }

        QFile file("manuf_rep.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << QString("Not open oui file");
            // error message here
            return -1;
        }

        QString getLine;
        QTextStream fileStream(&file);
        while (!fileStream.atEnd()) {
            getLine = fileStream.readLine();
            QStringList ouiResult = getLine.split('\t');

            {
                QMutexLocker ml(&m);
                ouiQuery.prepare("INSERT INTO oui VALUES(:mac, :organ)");
                ouiQuery.bindValue(":mac", ouiResult.at(0));
                ouiQuery.bindValue(":organ", ouiResult.at(1));
                ouiQuery.exec();
            }
        }
    }
}
