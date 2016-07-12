#ifndef DATAFROMTABLES_H
#define DATAFROMTABLES_H
#include <QRegularExpression>
#include <tablereader.h>

class dataFromTables
{
public:
    QRegularExpression CreateRegExpForTable(QList<int>);
    TableData GetDataFromTable(TableData);
    QList<QString> GetDataFromString(QString, QRegularExpression);
    bool ShowDividedTable(QList<QList<QString>>);
    dataFromTables();
};

#endif // DATAFROMTABLES_H
