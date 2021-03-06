#include "tablereader.h"
#include <QRegularExpression>
#include <QDebug>
#include <QFile>


bool TableReader::SetData(QStringList _data)
{
    datafile=_data;
    return true;
}

TableData TableReader::GetDataFromTable(TableData table)
{
    if (table.ranges.length()>0)
    {
        QRegularExpression regExpForTable(GetPatternForTable(table.ranges));
        QString text,pattern;

        if (table.tableNumber==9)
            table=GetDataFrom9Table(table, regExpForTable);
        else
        for(int j=0;j<table.headers;j++){  // Работа поочередно с каждым набором данных в таблице
            for(int i=0;i<table.tableStrings.length();i++){ // Работа с каждой строкой
                if (table.headers==1) // Если таблица одна, то оставляем полную строку
                {
                    text=table.tableStrings.at(i);
                }
                else // Иначе обрезаем её
                {
                    pattern=".{"+ QString::number(table.headerPosition.at(j)) +"}(.{" + QString::number(table.headerLength) +"})";
                    QRegularExpression oneTablePerString(pattern);
                    text=oneTablePerString.match(table.tableStrings.at(i)).captured(1); // Берем из строки только нужную таблицу
                }

                text=CutProvod(text);
                text=CutStationNumber(text);

                // Тут добавляем пробелы, потому что иначе некоторые строки оказываются слишком короткими
                QRegularExpressionMatch myMatch=regExpForTable.match(text+"                                                                                                                                                                 "); // ДА ЗДРАВСТВУЮТ КОСТЫЛИ!!!                
                QRegularExpression meaninglessString("^[ _-]*$");

                if (!meaninglessString.match(text).hasMatch()) // Если сторока с чем-то полезным
                {
                    QRegularExpression onlySpaces("^ +$");
                    QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");
                    if (myMatch.hasMatch()) // Для строки с данными соответствующими колонкам
                    {
                        QList<QString> dataFromString;
                        int k=0; // Итератор для данных в строке

                        if (onlySpaces.match(myMatch.captured(1)).hasMatch()) // Если на месте названия станции ничего нет
                        {
                            QString missingStationName="Название на предыдущей строке";
                            if (!table.dividedData.isEmpty())
                            {
                                if (!table.dividedData.last().isEmpty())
                                {
                                    missingStationName=table.dividedData.last().first();
                                }
                            }
                            dataFromString << missingStationName;
                            k=1;
                        }

                        if (_spec->getTableData(table.tableName).isEmpty()) // Если таблица отсутсвует в спецификации
                        {
                            for(;k*2+1<myMatch.lastCapturedIndex()+1;k++)
                                if (onlySpaces.match(myMatch.captured(k*2+1)).hasMatch()){  // Если данных нет, вернуть пустую строку
                                    dataFromString << "";
                                }
                                else{ // Иначе записать данные без пробелов по бокам
                                    dataFromString << withoutSpaces.match(myMatch.captured(k*2+1)).captured(2);
                                }
                            table.dividedData << dataFromString; // Запись данных из строки в структуру
                        }
                        else
                            table.dividedData+=MultipleStrings(dataFromString, myMatch, k, table.tableName);
                    }
                    else // Для строк, в которых "плохие" данные
                    {
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
        return table;
    }
}

QStringList TableReader::GetTableNames()
{
    QString text;
    QRegularExpressionMatch match;
    QRegularExpression tableNameRead("Таблица[ 0-9.]+([0-9а-яА-Я " + minus + "–,]+)");
    QStringList names;

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

QList<QStringList> TableReader::MultipleStrings(QStringList dataFromString, QRegularExpressionMatch myMatch, int k, QString tableName)
{
    QList<QStringList> multipleDateStrings;
    multipleDateStrings << dataFromString;

    QRegularExpression onlySpaces("^ +$");
    QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

    int specColumnIndex=k+3;

    for (;k*2+1<myMatch.lastCapturedIndex()+1 && specColumnIndex<_spec->getTableData(tableName).columns.length();k++)
    {
        if (onlySpaces.match(myMatch.captured(k*2+1)).hasMatch())  // Если данных нет, вернуть пустую строку
        {
            for (int h=0;h<multipleDateStrings.length();h++){
                multipleDateStrings[h] << "";
            }
        }
        else // Иначе записать данные без пробелов по бокам
        {
            // Тут проверка на Дата
            if (_spec->getTableData(tableName).columns.at(specColumnIndex).dataType == "DATE"){
                QRegularExpression hackDate("([0-9]+)( |$)");
                QRegularExpressionMatchIterator iterator=hackDate.globalMatch(myMatch.captured(k*2+1));
                QList<QStringList> dataFromStringsBuffer;
                QList<QStringList> resultStrings;
                while (iterator.hasNext()){
                    QRegularExpressionMatch dividedDate=iterator.next();
                    dataFromStringsBuffer=multipleDateStrings;
                    QList<QStringList>::iterator iDataBuffer;
                    for(iDataBuffer = dataFromStringsBuffer.begin(); iDataBuffer != dataFromStringsBuffer.end() ; ++iDataBuffer){
                        *iDataBuffer << withoutSpaces.match(dividedDate.captured(1)).captured(2);
                    }
                    resultStrings+= dataFromStringsBuffer;
                }

                multipleDateStrings=resultStrings;
            }
            else
            // Тут проверка на время
            if (_spec->getTableData(tableName).columns.at(specColumnIndex).dataType == "HOUR") // Проверка на тип данных "время"
            {

                QRegularExpression hackTime("([0-9]+) +([0-9]+)");
                QRegularExpressionMatch timeMatch=hackTime.match(myMatch.captured(k*2+1));
                QList<QStringList>::iterator multipleDateStringsIterator;
                for(multipleDateStringsIterator = multipleDateStrings.begin(); multipleDateStringsIterator != multipleDateStrings.end() ; ++multipleDateStringsIterator){
                    *multipleDateStringsIterator << withoutSpaces.match(timeMatch.captured(1)).captured(2) << withoutSpaces.match(timeMatch.captured(2)).captured(2);
                }
                specColumnIndex++;
            }
            else
            {
                QList<QStringList>::iterator multipleDateStringsIterator;
                for(multipleDateStringsIterator = multipleDateStrings.begin(); multipleDateStringsIterator != multipleDateStrings.end() ; ++multipleDateStringsIterator){
                    *multipleDateStringsIterator << withoutSpaces.match(myMatch.captured(k*2+1)).captured(2);
                }
            }
        }
        specColumnIndex++;
    }

//    if (!specColumnIndex<_spec->getTableData(tableName).columns.length())
//        qDebug() << "Vot tut nakosyachil " << tableName;
    return multipleDateStrings;
}

TableData TableReader::GetDataFrom9Table(TableData table, QRegularExpression regExpForTable)
{
    int i=0,j=0;
    QString text,pattern;

    while (j<table.headers)  // Работа поочередно с каждым набором данных в таблице
    {
        i=0;
        QString varFor9Table;// Переменная, которая нужна для записи в 9 таблицу названия пункта
        while (i<table.tableStrings.length())
        {
            if (table.headers==1)
            {
                text=table.tableStrings.at(i);
            }
            else
            {
                pattern=".{"+ QString::number(table.headerPosition.at(j)) +"}(.{" + QString::number(table.headerLength) +"})";
                QRegularExpression oneTablePerString(pattern);
                text=oneTablePerString.match(table.tableStrings.at(i)).captured(1); // Берем из строки только нужную таблицу
            }

            // Для начала убираем из строки слова про посторонние провода и переходы
            text=CutProvod(text);
            // Теперь убираем из строки номер станции
            text=CutStationNumber(text);
            // Извлечение данных из строки

            // Тут добавляем пробелы, потому что иначе некоторые строки оказываются слишком короткими
            QRegularExpressionMatch myMatch=regExpForTable.match(text+"                                                                                                                                                                 "); // ДА ЗДРАВСТВУЮТ КОСТЫЛИ!!!
            QRegularExpression onlySpaces("^ +$");
            QRegularExpression meaninglessString("^[ _-]*$");
            QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

            if (!meaninglessString.match(text).hasMatch())
            {
                if (myMatch.hasMatch()) // Для строки с данными соответствующими колонкам
                {
                    QList<QString> dataFromString;

                    // Отдельная обработка 9 таблицы, в которую криво занесли данные (как так можно?)
                    dataFromString << varFor9Table;

                    int k=0;
                    if (_spec->getTableData(table.tableName).isEmpty()) // Если таблица отсутсвует в спецификации
                    {
                        while (k<myMatch.lastCapturedIndex()+1)
                        {
                            if (onlySpaces.match(myMatch.captured(k)).hasMatch()){  // Если данных нет, вернуть пустую строку
                                dataFromString << "";
                            }
                            else{ // Иначе записать данные без пробелов по бокам
                                dataFromString << withoutSpaces.match(myMatch.captured(k)).captured(2);
                            }
                            k+=2;
                        }
                        table.dividedData << dataFromString; // Запись данных из строки в структуру
                    }
                    else{
                        QList<QStringList> multipleDateStrings;
                        multipleDateStrings << dataFromString;

                        QRegularExpression onlySpaces("^ +$");
                        QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

                        int specColumnIndex=4;

                        for (;k*2+1<myMatch.lastCapturedIndex()+1 && specColumnIndex<_spec->getTableData(table.tableName).columns.length();k++)
                        {
                            if (onlySpaces.match(myMatch.captured(k*2+1)).hasMatch())  // Если данных нет, вернуть пустую строку
                            {
                                for (int h=0;h<multipleDateStrings.length();h++){
                                    multipleDateStrings[h] << "";
                                }
                            }
                            else // Иначе записать данные без пробелов по бокам
                            {
                                // Тут проверка на Дата
                                if (_spec->getTableData(table.tableName).columns.at(specColumnIndex).dataType == "DATE"){
                                    QRegularExpression hackDate("([0-9]+)( |$)");
                                    QRegularExpressionMatchIterator iterator=hackDate.globalMatch(myMatch.captured(k*2+1));
                                    QList<QStringList> dataFromStringsBuffer;
                                    QList<QStringList> resultStrings;
                                    while (iterator.hasNext()){
                                        QRegularExpressionMatch dividedDate=iterator.next();
                                        dataFromStringsBuffer=multipleDateStrings;
                                        QList<QStringList>::iterator iDataBuffer;
                                        for(iDataBuffer = dataFromStringsBuffer.begin(); iDataBuffer != dataFromStringsBuffer.end() ; ++iDataBuffer){
                                            *iDataBuffer << withoutSpaces.match(dividedDate.captured(1)).captured(2);
                                        }
                                        resultStrings+= dataFromStringsBuffer;
                                    }

                                    multipleDateStrings=resultStrings;
                                }
                                else
                                // Тут проверка на время
                                if (_spec->getTableData(table.tableName).columns.at(specColumnIndex).dataType == "HOUR") // Проверка на тип данных "время"
                                {
                                    QRegularExpression hackTime("([0-9]+) +([0-9]+)");
                                    QRegularExpressionMatch timeMatch=hackTime.match(myMatch.captured(k*2+1));
                                    QList<QStringList>::iterator multipleDateStringsIterator;
                                    for(multipleDateStringsIterator = multipleDateStrings.begin(); multipleDateStringsIterator != multipleDateStrings.end() ; ++multipleDateStringsIterator){
                                        *multipleDateStringsIterator  << timeMatch.captured(1) << timeMatch.captured(2);
                                    }
                                    specColumnIndex++;
                                }
                                else
                                {
                                    QList<QStringList>::iterator multipleDateStringsIterator;
                                    for(multipleDateStringsIterator = multipleDateStrings.begin(); multipleDateStringsIterator != multipleDateStrings.end() ; ++multipleDateStringsIterator){
                                        *multipleDateStringsIterator << withoutSpaces.match(myMatch.captured(k*2+1)).captured(2);
                                    }
                                }
                            }
                            specColumnIndex++;
                        }
                        table.dividedData+=multipleDateStrings; // Запись данных из строки в структуру
                    }
                }
                else // Для строк, в которых "плохие" данные
                {
                    QRegularExpression stationName("[а-яА-Яa-zA-Z0-9" + minus + "–]+[ а-яА-Яa-zA-Z0-9" + minus + "–]*");
                    if (stationName.match(text).hasMatch())
                    {
                        varFor9Table=stationName.match(text).captured();
                    }
                    else table.wrongStrings << text;
                }
            }
            i++;
        }
        j++;
    }
    table.ranges << 0; // Да, я закинул мусор, но нам нужно чтобы длина этого списка соответствовала числу столбцов +1
    return table;
}

QString TableReader::GetPatternForTable(QList<int> ranges)
{
    int i=0;

    QString pattern="^";
    while (i<ranges.length()-1) // -1 потому что в последней колонке бывает на символ меньше
    {
        pattern = pattern + "(.{" + QString::number((ranges.at(i++))-1) + "})( |$)";
    }

    pattern = pattern + "(.{" + QString::number((ranges.at(i++))-1) + "}.{0,2})( |$)";

    return pattern;
}

QString TableReader::CutProvod(QString text)
{
    QRegularExpression postProvod("(.+)(Пост.провод)(.+)");
    QRegularExpression perehod("(.+)(Переход.+месяца?)(.*)");
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

    return text;
}

QString TableReader::CutStationNumber(QString text)
{
    QRegularExpression stationNumber("( *)([0-9]+[.])([а-яА-Яa-zA-Z ]+)(.*)");
    QRegularExpressionMatch stationNumberMatch=stationNumber.match(text);
    if (stationNumberMatch.hasMatch())
    {
        text=stationNumberMatch.captured(1);
        for (int numOfNum = stationNumberMatch.captured(2).length();numOfNum>0;numOfNum--)
        {
            text+=" ";
        }
        text=text+stationNumberMatch.captured(3)+stationNumberMatch.captured(4);
    }

    return text;
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
    QRegularExpression notTable("Таблица *([0-9]+)a?. *([0-9а-яА-Я " + minus + ",.()–]+)[.…]{2,}"); // Как же я устал...
    QRegularExpression tableSearch("Таблица *([0-9]+)a?. *([0-9а-яА-Я " + minus + ",.()–]+)");
    //    QRegularExpression dataString(" *[0-9]+[.] *[а-яА-Я ,.]+");

    while (index<datafile.length()){ // Поиск объявления таблицы
        text = datafile.at(index++);
        match = tableSearch.match(text);

        if (match.hasMatch() && !notTable.match(text).hasMatch()) // Если текущая строка оказалась объявлением таблицы записать её название и номер
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
            if (firstSymbol.match(text).hasMatch())
            {
                pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "* *$";
            }
            else
            {
                pattern="^$";
            }
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
            if (firstSymbol.match(text).hasMatch())
            {
                pattern="^" +  QRegularExpression::escape(firstSymbol.match(text).captured(1)) + "* *$";
            }
            else
            {
                pattern="^$";
            }
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
        return nextTable;
    }
}

QString TableReader::GetCurrentString()
{
    return datafile[index];
}

TableData TableReader::GetTableByName(QString input)
{
    index = 0;
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

    // Запись в файл всех "неправильных" строк для облегчения исправления ошибок

    QFile wrongStringsOutput("Wrong_strings.txt");
    TableData table;
    QFile::OpenMode openMode;
    QFileInfo fileInfo(wrongStringsOutput.fileName());
    if(!fileInfo.exists() || fileInfo.size() > 2097152){
        openMode = QIODevice::WriteOnly | QIODevice::Text;
    }
    else{
        openMode = QIODevice::Append | QIODevice::Text;
    }

    if(wrongStringsOutput.open(openMode)){
        QTextStream out(&wrongStringsOutput);

        int i=0;
        while (i<allTables.length())
        {
            table=allTables.at(i);

            if (table.wrongStrings.length()>0)
            {
                out << endl << table.tableNumber << ".   "<< table.tableName << " Month " << table.month << " Year  " << table.year << " Edition  " << table.edition << endl << endl;
                for (int iter=0;iter<table.wrongStrings.length();iter++)
                {
                    out << table.wrongStrings.at(iter) << endl;
                }
            }
            i++;
        }
    }
    else qDebug() << "Can't open file <Wrong strings.txt>" << endl;

    // Дополнительная обработка 9 таблицы (ввод названий станций во все строки)
    QString varForStationName;
    for (int i=0, length=allTables.length(); i<length; i++)
    {
        if (allTables[i].tableNumber==9)
        {
            for (int j=0,lengthOfDvd=allTables[i].dividedData.length(); j<lengthOfDvd; j++)
            {
                QRegularExpression meaninglessString("^[ _-]*$");
                QRegularExpressionMatch match=meaninglessString.match(allTables[i].dividedData.at(j).first());
                if (!match.hasMatch())
                {
                    varForStationName=allTables[i].dividedData.at(j).first();
                }
                else
                {
                    if (!allTables[i].dividedData.at(j).isEmpty())
                    {
                        allTables[i].dividedData[j].pop_front(); // Удаляем мусор вместо названия станции
                        allTables[i].dividedData[j].push_front(varForStationName); // Добавляем название станции
                    }
                }
            }
        }
    }

    wrongStringsOutput.close();
    index=0;
    return allTables;
}

bool TableReader::ShowDividedTable(QList<QStringList > dividedData)
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

QList<TableData> TableReader::AddYearAndEdition()
{
    QList<TableData> allTables=GetAllTables();

    for (int i=0,length=allTables.length(); i<length; i++)
    {
        for (int j=0,length1=allTables[i].dividedData.length(); j<length1; j++)
        {
            allTables[i].dividedData[j].prepend(QString::number(allTables[i].month));
            allTables[i].dividedData[j].prepend(QString::number(allTables[i].year));
            allTables[i].dividedData[j].prepend(QString::number(allTables[i].edition));
        }
    }
    return allTables;
}

QList<int> TableReader::GetRanges(QString text)
{
    QList<int> ranges;

    QRegularExpression findRanges("(([ а-яА-Яa-zA-Z0-9.,/<>"+ QRegularExpression::escape("-") +"%]+[^ а-яА-Яa-zA-Z0-9.,/<>"+ QRegularExpression::escape("-") +"%])+)([ %а-яА-Яa-zA-Z0-9.,/<>"+ QRegularExpression::escape("-") +"]+)");

    QRegularExpressionMatch match=findRanges.match(text);

    QRegularExpression findSubRanges("[ а-яА-Яa-zA-Z0-9.,/<>"+ QRegularExpression::escape("-") +"%]+[^ а-яА-Яa-zA-Z0-9.,/<>"+ QRegularExpression::escape("-") +"%]");
    QRegularExpressionMatchIterator iterator=findSubRanges.globalMatch(match.captured(1));


    while (iterator.hasNext())
    {
        ranges << iterator.next().capturedLength();
    }
    ranges << match.capturedLength(3);

    return ranges;
}

TableReader::TableReader(QObject *parent) : QObject(parent)
{

}

//<<-----------{
void TableReader::setSpec(Specificator &spec)
{
    _spec = &spec;
}
//<<-----------}
