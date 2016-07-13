#ifndef READER_H
#define READER_H

#include <QObject>


#include "storage.h"
#include <QRegularExpression>

class Reader : public QObject
{
    Q_OBJECT
    Storage * _dataBase; //работай исключительно с этой бд
public:
    explicit Reader(QObject *parent = 0);
    ~Reader();

    void setDataBse(Storage & dataBase); //вызвана будет в Interface

    void readTable(QStringList file);


signals:

public slots:
};

#endif // READER_H
