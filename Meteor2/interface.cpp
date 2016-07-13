#include "interface.h"
#include <QStyle>
#include <QDebug>

Interface::Interface(QWidget *parent)
    : QWidget(parent)
{
    _lay.addWidget(&_menuButton, 0,0,1,1);
    _lay.addWidget(&_stateString, 0,1,1,1); _stateString.setText("нет подключения к БД");
    _lay.addWidget(&_tableList, 0,2,1,2);
    _lay.addWidget(&_table, 1,0,1,4);
    _lay.addWidget(&_infoString,2,0,1,1); _infoString.setText("-");
    setLayout(&_lay);

    //добавление меню и его пунктов
    _menuButton.setMenu(&_menu);
    _menuActions.push_back(_menu.addAction("Создать новую БД", this, SLOT(createDBFile())));
    _menuActions.push_back(_menu.addAction("Открыть существующую БД", this, SLOT(openDBFile())));
    _menuActions.push_back(_menu.addAction("Добавить новые данные", this, SLOT(addDataFiles())));
    _menuActions.push_back(_menu.addAction("Сохранить как...", this, SLOT(saveAsCSV())));


    //блокировка по умолчанию некоторых элементов меню (не кликабельные)
    _menuActions.at(2)->setEnabled(false);
    _menuActions.at(3)->setEnabled(false);

    _menuButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _menuButton.setMinimumSize(100, 30);

    _tableList.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _tableList.setMinimumHeight(30);
    _tableList.setMaximumWidth(300);
    _tableList.setMinimumWidth(300);
    _tableList.setLayoutDirection(Qt::RightToLeft);


}

Interface::~Interface()
{

}

void Interface::createDBFile()
{

}

void Interface::openDBFile()
{

}

void Interface::addDataFiles()
{

}

void Interface::saveAsCSV()
{

}

void Interface::setInfoString(QString data)
{
    _infoString.setText(data);
}

void Interface::setStateString(QString data)
{
    _stateString.setText(data);
}


