#include "iolog.h"
#include <QDebug>


IOLog::IOLog(QObject *parent) : QObject(parent)
{  
}

IOLog::~IOLog()
{
    _log.close();
}


bool IOLog::start()
{
    QStringList conf_strings = readFile(QCoreApplication::applicationDirPath() + "/configs.configurate"); //получение строк файла конфигурации
    for(int iterator_conf = 0, length_iterator = conf_strings.length(); iterator_conf < length_iterator; iterator_conf++){
        QString url;
        url.toUtf8();
        if(QUrl(conf_strings.at(iterator_conf)).isRelative()){
            //если путь относительный - делаем его абсолютным
            url = QCoreApplication::applicationDirPath() + conf_strings.at(iterator_conf);
        }
        //запись путей в соответствующие файлы
        switch(iterator_conf){
        case 0:  _specification.setFileName(url); break;
        case 1:  _log.setFileName(url); break;
        default: qDebug() << "В файле конфигурации есть не декларированные строки";
        }
    }

    if(!_specification.exists()){
        qDebug() << "Файл спецификации не существует по заданному пути в конфигурации";
        return false;
    }

    if(!_log.exists() || !_log.open(QIODevice::Append | QIODevice::Text)){
        errorProgram(Error("IOLog","start","лог файл не может быть открыт"), MSG_CONNECTED_QDEBUG);
    }

    readSpecificate();
    return true;
}

bool IOLog::isExistTable(QString originalNameTable)
{
    for(int iteratorTable = 0, maxLengthTable = tablesSpecificate.length(); iteratorTable < maxLengthTable; iteratorTable++){
        if(tablesSpecificate[iteratorTable].originName == originalNameTable){
            return true;
        }
    }
    return false;
}

TableSpecificate IOLog::getTable(QString anyNameTable)
{
    for(int iteratorTable = 0, maxLengthTable = tablesSpecificate.length(); iteratorTable < maxLengthTable; iteratorTable++){
        if(tablesSpecificate[iteratorTable].originName == anyNameTable || tablesSpecificate[iteratorTable].transliterationName == anyNameTable){
            return tablesSpecificate[iteratorTable];
        }
    }
    return TableSpecificate();
}

void IOLog::writeLog(Error error)
{
    if(!_log.isOpen()){
        errorProgram(Error("IOLog","writeLog","Вывод в лог не возможен: файл закрыт"), MSG_QDEBUG);
        return;
    }
    static QTextStream out(&_log);
    out << endl << "Error:   " << error.errorClass << "::" << error.errorMethod << endl << error.text << endl;
}

Error IOLog::lastError() const
{
    return *_error;
}

void IOLog::writeSpec(TableSpecificate table)
{
    if(table.isEmpty()){
        qDebug() << "пустая обертка";
        return ;
    }

    tablesSpecificate.push_back(TableSpecificate());
    for(int iterator_table = 0, length_table = table.columns.length(); iterator_table <length_table; iterator_table++){
        tablesSpecificate.last().columns.push_back(ColumnSpecificate());
        tablesSpecificate.last().columns.last().viewName =              table.columns.at(iterator_table).viewName;
        tablesSpecificate.last().columns.last().transliterateName =     table.columns.at(iterator_table).transliterateName;
        tablesSpecificate.last().columns.last().dataType =              table.columns.at(iterator_table).dataType;
    }
    tablesSpecificate.last().originName = table.originName;
    tablesSpecificate.last().transliterationName = table.transliterationName;


    if(_specification.exists() &&_specification.open(QIODevice::WriteOnly | QIODevice::Text)){
        //file.seek(0);
        QTextStream out(&_specification);
        out.setCodec("UTF-8");
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << endl << "<tables>" << endl;
        for(int iterator_tables = 0, length_tables = tablesSpecificate.length(); iterator_tables < length_tables; iterator_tables++){
            out << "<table>" <<endl
                << "<tableNameOrigin>" << tablesSpecificate.at(iterator_tables).originName << "</tableNameOrigin>" << endl
                << "<tableNameTransliterate>" << tablesSpecificate.at(iterator_tables).transliterationName << "</tableNameTransliterate>" << endl
                << "<columns>" << endl;
            for(int iterator_columns = 0, length_columns = tablesSpecificate.at(iterator_tables).columns.length(); iterator_columns < length_columns; iterator_columns++){
                out << "<column>" << endl
                    << "<columnNameView>" << tablesSpecificate.at(iterator_tables).columns.at(iterator_columns).viewName << "</columnNameView>" << endl
                    << "<columnNameTransliterate>" << tablesSpecificate.at(iterator_tables).columns.at(iterator_columns).transliterateName << "</columnNameTransliterate>" << endl
                    << "<dataType>" << tablesSpecificate.at(iterator_tables).columns.at(iterator_columns).dataType << "</dataType>" << endl
                    << "</column>" << endl;
            }
            out << "</columns>" << endl
                << "</table>" << endl;
        }
        out << "</tables>";
        _specification.close();
    }
    else{
        qDebug() << "файл специфкации не может быть модифицирован: файл закрыт. Все изменения в силе только для текущей сессии, после выхода из программы внесенные данные будут удалены";
    }


}

void IOLog::writeSpec(QString originNameTable, QList<QPair<QString, QString> > columns)
{
    TableSpecificate table;
    table.originName = originNameTable;
    table.transliterationName = transliteration(originNameTable);

    for(int iterator_columns = 0, length_columns = columns.length(); iterator_columns < length_columns; iterator_columns++){
        table.columns.push_back(ColumnSpecificate());
        table.columns.last().viewName =             columns.at(iterator_columns).first;
        table.columns.last().transliterateName =    "column" + QString::number(iterator_columns);//transliteration(columns.at(iterator_columns).first) + QString;
        table.columns.last().dataType =             columns.at(iterator_columns).second;
    }
    qDebug() << "передача в обертку завершена" << table.columns.length();
    return writeSpec(table);
}

QStringList IOLog::readFile(QFile & file)
{
    QStringList file_strings;
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        file.seek(0);
        QTextStream in(&file);
        while(!in.atEnd()){
            file_strings << in.readLine();
        }
    }
    else{
        errorProgram(Error("IOLog", "readFile", "Файл не был найден: " + file.fileName()), MSG_CONNECTED_QDEBUG);
    }
    file.close();
    return file_strings;
}

QStringList IOLog::readFile(QUrl url)
{
    QFile file(url.toLocalFile());
    return readFile(file);
}

QStringList IOLog::readFile(QString url)
{
    QFile file(url);
    return readFile(file);
}

QString IOLog::transliteration(QString text)
{
    QStringList nw_alphabet;
    QString result = "";
    QString rusLower = "абвгдеёжзийклмнопрстуфхцчшщыэюя -";
    QString rusUpper = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЫЭЮЯ -";
    QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_";
    nw_alphabet << "a" << "b" << "v" << "g" << "d" << "e" << "e" << "j" << "z" << "i" << "i" << "k"
                << "l" << "m" << "n" << "o" << "p" << "r" << "s" << "t" << "y" << "f" << "h" << "c"
                << "ch" << "sh" << "sch" << "i" << "e" << "u" << "ya" << "_" << "_" ;
    int count = text.length();
    for(int i =0; i<count; i++){
        QChar ch = text.at(i);
        if(validChars.indexOf(ch) != -1)
            result += ch;
        else if(rusLower.indexOf(ch) != -1)
            result += nw_alphabet.at(rusLower.indexOf(ch));
        else if(rusUpper.indexOf(ch) != -1)
            result += nw_alphabet.at(rusUpper.indexOf(ch));
    }
    return result.toLower();
}

void IOLog::readSpecificate()
{
    if(!_specification.open(QIODevice::ReadOnly | QIODevice::Text)){
        errorProgram(Error("IOLog", "readSpecificate", "файл не открыт"), MSG_QDEBUG);
        return ;
    }

    QXmlStreamReader xml(&_specification);
    if(xml.atEnd() || xml.hasError()){ //если xml не валиден
        errorProgram(Error("IOLog", "readSpecificate", "xml Не валиден"), MSG_QDEBUG);
        return ;
    }

    QXmlStreamReader::TokenType token = xml.readNext();
    if(token != QXmlStreamReader::StartDocument || xml.atEnd() || xml.hasError()){ //если xml не в начале
        errorProgram(Error("IOLog", "readSpecificate", "не найден заголовок в xml"), MSG_QDEBUG);
        return ;
    }


    int length_tables = 0;
    for(token = xml.readNext(); !xml.atEnd() || !xml.hasError(); token = xml.readNext()){

        if(xml.name() == ""){
            continue;
        }

        if(xml.name() == "table" && token != QXmlStreamReader::EndElement){
            tablesSpecificate.push_back(TableSpecificate());
            length_tables++;
            continue;
        }

        //чтение оригинального имени
        if(xml.name() == "tableNameOrigin" && tablesSpecificate.last().originName.isEmpty() && length_tables == tablesSpecificate.length()){
            token = xml.readNext();
            if(!xml.atEnd() || !xml.hasError()){
                tablesSpecificate.last().originName = xml.text().toString();
            }
            continue;
        }


        //чтение переведенного имени
        if(xml.name() == "tableNameTransliterate" && tablesSpecificate.last().transliterationName.isEmpty() && length_tables == tablesSpecificate.length()){
            token = xml.readNext();
            if(token != QXmlStreamReader::EndElement)
                tablesSpecificate.last().transliterationName = xml.text().toString();
            continue;
        }

        //вход в цикл колонок
        if(xml.name() == "columns" && length_tables == tablesSpecificate.length()){
            int length_columns = 0;

            for(token = xml.readNext(); (!xml.atEnd() || !xml.hasError()) && xml.name() != "columns"; token = xml.readNext()){
                if(xml.name() == ""){
                    continue;
                }

                if(xml.name() == "column" && token != QXmlStreamReader::EndElement){
                    tablesSpecificate.last().columns.push_back(ColumnSpecificate());
//костыль {
//                    tablesSpecificate.last().columns.last().isPrimaryKey = "yes";
//костыль }
                    length_columns++;
                    continue;
                }

                //чтение визуального имени
                if(xml.name() == "columnNameView" && tablesSpecificate.last().columns.last().viewName.isEmpty() && length_columns == tablesSpecificate.last().columns.length()){
                    token = xml.readNext();
                    if(token != QXmlStreamReader::EndElement)
                        tablesSpecificate.last().columns.last().viewName = xml.text().toString();
                    continue;
                }

                //чтение переведенного имени
                if(xml.name() == "columnNameTransliterate" && tablesSpecificate.last().columns.last().transliterateName.isEmpty() && length_columns == tablesSpecificate.last().columns.length()){
                    token = xml.readNext();
                    if(token != QXmlStreamReader::EndElement)
                        tablesSpecificate.last().columns.last().transliterateName = xml.text().toString();
                    continue;
                }

                //чтение типа данных
                if(xml.name() == "dataType" && tablesSpecificate.last().columns.last().dataType.isEmpty() && length_columns == tablesSpecificate.last().columns.length()){
                    token = xml.readNext();
                    if(token != QXmlStreamReader::EndElement)
                        tablesSpecificate.last().columns.last().dataType = xml.text().toString();
                    continue;
                }

                //чтение определения ключа
//                if(xml.name() == "primaryKey" && tablesSpecificate.last().columns.last().isPrimaryKey.isEmpty() && length_columns == tablesSpecificate.last().columns.length()){
//                    token = xml.readNext();
//                    if(token != QXmlStreamReader::EndElement)
//                        tablesSpecificate.last().columns.last().isPrimaryKey = xml.text().toString();
//                    continue;
//                }
            }
        }
    }
    _specification.close();
}

QList<QPair<QString, int> > IOLog::getUnidentifiedTables() const
{
    return unidentifiedTables;
}

void IOLog::addUnidentifiedTables(QString OriginNameTable, int count_columns)
{
    for(int iterator_unid = 0, length_unid = unidentifiedTables.length(); iterator_unid < length_unid; iterator_unid++){
        if(unidentifiedTables.at(iterator_unid).first == OriginNameTable){
            return;
        }
    }
    unidentifiedTables.append(QPair<QString, int>(OriginNameTable, count_columns));
}

bool IOLog::clearUnidentifiedTables()
{
    unidentifiedTables.clear();
    return true;
}

void IOLog::errorProgram(Error error, Message message)
{
    delete _error;
    _error = new Error(error.errorClass, error.errorMethod, error.text);

    if(message & 0b001){
        //вывод в qdebug
        qDebug() << error.errorClass + "::" + error.errorMethod + "-> " + error.text;
    }

    if(message & 0b010){
        //вывод в лог
        writeLog(error);
        qDebug() << "вызван writeLog";
    }

    if(message & 0b100){
        //вывод в подключенные объекты
        emit errorOut(error);
    }




}

