#ifndef TABLEREADER_H
#define TABLEREADER_H

#include <QObject>
#include <QRegularExpression>
#include "specificator.h" //<<-----------

struct TableData{
    int tableNumber=0;
    QString tableName="";
    QStringList tableStrings;
    QStringList wrongStrings;
    QList<QStringList> dividedData;
    QList<int> ranges,
               headerPosition;
    int month = 0,
        year  = 0,
        edition=0,
        headers=1,
        headerLength;
};

class TableReader : public QObject
{
    Q_OBJECT

    Specificator * _spec = Q_NULLPTR; //<<-----------

    QStringList datafile;
    int index=0;
    QString minus = QRegularExpression::escape("-");
    TableData GetDataFromTable(TableData);
    int ContentsSearch();
    int GetCurrentIndex();
    QString GetCurrentString();
    QList<int> GetRanges(QString);
    QStringList GetTableNames();
    QList<QStringList> MultipleStrings(QStringList, QRegularExpressionMatch, int, QString);
    TableData GetDataFrom9Table(TableData, QRegularExpression);
    QString GetPatternForTable(QList<int>);
    QString CutProvod(QString);
    QString CutStationNumber(QString);
public:
    bool SetData(QStringList); // Запись строк файла в класс
    TableData GetNextTable();  // Получение следующей таблицы
    TableData GetTableByName(QString); // Получение таблицы по названию
    QList<TableData> GetAllTables(); // Получение списка всех таблиц
    bool ShowDividedTable(QList<QStringList>); // Вывод отформатированных данных из таблицы в консоль
    bool ShowTables(QList<TableData>); // Вывод исходных строк (с необработанными данными) всех таблиц
    QList<TableData> AddYearAndEdition(); // Получение всех таблиц, где в данные нагло впихнуты месяц, год и выпуск
    explicit TableReader(QObject *parent = 0);

    void setSpec(Specificator & spec); //<<-----------
};

#endif // TABLEREADER_H
