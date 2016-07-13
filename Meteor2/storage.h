#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>

#include <QtSql>
#include <QTableView>

class Storage : public QObject
{
    Q_OBJECT

    QSqlDatabase _dataBase = QSqlDatabase::addDatabase("QSQLITE");
    QSqlDatabase _specFile = QSqlDatabase::addDatabase("QSQLITE");

    QUrl _pathDB;
    QUrl _pathSF;
    QString _user;
    QString _host;
    int _port;
    QString _pass;

    bool isExistentTableIntoDB(QString nameTable); //+
    bool createTableIntoDB(QString nameTable); //+
    bool connectDB(); //+
    bool disconnectDB(); //+
    bool connectSF(); //+
    bool disconnectSF(); //+
    QString transliteration(QString); //+
    QString quotes(QString data, QString type); //+
public:
    explicit Storage(QObject *parent = 0);
    ~Storage();


    bool setDataConnectionSF(QUrl path); //+
    bool writeToSF(QString nameTable, QString nameColumn, QString dataType, QString PK); //+
    QStringList getFormatNamesColumnsSF(QString formatNameTable); //для получения данных в writeToDB //+
    QStringList getInfoColumnSF(QString formatNameColumn, QString formatNameTable); //для получения данных в writeToDB //+

    bool setDataConnectionDB(QUrl path, QString user = "admin", QString host = "localhost", int port = 0, QString pass = ""); //+
    bool outDB(QTableView & table, QString nameTable = ""); //+


    //Для Reader

    //index - указывает номер столбца (несчитая ключевые) с которого начнется заполнение
    //имеющейся записи, которая существует (это будет поределено по ключам)
    //если index = -1(по умолчанию), то будет создана новая запись, а не изменена существующая
    bool writeToDB(QString nameTable, QStringList values, int index = -1); //+
    bool isExistentTableIntoSF(QString nameTable); //существование таблицы в спецификации //+
    QStringList primaryKeys(QString formatNameTable); //количество столбцов, что являются ключами




signals:

public slots:
};

#endif // STORAGE_H
