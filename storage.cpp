#include "storage.h"


QString Storage::transliteration(QString text)
{
    QStringList nw_alphabet;
    QString result = "";
    QString rusLower = "абвгдеёжзийклмнопрстуфхцчшщыэюя -";
    QString rusUpper = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЫЭЮЯ -";
    QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_";
    nw_alphabet << "a" << "b" << "v" << "g" << "d" << "e" << "e" << "j" << "z" << "i" << "i" << "k"
                << "l" << "m" << "n" << "o" << "p" << "r" << "s" << "t" << "y" << "f" << "h" << "c"
                << "ch" << "sh" << "sch" << "i" << "e" << "u" << "ya" << "_" << "_" ;
    int count = text.length();
    for(int i =0; i<count; i++){
        QChar ch = text.at(i);
        if(validChars.indexOf(ch) != -1)
            result += ch;
        else if(rusLower.indexOf(ch) != -1)
                result += nw_alphabet.at(rusLower.indexOf(ch));
            else if(rusUpper.indexOf(ch) != -1)
                    result += nw_alphabet.at(rusUpper.indexOf(ch));
    }
    return result.toLower();
}

QString Storage::quotes(QString data, QString type)
{
    if(type == "TEXT")
        return "'" + data + "'";
    else if(data == "-" || data == "" || data == "_")
        return "NULL";
    return data;
}


Storage::Storage(QObject *parent) : QObject(parent)
{
    _iolog = Q_NULLPTR;
    _table = Q_NULLPTR;
    _dataConnection = Q_NULLPTR;
    _dataBase.close();
}

Storage::~Storage()
{

}

void Storage::setInterationItems(IOLog &iolog, QTableWidget &table, DataConnection &dataConnection)
{
    _iolog = &iolog;
    _table = &table;
    _dataConnection = &dataConnection;
}

bool Storage::connectStorage()
{
    if(_iolog == Q_NULLPTR || _table == Q_NULLPTR || _dataConnection == Q_NULLPTR){
        //проверка на существование элкментов взаимодействия
        disconnectStorage();
        return false;
    }


    if(_dataConnection->isEmpty()){
        _iolog->errorProgram(Error("Storage","connectStorage","Данные для соединения с БД не заданы или некорректны"), MSG_LOG_QDEBUG);
        return false;
    }

    _dataBase.setDatabaseName(_dataConnection->path.toLocalFile());
    _dataBase.setUserName(_dataConnection->user);
    _dataBase.setHostName(_dataConnection->host);
    _dataBase.setPassword(_dataConnection->pass);

    if(_dataConnection->port != -1){
        _dataBase.setPort(_dataConnection->port);
    }

    if(!_dataBase.open()){
        _iolog->errorProgram(Error("Storage","connectStorage","БД не может быть открыта"), MSG_LOG_QDEBUG);
        return false;
    }
    //qDebug() << "Соединение успешно";
    //qDebug() << _iolog << _table << _dataConnection;
    return true;
}

bool Storage::disconnectStorage()
{
    _dataBase.close();
    if(_dataBase.isOpen()){
        _iolog->errorProgram(Error("Storage","disconnectStorage","БД не может быть отключена"), MSG_LOG_QDEBUG);
        return false;
    }
    return true;
}


bool Storage::readData(QString nameTable)
{
    if(!_dataBase.isOpen()){
        //БД не открыта
        return false;
    }

    if(!isExistTable(nameTable)){
        //такой таблицы не существует
        return false;
    }

    TableSpecificate spec = _iolog->getTable(nameTable);
    //QString request = "SELECT %1 FROM %2 WHERE %3";
    QString request = "SELECT * FROM "+ spec.transliterationName;

    QSqlQuery query;
    if(!query.exec(request)){
        qDebug() << "Запрос был отклонен: " << query.lastQuery() << endl << query.lastError();
    }
    qDebug() << query.lastQuery();

    QSqlRecord record = query.record();
    //QStringList values;
    _table->clear();

    _table->setColumnCount(spec.columns.length());
    _table->setRowCount(1);


    QStringList columns;
    for(int iterator_columns = 0, columns_length = spec.columns.length(); iterator_columns < columns_length; iterator_columns++){
        columns << spec.columns.at(iterator_columns).viewName;
    }

    _table->setHorizontalHeaderLabels(columns);
    //int iterator_strs;
    //for(iterator_strs = 1;query.next(); iterator_strs++){
    for(int iterator_strs = 1;query.next(); iterator_strs++){
        //values.clear();
        _table->setRowCount(_table->rowCount()+1);

        for(int iterator_columns = 0, columns_length = spec.columns.length(); iterator_columns < columns_length; iterator_columns++){
            _table->setItem(iterator_strs, iterator_columns, new QTableWidgetItem(query.value(spec.columns.at(iterator_columns).transliterateName).toString()));
            //values << query.value(spec.columns.at(iterator_columns).transliterateName);

        }

    }


    //QSqlTableModel * model = new QSqlTableModel;
    //model->setTable(nameTable);
    //model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    //model->select();
    //delete _table->model();
    //table->setModel(model);
    //

    return true;
}

bool Storage::writeData(QString nameTable, QList<QList<QString> > values)
{
    if(!_dataBase.isOpen()){
        //БД не открыта
        return false;
    }

    if(!isExistTable(nameTable)){
        //Этой таблицы нет в БД
        if(!createTable(nameTable)){
            //таблица не создалась
            return false;
        }
    }

    TableSpecificate spec = _iolog->getTable(nameTable); //Валидность уже проверена в isExitentTable
    for(int iterator_strs = 0, strs_length = values.length(); iterator_strs < strs_length; iterator_strs++){ //Будет пройдена каждая строка
        QString insert_columns = "";
        QString insert_values = "";
        int values_length = (spec.columns.length() < values.at(iterator_strs).length())? spec.columns.length(): values.at(iterator_strs).length();
        for(int iterator_values = 0; iterator_values < values_length; iterator_values++){ //values_length = values.at(iterator_strs).length()
            if(iterator_values != 0){
                insert_columns += ", ";
                insert_values += ", ";
            }
            insert_columns += spec.columns.at(iterator_values).transliterateName;
            insert_values += quotes(values.at(iterator_strs).at(iterator_values), spec.columns.at(iterator_values).dataType);
        }
        QString request = "";
        request += "INSERT INTO " + spec.transliterationName + " (" + insert_columns + ") VALUES(" + insert_values + ")";

        QSqlQuery query;
        if(!query.exec(request)){
            qDebug() << "Запрос был отклонен: " << query.lastQuery() << endl << query.lastError();
            //здесь ретурн не надо ибо запросов тьма
        }
    }
    qDebug() << "write ok";
    return true;
}

QStringList Storage::namesTables()
{
    if(!_dataBase.isOpen()){
        //БД не открыта
        return QStringList();
    }

    return _dataBase.tables(QSql::Tables);
}

bool Storage::isExistTable(QString nameTable)
{
    if(!_dataBase.isOpen()){
        //БД не открыта
        return false;
    }

    QStringList tables = _dataBase.tables(QSql::Tables);
    if(tables.isEmpty()){
        //в БД таблиц не существует
        return false;
    }

    TableSpecificate spec = _iolog->getTable(nameTable);
    if(spec.isEmpty()){
        //данной таблицы не существует в спецификации
        return false;
    }

    for(int iterator = 0, table_length = tables.length(); iterator < table_length; iterator++){
        if(tables.at(iterator) == spec.originName || tables.at(iterator) == spec.transliterationName){
            qDebug() << "isExist ok";
            return true;
        }
    }

    return false;
}

bool Storage::createTable(QString nameTable)
{
    if(!_dataBase.isOpen()){
        //БД не открыта
        return false;
    }

    if(isExistTable(nameTable)){
        //Эта таблица уже есть в БД
        return false;
    }

    TableSpecificate spec = _iolog->getTable(nameTable); //Валидность уже проверена в isExistTable
    if(spec.isEmpty()){
        //если такого нет в спецификации
        return false;
    }
    QString sub_request = "";
    QString primaryKeys = "";
    for(int iterator = 0, columns_length = spec.columns.length(); iterator < columns_length; iterator++){ //получение имент и типов колонок + записывание ключей
        if(spec.columns[iterator].isEmpty()){//проверка на валидность всех колонок таблицы
            //есть не валидная колонка в таблице
            return false;
        }

        if(sub_request != ""){
            sub_request += ", ";
        }

        sub_request += spec.columns.at(iterator).transliterateName + " " + spec.columns.at(iterator).dataType;
        if(spec.columns.at(iterator).isPrimaryKey == "yes"){
            if(primaryKeys != ""){
                primaryKeys += ", ";
            }
            primaryKeys += spec.columns.at(iterator).transliterateName;
        }
    }

    if(primaryKeys != ""){
        sub_request += ", PRIMARY KEY(" + primaryKeys + ")";
    }

    QString request = "CREATE TABLE "+spec.transliterationName+" ("+ sub_request +")";

    QSqlQuery query;
    if(!query.exec(request)){
        qDebug() << "Запрос был отклонен: " << query.lastQuery() << endl << query.lastError();
        return false;
    }
    qDebug() << "create ok";
    return true;
}


