#include "interface.h"
#include <QStyle>
#include <QDebug>

Interface::Interface(QWidget *parent)
    : QWidget(parent)
{
    _lay.addWidget(&_menuButton, 0,0,1,1);
    _lay.addWidget(&_stateString, 0,1,1,1); _stateString.setText("нет подключения к БД");
    _lay.addWidget(&_tableList, 0,2,1,2);
    _lay.addWidget(&_table, 1,0,1,4);
    _lay.addWidget(&_infoString,2,0,1,1); _infoString.setText("-");
    setLayout(&_lay);

    //добавление меню и его пунктов
    _menuButton.setMenu(&_menu);
    _menuActions.push_back(_menu.addAction("Создать новую БД", this, SLOT(createDBFile())));
    _menuActions.push_back(_menu.addAction("Открыть существующую БД", this, SLOT(openDBFile())));
    _menuActions.push_back(_menu.addAction("Добавить новые данные", this, SLOT(addDataFiles())));
    _menuActions.push_back(_menu.addAction("Сохранить как...", this, SLOT(saveAsCSV())));


    //блокировка по умолчанию некоторых элементов меню (не кликабельные)
    _menuActions.at(2)->setEnabled(false);
    _menuActions.at(3)->setEnabled(false);

    _menuButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _menuButton.setMinimumSize(100, 30);

    _tableList.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _tableList.setMinimumHeight(30);
    _tableList.setMaximumWidth(450);
    _tableList.setMinimumWidth(450);
    _tableList.setLayoutDirection(Qt::RightToLeft);

    _messageBox.hide();

    connect(&_tableList, SIGNAL(activated(QString)), this, SLOT(selectComboBox(QString)));
    connect(&_iolog, SIGNAL(errorOut(Error)), this, SLOT(setMessageBox(Error)));
    _iolog.start();


    _storage.setInterationItems(_iolog, _table, _dataConnection);
    //_storage(_iolog, _table, _dataConnection);
    //_iolog.errorProgram(Error("testclass","testmethod","test"), MSG_ALL);

}

Interface::~Interface()
{

}

void Interface::createDBFile()
{
    QUrl path = QFileDialog::getSaveFileUrl(0,"Создать файл БД", QUrl(), "");
    if(path.isEmpty()){
        return ;
    }
    _dataConnection.path = path;
    if(!_storage.connectStorage()){
        setStateString("Нет соединения с БД");
        qDebug() << "Файл не может быть использован под нужды БД";
        _menuActions.at(2)->setEnabled(false);
    }
    else{
        _menuActions.at(2)->setEnabled(true);
        setStateString("БД подключена");
    }
    _storage.disconnectStorage();
    _table.clear(); _table.setColumnCount(0); _table.setRowCount(0);
    _tableList.clear();
}

void Interface::openDBFile()
{
    QUrl path = QFileDialog::getOpenFileUrl(0,"Открыть файл БД", QUrl(), "");
    if(path.isEmpty()){
        return ;
    }
    _dataConnection.path = path;
    if(!_storage.connectStorage()){
        setStateString("Нет соединения с БД");
        _menuActions.at(2)->setEnabled(false);
        return ;
    }
    else{
        setStateString("БД подключена");
        _menuActions.at(2)->setEnabled(true);
    }

    QStringList tables = _storage.namesTables();
    _storage.disconnectStorage();

    if(tables.isEmpty()){
        //В БД отстуствуют таблицы
        return ;
    }
    _tableList.clear();
    for(int iterator_tables = 0, tables_length = tables.length(); iterator_tables < tables_length; iterator_tables++){
        TableSpecificate spec = _iolog.getTable(tables.at(iterator_tables));
         _tableList.addItem(spec.originName);
    }

    if(tables.length() > 0)
        _tableList.activated(_iolog.getTable(tables.first()).originName);
}

void Interface::addDataFiles()
{
    QList<QUrl> paths = QFileDialog::getOpenFileUrls(0,"Загрузить файлы", QUrl(), "");
    if(paths.isEmpty()){
        return ;
    }

    if(!_storage.connectStorage()){
        setStateString("Нет соединения с БД");
        _menuActions.at(2)->setEnabled(false);
        return ;
    }
    else{
        setStateString("БД подключена");
        _menuActions.at(2)->setEnabled(true);
    }

    for(int iterator_paths = 0, paths_length = paths.length(); iterator_paths < paths_length; iterator_paths++){
        //получение пути, файла и его строк
        QString path = paths.at(iterator_paths).toLocalFile();
        QFile file(path);
        QStringList strings_file = _iolog.readFile(file);
        if(strings_file.isEmpty()){
            //строки не получены
            return ;
        }

        //получение считанных данных
        TableReader tableReader;
        tableReader.SetData(strings_file);
        QList<TableData> tablesData(tableReader.AddYearAndEdition());
        if(tablesData.isEmpty()){
            //данные не считаны
            return ;
        }


        for(int iterator_tablesData = 0, tablesData_length = tablesData.length(); iterator_tablesData < tablesData_length; iterator_tablesData++){
            TableData table = tablesData.at(iterator_tablesData);
            //проверка на существование
            if(!_storage.writeData(table.tableName, table.dividedData)){
                //Что-то не записалось (мб даже ничего не записалось)
            }


        }
        qDebug() << "writeAll OK";
    }

    QStringList tables = _storage.namesTables();
    _storage.disconnectStorage();

    if(tables.isEmpty()){
        //В БД отстуствуют таблицы
        return ;
    }
    _tableList.clear();
    for(int iterator_tables = 0, tables_length = tables.length(); iterator_tables < tables_length; iterator_tables++){
        TableSpecificate spec = _iolog.getTable(tables.at(iterator_tables));
         _tableList.addItem(spec.originName);
    }

    if(tables.length() > 0)
        _tableList.activated(_iolog.getTable(tables.first()).originName);

}

void Interface::saveAsCSV()
{

}

void Interface::selectComboBox(QString nameTable)
{
    qDebug() << "link";
    if(!_storage.connectStorage()){
        setStateString("Нет соединения с БД");
        _menuActions.at(2)->setEnabled(false);
        return ;
    }
    else{
        setStateString("БД подключена");
        _menuActions.at(2)->setEnabled(true);
    }

    _storage.readData(nameTable);
    qDebug() << "readDataAll ok";

    _storage.disconnectStorage();
}

void Interface::setInfoString(QString &data)
{
    _infoString.setText(data);
}

void Interface::setStateString(QString data)
{
    _stateString.setText(data);
}

void Interface::setMessageBox(const Error error)
{
    //_messageBox.setWindowIconText("Ошибка!");
    _messageBox.setWindowTitle("Error "+error.errorClass+":: "+error.errorMethod);
    _messageBox.setText(error.text);
    _messageBox.show();
    _messageBox.exec();
}


