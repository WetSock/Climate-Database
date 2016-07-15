#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>

#include <QtSql>
#include <QTableWidget>
#include "iolog.h"



struct DataConnection{
    QUrl path;
    QString user =      "admin";
    QString host =      "admin";
    int port =          -1;
    QString pass =      "";
    bool isEmpty(){
        if(path.isEmpty() || user == "" || host == ""){
            return true;
        }
        return false;
    }
};

class Storage : public QObject
{
    Q_OBJECT

    QSqlDatabase _dataBase = QSqlDatabase::addDatabase("QSQLITE");

    IOLog * _iolog; //освобождать память нет необходимоси
    QTableWidget * _table;
    DataConnection * _dataConnection;

public:
    explicit Storage(QObject *parent = 0);
    ~Storage();

    void setInterationItems(IOLog & iolog, QTableWidget & table, DataConnection & dataConnection);

    bool connectStorage(); //обязательно открыть БД для записи перед ней //+
    bool disconnectStorage(); //обязательно закрыть БД после окончания записи //+

    bool readData(QString nameTable);
    bool writeData(QString nameTable, QList<QList<QString>> values);
    //bool updateData(QString nameTable);

    QStringList namesTables();
    QString transliteration(QString); //целесообразность данного метода необходимо пересмотреть

private:
    bool isExistTable(QString nameTable); //проверка по любому из имен на существование таблицы в БД //+
    bool createTable(QString nameTable); //+


    QString quotes(QString data, QString type);






signals:

public slots:
};

#endif // STORAGE_H
