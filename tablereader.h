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
    bool SetData(QList<QString>);
    TableData GetNextTable();
    TableData GetTableByName(QString);
    QList<TableData> GetAllTables();
    bool ShowDividedTable(QList<QList<QString>>);
    bool ShowTables(QList<TableData>);
    TableReader();
};

#endif // TABLEREADER_H
