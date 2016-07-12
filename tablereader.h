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
public:
    bool SetData(QList<QString>);
    QList<QString> GetTableNames();
    int ContentsSearch();
    int GetCurrentIndex();
    TableData GetNextTable();
    QString GetCurrentString();
    TableData GetTableByName(QString);
    QList<TableData> GetAllTables();
    QList<int> GetRanges(QString);
    bool ShowTables(QList<TableData>);
    TableReader();
};

#endif // TABLEREADER_H
