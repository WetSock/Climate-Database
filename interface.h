#ifndef INTERFACE_H
#define INTERFACE_H

#include <QWidget>

#include "storage.h"
#include "iolog.h"
#include "tablereader.h"

#include <QFileDialog>
#include <QUrl>
#include <QtWidgets>

class Interface : public QWidget
{
    Q_OBJECT

    //элементы интерфейса
    QGridLayout         _lay;           //базовый слой
    QPushButton         _menuButton;    //кнопка открывающая меню
    QMenu               _menu;          //меню действий
    QComboBox           _tableList;     //комбобокс для выбора таблиц
    QTableWidget        _table;         //визуализация таблицы
    QLabel              _infoString;    //таекстовое поле отображающее некоторую информаю
    QLabel              _stateString;   //состояние подключения к БД

    //вспомогательные элементы
    QList<QAction *>    _menuActions; //хранит действия меню //освобождать память дерективой delete НЕ нужно

    QMessageBox         _messageBox;
    IOLog               _iolog;
    DataConnection      _dataConnection;
    Storage             _storage;// = Storage(_iolog, _table, _dataConnection, parent); //внесение объекта работы с файловой системой и виджета для вывода таблицы


public:
    Interface(QWidget *parent = 0);
    ~Interface();


public slots:
    //пункты меню
    void createDBFile();
    void openDBFile();
    void addDataFiles();
    void saveAsCSV();

    void selectComboBox(QString nameTable);

    //изменение информационных строк
    void setInfoString(QString &);
    void setStateString(QString );


    void setMessageBox(const Error error);



signals:


};

#endif // INTERFACE_H
