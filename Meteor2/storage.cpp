#include "storage.h"

bool Storage::connectSF()
{
    if(_pathSF.toLocalFile() == ""){
        //qDebug() << "Невозможно подключиться к БД: некоректный путь";
        return false;
    }
    _specFile.setDatabaseName(_pathSF.toLocalFile());

    _specFile.setUserName("ADMIN");
    _specFile.setHostName("localhost");
    _specFile.setPassword("");

    if(!_specFile.open()){
        qDebug() << "Spec не открыт: " << _specFile.lastError() << endl << _pathSF.toLocalFile();
        //запись в лог файл
        return false;
    }
        //qDebug() << "Успешное подключение к БД";
    return true;
}

bool Storage::disconnectSF()
{
    _specFile.close();
    if(_specFile.isOpen()){
        //qDebug() << "Spec не закрыт!";
        return false;
    }
    return true;
}

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

}

Storage::~Storage()
{

}

bool Storage::setDataConnectionSF(QUrl path)
{
    _pathSF = path;
    return true;
}

//                               view                view                         0 or 1
bool Storage::writeToSF(QString nameTable, QString nameColumn, QString dataType, QString PK)
{

    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return false;
    }
    QStringList namesTables = _specFile.tables(QSql::Tables);
    disconnectSF();
    if(namesTables.indexOf("spec_info_table") == -1){
        qDebug() << "Регистрация новой таблицы невозможна: отсутствует spec_info_table";
        return false;
    }

    QSqlQuery specQuery(_specFile);
    if(isExistentTableIntoSF(nameTable)){
        if(!specQuery.exec("INSERT INTO spec_info_table (view_name, format_name) VALUES('"+ nameTable +"', '"+ transliteration(nameTable) +"')")){
            qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
            return false;
        }
    }

    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return false;
    }
    if(!specQuery.exec("SELECT format_name FROM spec_info_table WHERE view_name='"+ nameTable +"'")){
        qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
        disconnectSF();
        return false;
    }
    QStringList specList;
    while(specQuery.next()){
        specList << specQuery.value("format_name").toString();
    }
    disconnectSF();

    if(specList.length() != 1){
        qDebug() << "Регистрация новой таблицы невозможна: forma_name в данной таблице (не) встречается не один раз";
        disconnectSF();
        return false;
    }

    QString request = "";
    request += "CREATE TABLE IS NOT EXIST "+ specList.first() +" (format_name TEXT NOT NULL, "
                                                               "view_name TEXT NOT NULL, "
                                                               "data_type TEXT NOT NULL, "
                                                               "is_key INTEGER NOT NULL, "
                                                               "PRIMERY KEY(format_name))";
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return false;
    }
    if(!specQuery.exec(request)){
        qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
        disconnectSF();
        return false;
    }
    disconnectSF();

    request.clear();
    request += "SELECT * FROM "+ specList.first() +" WHERE format_name='%2'";
    request.arg(transliteration(nameColumn));
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return false;
    }
    if(!specQuery.exec(request)){
        qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
        disconnectSF();
        return false;
    }
    QStringList resultList;
    while(specQuery.next()){
        resultList << specQuery.value("format_name").toString();
    }
    disconnectSF();
    if(resultList.length() > 0){
        qDebug() << "Регистрация нового столбца невозможна: такой столбец уже существует";
        return false;
    }

    request.clear();
    request += "INSERT INTO "+ specList.first() +" (format_name,view_name,data_type,is_key) ";
    request += "VALUES ('%1','%2','%3',%4)";
    request.arg(transliteration(nameColumn), nameColumn, dataType, PK);
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return false;
    }
    if(!specQuery.exec(request)){
        qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
        disconnectSF();
        return false;
    }
    disconnectSF();
    return true;
}

QStringList Storage::getFormatNamesColumnsSF(QString formatNameTable)
{

    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return QStringList();
    }
    QStringList namesTables = _specFile.tables(QSql::Tables);
    disconnectSF();
    if(namesTables.indexOf("spec_info_table") == -1){
        qDebug() << "Получить данные невозможно: отсутствует spec_info_table";
        disconnectSF();
        return QStringList();
    }

    QSqlQuery query(_specFile);
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return QStringList();
    }
    if(!query.exec("SELECT format_name FROM spec_info_table WHERE format_name='"+ formatNameTable +"'")){
        qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
        disconnectSF();
        return QStringList();
    }
    namesTables.clear();
    while(query.next()){
        namesTables << query.value("format_name").toString();
    }
    disconnectSF();

    if(namesTables.length() != 1){
        qDebug() << "Получить данные невозможно: данное имя неуникально или не существует";
        return QStringList();
    }

    QStringList namesColumns;
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return QStringList();
    }
    if(!query.exec("SELECT format_name FROM "+ formatNameTable )){
        qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
        disconnectSF();
        return QStringList();
    }
    while(query.next()){
        namesColumns << query.value("format_name").toString();
    }
    disconnectSF();
    return namesColumns;


}

QStringList Storage::getInfoColumnSF(QString formatNameColumn, QString formatNameTable)
{


    QStringList columns = getFormatNamesColumnsSF(formatNameTable);
    if(columns.indexOf(formatNameColumn) == -1){
        qDebug() << "Получить данные невозможно: такого столбца не существует";
        return QStringList();

    }

    QSqlQuery query(_specFile);
    if(!connectSF()){
        qDebug() << "Регистрация новой таблицы невозможна: spec закрыт";
        return QStringList();
    }
    if(!query.exec("SELECT * FROM "+ formatNameTable+" WHERE format_name='"+ formatNameColumn +"'")){
        qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
        disconnectSF();
        return QStringList();
    }

    QStringList values;
    while(query.next()){
        values << query.value("format_name").toString();
        values << query.value("view_name").toString();
        values << query.value("data_type").toString();
        values << query.value("is_key").toString();
    }

    disconnectSF();

    return values;
}

bool Storage::setDataConnectionDB(QUrl path, QString user, QString host, int port, QString pass)
{
    _pathDB=path;
    _user=user;
    _host=host;
    _port=port;
    _pass=pass;
    return true;
}

bool Storage::outDB(QTableView &table, QString nameTable)
{
    if(!isExistentTableIntoDB(nameTable)){
        qDebug() << "Таблица не найдена: " << nameTable;
        return false;
    }

//    QSqlQuery query1;
//    query1.exec("PRAGMA table_info(" + nameTable +")");
//    QSqlRecord rec = query1.record();
//    while(query1.next())
//        qDebug() << "name: " << query1.value(rec.indexOf("name")).toString();


    QSqlQuery query;
    if(!connectDB()){
        qDebug() << "Невозможно продолжить раюоту: нет оединения с БД";
        return false;
    }
    query.exec("SELECT * FROM "+nameTable);
    QSqlTableModel  * model = new QSqlTableModel;
    model->setTable(nameTable);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

//    QStringList nameColumns;
//    foreach(QString nameColunm, nameColumns){
//        model->setHeaderData(nameColumns.indexOf(nameColunm), Qt::Horizontal, QVariant(nameColunm));
//        //qDebug() << nameColunm;
//    }

    model->select();
    disconnectDB();
    QAbstractItemModel * old_model = table.model();
    delete old_model;
    table.setModel(model);


    return true;
}

bool Storage::writeToDB(QString nameTable, QStringList values, int index)
{

    if(!isExistentTableIntoDB(nameTable)){
        if(!createTableIntoDB(nameTable)){
            qDebug() << "Невозможно ввести данные в БД";
            return false;
        }
    }

    QSqlQuery query(_dataBase);
    QString request = "";
    QString formatNameTable = transliteration(nameTable);
    QStringList namesColumns = getFormatNamesColumnsSF(formatNameTable);
    if(namesColumns.isEmpty()){
        qDebug() << "в spec отсутствуют колонки у этой таблицы: " << nameTable;
        return false;
    }

    if(values.length() > namesColumns.length()){
        qDebug() << "Слишком много значений передано в values";
        return false;
    }

    if(index == -1){
        request += "INSERT INTO " + formatNameTable + " (";
        foreach(QString name, namesColumns){
            request += name;
            if(namesColumns.last() != name){
                request += ", ";
            }
            else{
                request += ") ";
            }
        }
        request += "VALUES(";

        QString post_request = request;
        foreach(QString name, namesColumns){
            int iterator = namesColumns.indexOf(name);
            if(iterator == -1){
                break;
            }

            if(namesColumns.first() != name){
                post_request = ", ";
            }

            QStringList infoColumn = getInfoColumnSF(name,formatNameTable);
            if(infoColumn.length() != 4){
                qDebug() << "В колонке " + name + " некорретное количество данных";
                return false;
            }

            post_request += quotes(values.at(iterator), infoColumn.at(2));

        }
        post_request += ")";

        if(!connectDB()){
            qDebug() << "Невозможно продолжить раюоту: нет оединения с БД";
            return false;
        }
        if(!query.exec(post_request)){
            qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
            disconnectDB();
            return false;
        }
        disconnectDB();
    }
    else{
        request += "UPDATE " + formatNameTable + " SET ";
        QStringList primaries = primaryKeys(formatNameTable);

        for(int i = primaries.length() ;( i < namesColumns.length()) && (i < values.length()); i++){

            QStringList infoColumn = getInfoColumnSF(namesColumns.at(i), formatNameTable);
            if(infoColumn.length() != 4){
                qDebug() << "Некорретные данные";
                return false;
            }

            request += namesColumns.at(i+index) + "=" + quotes(values.at(i), infoColumn.at(2));
            if(values.at(i) != values.last()){
                request += ", ";
            }

        }
        request += " WHERE ";
        foreach(QString key, primaries){
            int iterator = primaries.indexOf(key);
            if(iterator >= values.length()){
                qDebug() << "значений оказалось неожиданно мало";
                return false;
            }
            QStringList infoColumn = getInfoColumnSF(namesColumns.at(iterator), formatNameTable);
            request += key + "=" + quotes(values.at(iterator), infoColumn.at(2));
            if(primaries.last() != key){
                request += " AND ";
            }
        }
        if(!connectDB()){
            qDebug() << "Невозможно продолжить раюоту: нет оединения с БД";
            return false;
        }
        if(!query.exec(request)){
            qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
            disconnectDB();
            return false;
        }
        disconnectDB();
    }
    return true;
}

bool Storage::isExistentTableIntoSF(QString nameTable)
{

    QSqlQuery specQuery(_specFile);
    if(!connectSF()){
        qDebug() << "Невозможно продолжить работу: нет оединения с spec";
        return false;
    }
    if(!specQuery.exec("SELECT view_name FROM spec_info_table")){
        qDebug() << "Запрос отклонен:" << specQuery.lastQuery() << endl << specQuery.lastError();
        disconnectSF();
        return false;
    }
    //QSqlRecord specRecord = specQuery.record();
    QStringList specList;
    while(specQuery.next()){
        specList << specQuery.value("view_name").toString();
    }
    disconnectSF();

    if(specList.indexOf(nameTable) == -1){
        return false;
    }

    return true;
}

QStringList Storage::primaryKeys(QString formatNameTable)
{
    QSqlQuery query(_specFile);
    if(!connectSF()){
        return QStringList();
    }
    if(!query.exec("SELECT * FROM "+ formatNameTable )){
        return QStringList();
    }
    QStringList result;
    while(query.next()){
        if(query.value("is_key") == "1"){
            result << query.value("format_name").toString();
        }
    }
    disconnectSF();

    return result;
}


bool Storage::connectDB()
{
    if(_pathDB.toLocalFile() == ""){
        qDebug() << "Невозможно подключиться к БД: некоректный путь";
        return false;
    }
    _dataBase.setDatabaseName(_pathDB.toLocalFile());

    _dataBase.setUserName(_user);
    _dataBase.setHostName(_host);
    _dataBase.setPassword(_pass);

    if(_port)
        _dataBase.setPort(_port);

    if(!_dataBase.open()){
        qDebug() << "БД не открыта: " << _dataBase.lastError() << endl << _pathDB.toLocalFile();
        //запись в лог файл
        return false;
    }
    return true;

}

bool Storage::disconnectDB()
{
    _dataBase.close();
    if(_dataBase.isOpen()){
        //qDebug() << "Spec не закрыт!";
        return false;
    }
    return true;
}

bool Storage::isExistentTableIntoDB(QString nameTable)
{
    if(!connectDB()){
        qDebug() << "Невозможно продолжить работу: нет оединения с БД";
        return false;
    }
    QStringList tables = _dataBase.tables(QSql::Tables);
    disconnectDB();
    //qDebug() << "имена таблиц: " << tables;
    if(tables.indexOf(nameTable) == -1){
        //qDebug() << "Эта таблица уже существует в БД: " << nameTable;
        return false;
    }
    return true;
}

bool Storage::createTableIntoDB(QString nameTable)
{
    if(!isExistentTableIntoSF(nameTable)){
        qDebug() << "Данная таблица отсутствует в spec";
        return false;
    }
    if(isExistentTableIntoDB(nameTable)){
        qDebug() << "Данная таблица уже существует в БД";
        return false;
    }


    QStringList namesColumns = getFormatNamesColumnsSF(transliteration(nameTable));
    if(namesColumns.length() < 1){
        qDebug() << "У таблицы не существуют колонки в spec";
        return false;
    }

    QSqlQuery query(_dataBase);
    QString request = "";
    request += "CREATE TABLE "+ transliteration(nameTable) +" (";
    QStringList PKs;
    foreach(QString name, namesColumns){
        QStringList values = getInfoColumnSF(name, transliteration(nameTable));
        if(values.length() != 4){
            qDebug() << "Количество значений некоррктно";
            return false;
        }
        request += values.at(0) +" "+ values.at(2);
        if(values.at(3) =="1"){
            PKs << name;
        }
        if(namesColumns.last() != name){
            request += ", ";
        }

    }
    if(PKs.length() > 0){
        request += ", PRIMARY KEY(";
        foreach(QString name, PKs){
            request += name;
            if(PKs.last() != name){
                request += ", ";
            }
            else{
                request += "))";
            }
        }
    }
    if(!connectDB()){
        qDebug() << "Невозможно продолжить работу: нет оединения с БД";
        return false;
    }
    if(!query.exec(request)){
        qDebug() << "Запрос отклонен:" << query.lastQuery() << endl << query.lastError();
        disconnectDB();
        return false;
    }
    disconnectDB();
    return true;
}

