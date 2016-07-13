#include "reader.h"

Reader::Reader(QObject *parent) : QObject(parent)
{

}

Reader::~Reader()
{

}

void Reader::setDataBse(Storage & dataBase)
{
    _dataBase = &dataBase;
}

void Reader::readTable(QStringList file)
{

}
