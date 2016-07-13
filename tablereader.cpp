#include "tablereader.h"
#include <QRegularExpression>
#include <QDebug>


bool TableReader::SetData(QList<QString> _data)
{
    datafile=_data;
    return true;
}

TableData TableReader::GetDataFromTable(TableData table)
{
    if (table.ranges.length()>0)
    {
        QRegularExpression regExpForTable;
        int i=0,j=0;

        QString pattern="";
        while (i<table.ranges.length()-1) // -1 потому что в последней колонке бывает на символ меньше
        {
            pattern = pattern + "(.{" + QString::number((table.ranges.at(i++))-1) + "})( |$)";
        }

        pattern = pattern + "(.{" + QString::number((table.ranges.at(i++))-1) + "}.{0,2})( |$)";

        regExpForTable.setPattern(pattern);
        QString text;

        if (table.headers==1)
        {
            QString varFor9Table;// Переменная, которая нужна для записи в 9 таблицу названия пункта

            i=0; // Итератор для считывания всех строк таблицы
            while (i<table.tableStrings.length())
            {
                text=table.tableStrings.at(i);

                // Для начала убираем из строки слова про посторонние провода и переходы
                {
                    QRegularExpression postProvod("(.+)(Пост.провод)(.+)");
                    QRegularExpression perehod("(.+)(Переход.+месяца?)(.+)");
                    QRegularExpressionMatch postProvodMatch=postProvod.match(text);
                    QRegularExpressionMatch perehodMatch=perehod.match(text);
                    if (postProvodMatch.hasMatch())
                    {
                        text = postProvodMatch.captured(1);
                        for (int i=0;i<postProvodMatch.capturedLength(2);i++)
                        {
                            text+= " ";
                        }
                        text += postProvodMatch.captured(3);
                    }
                    if (perehodMatch.hasMatch())
                    {
                        text = perehodMatch.captured(1);
                        for (int i=0;i<perehodMatch.capturedLength(2);i++)
                        {
                            text+= " ";
                        }
                        text += perehodMatch.captured(3);
                    }
                }

                // Извлечение данных из строки
                QRegularExpressionMatch match=regExpForTable.match(text);
                QRegularExpression onlySpaces("^ +$");
                QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

                if (match.hasMatch()) // Для строки с данными соответсвующими колонкам
                {
                    QList<QString> dataFromString;

                    // Отдельная обработка 9 таблицы, в которую криво занесли данные (как так можно?)
                    if (table.tableNumber==9)
                    {
                        dataFromString << varFor9Table;
                    }

                    int k=1; // Итератор для данных в строке
                    while (k<match.lastCapturedIndex()+1)
                    {
                        if (onlySpaces.match(match.captured(k)).hasMatch())  // Если данных нет, записать _
                        {
                            dataFromString << "_";
                        }
                        else // Иначе записать данные без пробелов по бокам
                        {
                            dataFromString << withoutSpaces.match(match.captured(k)).captured(2);
                        }
                        k+=2;
                    }
                    table.dividedData << dataFromString; // Запись данных из строки в структуру
                }
                else // Для строк, в которых "плохие" данные
                {
                    if (table.tableNumber==9)
                    {
                        //qDebug() << "" << text << endl;
                        QRegularExpression stationName("[0-9]{1,3}.[ а-яА-Яa-zA-Z0-9" + minus + "]+");
                        if (stationName.match(text).hasMatch())
                        {
                            varFor9Table=stationName.match(text).captured();
                            qDebug() << stationName.match(text).captured();
                        }
                        else table.wrongStrings << text;
                    }
                    else
                        table.wrongStrings << text;
                }
                i++;
            }
        }
        else // Если в строке сразу несколько таблиц
        {
            while (j<table.headers)  // Работа поочередно с каждым набором данных в таблице
            {
                pattern=".{"+ QString::number(table.headerPosition.at(j++)) +"}(.{" + QString::number(table.headerLength) +"})";
                QRegularExpression oneTablePerString(pattern);
                i=0;
                QString varFor9Table;// Переменная, которая нужна для записи в 9 таблицу названия пункта
                while (i<table.tableStrings.length())
                {
                    text=oneTablePerString.match(table.tableStrings.at(i++)).captured(1); // Берем из строки только нужную таблицу

                    // Для начала убираем из строки слова про посторонние провода и переходы
                    {
                        QRegularExpression postProvod("(.+)(Пост.провод)(.+)");
                        QRegularExpression perehod("(.+)(Переход.+месяца?)(.+)");
                        QRegularExpressionMatch postProvodMatch=postProvod.match(text);
                        QRegularExpressionMatch perehodMatch=perehod.match(text);
                        if (postProvodMatch.hasMatch())
                        {
                            text = postProvodMatch.captured(1);
                            for (int i=0;i<postProvodMatch.capturedLength(2);i++)
                            {
                                text+= " ";
                            }
                            text += postProvodMatch.captured(3);
                        }
                        if (perehodMatch.hasMatch())
                        {
                            text = perehodMatch.captured(1);
                            for (int i=0;i<perehodMatch.capturedLength(2);i++)
                            {
                                text+= " ";
                            }
                            text += perehodMatch.captured(3);
                        }
                    }

                    // Извлечение данных из строки
                    QRegularExpressionMatch match=regExpForTable.match(text);
                    QRegularExpression onlySpaces("^ +$");
                    QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

                    if (match.hasMatch()) // Для строки с данными соответсвующими колонкам
                    {
                        QList<QString> dataFromString;

                        // Отдельная обработка 9 таблицы, в которую криво занесли данные (как так можно?)
                        if (table.tableNumber==9)
                        {
                            dataFromString << varFor9Table;
                        }

                        int k=1; // Итератор для данных в строке
                        while (k<match.lastCapturedIndex()+1)
                        {
                            if (onlySpaces.match(match.captured(k)).hasMatch())  // Если данных нет, записать _
                            {
                                dataFromString << "_";
                            }
                            else // Иначе записать данные без пробелов по бокам
                            {
                                dataFromString << withoutSpaces.match(match.captured(k)).captured(2);
                            }
                            k+=2;
                        }
                        table.dividedData << dataFromString; // Запись данных из строки в структуру
                    }
                    else // Для строк, в которых "плохие" данные
                    {
                        if (table.tableNumber==9)
                        {
                            //qDebug() << "" << text << endl;
                            QRegularExpression stationName("[0-9]{1,3}.[ а-яА-Яa-zA-Z0-9" + minus + "]+");
                            if (stationName.match(text).hasMatch())
                            {
                                varFor9Table=stationName.match(text).captured();
                            }
                            else table.wrongStrings << text;
                        }
                        else
                            table.wrongStrings << text;
                    }

                }
            }
        }
        return table;
    }
    else
    {
        qDebug() << "Invalid ranges value. Class dataFromTables";
    }
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
    QRegularExpression NOTAFUCKINGTABLE("Таблица *([0-9]+). *([0-9а-яА-Я " + minus + ",.()]+)[.]{2,}"); // Как же я устал...
    QRegularExpression tableSearch("Таблица *([0-9]+). *([0-9а-яА-Я " + minus + ",.()]+)");
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
        QRegularExpression notEmptyString("[^ ]");
        while (index<datafile.length()){ // Ищем начало заголовка (по сути, мы уже должны быть на нем, но мало ли...)
            text= datafile.at(index++);
            pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "* *$";
            QRegularExpression oneSymbolString(pattern);

            if (!oneSymbolString.isValid())
            {
                qDebug() << "First header oneSymbol";
            }

            if (oneSymbolString.match(text).hasMatch() || // Если строка состоит из одинаковых символов
                    !notEmptyString.match(text).hasMatch())   // Или пустая
            {
                break;
            }
            pattern="( *" + QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,}( |$)){2,}";
            QRegularExpression fewHeaders(pattern);
            if (!fewHeaders.isValid())
            {
                qDebug() << "First header fewHeaders";
            }
            pattern="( *)(" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,})";
            QRegularExpression countHeaders(pattern);
            if (!countHeaders.isValid())
            {
                qDebug() << "First header countHeaders";
            }
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
            pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "* *$";
            QRegularExpression oneSymbolString(pattern);
            if (!oneSymbolString.isValid())
            {
                qDebug() << "Error in Second header oneSymbolString. Table " << nextTable.tableName << ". Index " << index << endl << text << endl;
            }

            if (oneSymbolString.match(text).hasMatch() || !notEmptyString.match(text).hasMatch()) // Если строка состоит из одинаковых символов
            {
                break;
            }
            pattern="( *" + QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,}( |$)){2,}";
            QRegularExpression fewHeaders(pattern);
            if (!fewHeaders.isValid())
            {
                qDebug() << "Second header fewHeaders";
            }
            pattern="( *)(" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "{2,})";
            QRegularExpression countHeaders(pattern);
            if (!countHeaders.isValid())
            {
                qDebug() << "Second header countHeaders";
            }
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
            if (!findHeader.isValid())
            {
                qDebug() << "DataReading findHeader";
            }
            QRegularExpressionMatch match=findHeader.match(datafile.at(index-2));
            nextTable.ranges=GetRanges(match.captured(1));
        }

        while (index<datafile.length()){ // Наконец-то считываем строки с данными
            text= datafile.at(index++);
            match=notEmptyString.match(text);
            if (!match.hasMatch())  // Если это последняя строка таблицы
            {
                return GetDataFromTable(nextTable); // Возвращаем готовую таблицу
            }
            else
            {
                nextTable.tableStrings << text;
            }
        }

        return GetDataFromTable(nextTable);
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

bool TableReader::ShowDividedTable(QList<QList<QString> > dividedData)
{
    QTextStream out(stdout);
    int i=0,j=0;
    while (i<dividedData.length()) // Вывод таблицы на экран
    {
        while (j< dividedData.at(i).length() )
        {
            out << dividedData.at(i).at(j) << " ";
            j++;
        }
        out << endl;
        j=0;
        i++;
    }
    return true;
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
