#ifndef IOLOG_H
#define IOLOG_H

#include <QObject>
#include <QFile>
#include <QMessageBox>
#include <QtXml>
#include <QPair>

struct ColumnSpecificate{
    QString viewName;
    QString transliterateName;
    QString dataType;
    //QString isPrimaryKey;
    bool isEmpty(){
        if(viewName == "" || transliterateName == "" || dataType == ""){ // || isPrimaryKey == ""){
            return true;
        }
        return false;
    }
};

struct TableSpecificate{
    QString originName;
    QString transliterationName;
    QList<ColumnSpecificate> columns;
    bool isEmpty(){
        if(originName == "" || transliterationName == "" || columns.length() <=0){
            return true;
        }
        return false;
    }
};

struct Error{
    QString errorClass;
    QString errorMethod;
    QString text;
    Error(QString errClass, QString errMethod, QString errText){
        errorClass = errClass;
        errorMethod = errMethod;
        text = errText;
    }
};

enum Message{MSG_CONNECTED = 0b100, MSG_LOG = 0b010, MSG_QDEBUG = 0b001, MSG_CONNECTED_QDEBUG = 0b101, MSG_CONNECTED_LOG = 0b110, MSG_LOG_QDEBUG = 0b011, MSG_ALL = 0b111};


class IOLog : public QObject
{
    Q_OBJECT

    //QMessageBox _errorView; //для вывода текста некоторых ошибок на экран (нужно законнектить с экземляром класса)

    QFile _log;
    QFile _specification;

    QVector<TableSpecificate> tablesSpecificate;
    Error * _error; //не факт что ошибка будет именно та что нужна

    void readSpecificate();                                                 //парсер для чтения спецификации из xml

    QList<QPair<QString,int>> unidentifiedTables;

public:
    explicit IOLog(QObject *parent = 0);
    ~IOLog();

    bool start();                                                           //вынесенный код из конструктора для возможности получения ошибок чтения файла конфигурации
    bool isExistTable(QString originalNameTable);                           //проверка на существование таблицы //memory: int+int
    TableSpecificate getTable(QString anyNameTable);                        //возвращает таблицу по ее имени, если талица не найдена, то вернется пустой тип
    void writeLog(Error error);
    Error lastError() const;

    void writeSpec(TableSpecificate table); //здесь уже сгенеренные имена подаются
    void writeSpec(QString originNameTable, QList<QPair<QString,QString>> columns); // first - viewNameColumn, second - dataType

    QStringList readFile(QFile & file);//перегруженный метод
    QStringList readFile(QUrl url);
    QStringList readFile(QString url);

    QString transliteration(QString);



    QList<QPair<QString, int> > getUnidentifiedTables() const;
    void addUnidentifiedTables(QString, int);
    bool clearUnidentifiedTables();

signals:
    void errorOut(const Error error); //для получения ошибки во вне необходимо законнектиться к этому сигналу


public slots:
    void errorProgram(Error error, Message message); //метод для обработки ошибок



};

#endif // IOLOG_H
