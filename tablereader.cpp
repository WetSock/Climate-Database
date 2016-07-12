#include "tablereader.h"
#include <QRegularExpression>
#include <QDebug>


bool TableReader::SetData(QList<QString> _data)
{
    datafile=_data;
    return true;
}

QList<QString> TableReader::GetTableNames()
{
    QString text;
    QRegularExpressionMatch match;
    QRegularExpression tableNameRead("Таблица[ 0-9.]+([0-9а-яА-Я " + minus + ",]+)");
    QList<QString> names;

    while (index<datafile.length()){ // Запись названий таблиц в вектор
        text = datafile.at(index);
        match = tableNameRead.match(text);
        if (match.hasMatch())
        {
            names << match.captured(1);
            index++;
        }
        else break;
    }
    return names;
}

int TableReader::ContentsSearch()
{
    QString text;
    QRegularExpressionMatch match;
    QRegularExpression contentsTableSearch("Таблица");

    while (index<datafile.length()){  // Поиск строк с названиями таблиц в содержании
        text = datafile.at(index);
        match = contentsTableSearch.match(text);
        if (match.hasMatch())
            break;
        else index++;
    }
    return index;
}

int TableReader::GetCurrentIndex()
{
    return index;
}

TableData TableReader::GetNextTable()
{
    // Записал в структуру номер и название таблицы
    TableData nextTable;

    QString text;
    QRegularExpressionMatch match;
    QRegularExpression NOTAFUCKINGTABLE("Таблица *([0-9]+). *([0-9а-яА-Я " + QRegularExpression::escape("-") + ",.()]+)[.]{2,}"); // Как же я устал...
    QRegularExpression tableSearch("Таблица *([0-9]+). *([0-9а-яА-Я " + QRegularExpression::escape("-") + ",.()]+)");
//    QRegularExpression dataString(" *[0-9]+[.] *[а-яА-Я ,.]+");

    while (index<datafile.length()){ // Поиск объявления таблицы
        text = datafile.at(index++);
        match = tableSearch.match(text);

        if (match.hasMatch() && !NOTAFUCKINGTABLE.match(text).hasMatch()) // Если текущая строка оказалась объявлением таблицы записать её название и номер
        {
            nextTable.tableNumber = (match.captured(1)).toInt();
            nextTable.tableName = match.captured(2);
            break;
        }
    }

    // Если мы всё-таки нашли таблицу
    if (nextTable.tableNumber!=0)
    {
        // Считывание месяца, года и выпуска
        text = datafile.at(index++);
        QRegularExpression year("Год ([0-9]{4})[^0-9]+([0-9]+)");
        QRegularExpression month("([0-9]*) +Год ([0-9]{4})[^0-9]+([0-9]+)");
        match=month.match(text);
        if (match.hasMatch())
        {
            nextTable.month   = match.captured(1).toInt();
            nextTable.year    = match.captured(2).toInt();
            nextTable.edition = match.captured(3).toInt();
        }
        else
        {
            nextTable.year    = year.match(text).captured(1).toInt();
            nextTable.edition = year.match(text).captured(2).toInt();
        }

        // Работа с заголовком
        QString pattern=""; // Здесь будет конструироваться регулярка для проверки на начало и конец заголовка
        QRegularExpression firstSymbol("^ *(.)"); // Регулярка для получения первого символа строки кроме пробела
        QRegularExpression notEmptyString("[^ ]+");
        while (index<datafile.length()){ // Ищем начало заголовка (по сути, мы уже должны быть на нем, но мало ли...)
            text= datafile.at(index++);
            pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "*$";
            QRegularExpression oneSymbolString(pattern);

            if (oneSymbolString.match(text).hasMatch() || // Если строка состоит из одинаковых символов
                    !notEmptyString.match(text).hasMatch())   // Или пустая
            {
                break;
            }

            pattern="( *" + QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,}( |$)){2,}";
            QRegularExpression fewHeaders(pattern);
            pattern="( *)(" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,})";
            QRegularExpression countHeaders(pattern);
            QRegularExpressionMatchIterator iterator=countHeaders.globalMatch(text);
            QRegularExpressionMatch match;
            if (fewHeaders.match(text).hasMatch())
            {
                int i=0,j=0,x=0;
                while (iterator.hasNext())
                {
                    match=iterator.next();
                    nextTable.headerLength=match.capturedLength(2);
                    x=0;

                    while (j<nextTable.headerPosition.length()) // Считаем место, занимаемое предыдущими заголовками
                    {
                        x+=nextTable.headerPosition.at(j++) + nextTable.headerLength;
                    }

                    nextTable.headerPosition << (match.capturedLength(1)+x);
                    i++;
                }
                nextTable.headers = i;
                break;
            }
        }

        while (index<datafile.length()){ // Ищем конец заголовка и что-то делаем по пути (кажется)
            text= datafile.at(index++);
            pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "*$";
            QRegularExpression oneSymbolString(pattern);

            if (oneSymbolString.match(text).hasMatch() || !notEmptyString.match(text).hasMatch()) // Если строка состоит из одинаковых символов
            {
                break;
            }
            pattern="( *" + QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,}( |$)){2,}";
            QRegularExpression fewHeaders(pattern);
            pattern="( *)(" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,})";
            QRegularExpression countHeaders(pattern);
            QRegularExpressionMatchIterator iterator=countHeaders.globalMatch(text);
            QRegularExpressionMatch match;
            if (fewHeaders.match(text).hasMatch())
            {
                int i=0,j=0,x=0;
                while (iterator.hasNext())
                {
                    match=iterator.next();
                    nextTable.headerLength=match.capturedLength(2);
                    x=0;

                    while (j<nextTable.headerPosition.length()) // Считаем место, занимаемое предыдущими заголовками
                    {
                        x+=nextTable.headerPosition.at(j++) + nextTable.headerLength;
                    }

                    nextTable.headerPosition << (match.capturedLength(1)+x);
                    i++;
                }
                nextTable.headers = i;
                break;
            }    }

        // Получение места данных в строке
        if (nextTable.headers==1)
        {
            nextTable.ranges=GetRanges(datafile.at(index-2));
        }
        else
        {
            QString pattern="^.{" + QString::number(nextTable.headerPosition.at(0)) + "}(.{" + QString::number(nextTable.headerLength) + "})";
            QRegularExpression findHeader(pattern);
            QRegularExpressionMatch match=findHeader.match(datafile.at(index-2));
            nextTable.ranges=GetRanges(match.captured(1));
        }

        while (index<datafile.length()){ // Наконец-то считываем строки с данными
            text= datafile.at(index++);
            match=notEmptyString.match(text);
            if (!match.hasMatch())  // Если это последняя строка таблицы
            {
                return nextTable; // Возвращаем готовую таблицу
            }
            else
            {
                nextTable.tableStrings << text;
            }
        }

        return nextTable;
    }
    else
    {
        qDebug() << "Can't find table. Class TableReader";
    }
}

QString TableReader::GetCurrentString()
{
    return datafile[index];
}

TableData TableReader::GetTableByName(QString input)
{
    QRegularExpression TableSearch(" Таблица[ 0-9.]+.*" + QRegularExpression::escape(input));
    QRegularExpressionMatch match;
    int j=0;
    bool flag=true;
    while (j<datafile.length() && flag)
    {
        QString text = datafile.at(j);
        match=TableSearch.match(text);
        if (match.hasMatch())
            flag = false;
        j++;
    }
    index=j-1;
    return GetNextTable();
}

QList<TableData> TableReader::GetAllTables()
{
    index=0;
    QList<TableData> allTables;

    while (index<datafile.length())
    {
        allTables << GetNextTable();
    }
    index=0;
    return allTables;
}

bool TableReader::ShowTables(QList<TableData> allTables)
{
    QTextStream out(stdout);
    int i=0,j=0;
    while (j<allTables.length())
    {
        TableData tableString=allTables.at(j++);

        i=0;
        out << " " << tableString.tableNumber << ".  " << tableString.tableName << endl << endl;
        while (i< tableString.tableStrings.length())
        {
            out << tableString.tableStrings.at(i++) << endl;
        }
        out << endl;
    }

    return true;
}

QList<int> TableReader::GetRanges(QString text)
{
    QList<int> ranges;

    QRegularExpression findRanges("(([ а-яА-Яa-zA-Z0-9.,/"+ QRegularExpression::escape("-") +"]+[^ а-яА-Яa-zA-Z0-9.,/"+ QRegularExpression::escape("-") +"])+)([ а-яА-Яa-zA-Z0-9.,/"+ QRegularExpression::escape("-") +"]+)");

    QRegularExpressionMatch match=findRanges.match(text);

    QRegularExpression findSubRanges("[ а-яА-Яa-zA-Z0-9.,/"+ QRegularExpression::escape("-") +"]+[^ а-яА-Яa-zA-Z0-9.,/"+ QRegularExpression::escape("-") +"]");
    QRegularExpressionMatchIterator iterator=findSubRanges.globalMatch(match.captured(1));


    while (iterator.hasNext())
    {
        ranges << iterator.next().capturedLength();
    }
    ranges << match.capturedLength(3);

    return ranges;
}

TableReader::TableReader()
{

}
