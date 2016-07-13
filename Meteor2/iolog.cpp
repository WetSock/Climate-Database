#include "iolog.h"
#include <QDebug>


IOLog::IOLog(QObject *parent) : QObject(parent)
{
    _errorView.hide();
    QFile configuration;
    configuration.setFileName(QCoreApplication::applicationDirPath() + "/configs.configurate"); //статический путь до файла конфигурации
    QStringList conf_strings = readFile(configuration);
    if(conf_strings.length() >= 2){ //spec, log, convertToTxt, convertToCSV
        _specification.setFileName(conf_strings.at(0));
        readSpecificate();

//        foreach(TableSpecificate table, tablesSpecificate){
//            qDebug() << table.originName << table.transliterationName;
//            foreach(ColumnSpecificate column, table.columns){
//                qDebug() << column.viewName << column.transliterateName << column.dataType << column.isPrimaryKey;
//            }
//            qDebug() << endl;
//        }


        _log.setFileName(conf_strings.at(1));
    }

}



void IOLog::showErrorView(QString data)
{

    _errorView.setText(data);
    _errorView.show();
    _errorView.exec();
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
        showErrorView("Файл не был найден: " + file.fileName());
    }
    file.close();
    return file_strings;
}

void IOLog::readSpecificate()
{
    if(!_specification.open(QIODevice::ReadOnly | QIODevice::Text)){
        showErrorView("Файл не открыт");
        return ;
    }

    QXmlStreamReader xml(&_specification);
    if(xml.atEnd() || xml.hasError()){ //если xml не валиден
        showErrorView("xml не валиден");
        return ;
    }

    QXmlStreamReader::TokenType token = xml.readNext();
    if(token != QXmlStreamReader::StartDocument || xml.atEnd() || xml.hasError()){ //если xml не в начале
        showErrorView("xml Не в начале");
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
                if(xml.name() == "primaryKey" && tablesSpecificate.last().columns.last().isPrimaryKey.isEmpty() && length_columns == tablesSpecificate.last().columns.length()){
                    token = xml.readNext();
                    if(token != QXmlStreamReader::EndElement)
                        tablesSpecificate.last().columns.last().isPrimaryKey = xml.text().toString();
                    continue;
                }
            }
        }
    }
    _specification.close();
}

