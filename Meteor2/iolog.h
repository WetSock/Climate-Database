#ifndef IOLOG_H
#define IOLOG_H

#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QtXml>




struct ColumnSpecificate{
    QString viewName;
    QString transliterateName;
    QString dataType;
    QString isPrimaryKey;
};

struct TableSpecificate{
    QString originName;
    QString transliterationName;
    QList<ColumnSpecificate> columns;
};



class IOLog : public QObject
{
    Q_OBJECT

    QMessageBox _errorView; //для вывода текста некоторых ошибок на экран (нужно законнектить с экземляром класса)

    QFile _log;
    QFile _specification;

    QList<TableSpecificate> tablesSpecificate;


public:
    explicit IOLog(QObject *parent = 0);
    bool isExistTable(QString nameTable);
    void readSpecificate();


    //void openFileConfiguration(); //открывает конфигурацию и все что в нем хранится подгружается

signals:


public slots:
    //активация окна сообщений
    void showErrorView(QString data);
    QStringList readFile(QFile & file);


};

#endif // IOLOG_H
