#include "datafromtables.h"
#include <QRegularExpression>
#include <QDebug>

QRegularExpression dataFromTables::CreateRegExpForTable(QList<int> ranges)
{
    QString pattern="";
    int i=0;
    while (i<ranges.length())
    {
        pattern=pattern + "(.{" + QString::number(ranges.at(i++)-1) + "}) ";
    }

    QRegularExpression regexp(pattern);
    return regexp;
}

TableData dataFromTables::GetDataFromTable(TableData table)
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
            i=0; // Итератор для считывания всех строк таблицы
            while (i<table.tableStrings.length())
            {
                text=table.tableStrings.at(i);

                // Извлечение данных из строки
                QRegularExpressionMatch match=regExpForTable.match(text);
                QRegularExpression onlySpaces("^ +$");
                QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

                if (match.hasMatch()) // Для строки с данными соответсвующими колонкам
                {
                    QList<QString> dataFromString;
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
                while (i<table.tableStrings.length())
                {
                    text=oneTablePerString.match(table.tableStrings.at(i++)).captured(1); // Берем из строки только нужную таблицу

                    // Извлечение данных из строки
                    QRegularExpressionMatch match=regExpForTable.match(text);
                    QRegularExpression onlySpaces("^ +$");
                    QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");

                    if (match.hasMatch()) // Для строки с данными соответсвующими колонкам
                    {
                        QList<QString> dataFromString;
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

// Этот метод уже не нужен, потому что стал частью GetDataFromTable, но я его оставлю на всякий случай
QList<QString> dataFromTables::GetDataFromString(QString input, QRegularExpression regexp)
{
    QRegularExpressionMatch match=regexp.match(input),match1;
    QRegularExpression onlySpaces("^ +$");
    QRegularExpression withoutSpaces("^( *)(.*[^ \f\n\r\t\v]+)( *)$");
    QList<QString> dataFromString;

    if (match.hasMatch()) // Для строк с данными соответсвующими колонкам
    {
        QString ans;
        int i=1;
        while (i<match.lastCapturedIndex()+1)
        {
            ans = match.captured(i);
            match1=onlySpaces.match(ans);
            if (match1.hasMatch())
            {
                dataFromString << "_";
            }
            else
            {
                match1=withoutSpaces.match(ans);
                dataFromString << match1.captured(2);
            }
            i++;
        }
    }
    else
    {

    }
    return dataFromString;
}

bool dataFromTables::ShowDividedTable(QList<QList<QString> > dividedData)
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

dataFromTables::dataFromTables()
{

}
