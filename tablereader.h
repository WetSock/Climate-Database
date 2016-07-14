#ifndef TABLEREADER_H
#define TABLEREADER_H

#include <QRegularExpression>

struct TableData{
    int tableNumber=0;
    QString tableName="";
    QList<QString> tableStrings;
    QList<QString> wrongStrings;
    QList<QList<QString>> dividedData;
    QList<int> ranges,
               headerPosition;
    int month = 0,
        year  = 0,
        edition=0,
        headers=1,
        headerLength;
};

class TableReader
{
    QList<QString> datafile;
    int index=0;
    QString minus = QRegularExpression::escape("-");
    TableData GetDataFromTable(TableData);
    int ContentsSearch();
    int GetCurrentIndex();
    QString GetCurrentString();
    QList<int> GetRanges(QString);
    QList<QString> GetTableNames();
public:
    bool SetData(QList<QString>); // Запись строк файла в класс
    TableData GetNextTable();  // Получение следующей таблицы
    TableData GetTableByName(QString); // Получение таблицы по названию
    QList<TableData> GetAllTables(); // Получение списка всех таблиц
    bool ShowDividedTable(QList<QList<QString>>); // Вывод отформатированных данных из таблицы в консоль
    bool ShowTables(QList<TableData>); // Вывод исходных строк (с необработанными данными) всех таблиц
    QList<TableData> AddYearAndEdition();
    TableReader();
};

#endif // TABLEREADER_H
