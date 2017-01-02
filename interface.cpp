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
    _menuActions.push_back(_menu.addAction("Зарегистрирвоать новую таблицу", this, SLOT(tableCreator())));


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
    if(!_iolog.start()){
        setMessageBox(Error("Interface","Interface","Отсутствует файл спецификации. Программа не сможет работать в штатном режиме."));
    }


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
            _storage.writeData(table.tableName, table.dividedData);

            //Если этой таблицы нет спецификации, то нужно занести инфу в список неидентифицированных таблиц
            if(!_iolog.isExistTable(table.tableName)){
                _iolog.addUnidentifiedTables(table.tableName, table.ranges.length());
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

void Interface::tableCreator()
{
    _menuActions.at(4)->setEnabled(false); //что бы не было дублирований этого окна

    QGridLayout * creater_lay  = new QGridLayout();
    QPushButton * closeButton = new QPushButton("Закрыть");
    QComboBox * unidTables = new QComboBox(); unidTables->setObjectName("tables");
    creater_lay->setObjectName("creator_lay");
    //QFrame * frame = new QFrame();
    //frame->setObjectName("frame");

    connect(&_tableCreator, SIGNAL(rejected()), this, SLOT(clearCreator()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(clearCreator()));
    connect(closeButton, SIGNAL(clicked()), &_tableCreator, SLOT(close()));
    //connect(&_tableCreator, SIGNAL(finished()), this, SLOT(clearCreator()));
    _tableCreator.setLayout(creater_lay);


    QList<QPair<QString, int>> tables = _iolog.getUnidentifiedTables();
    for(int iterator_tables = 0, length_tables = tables.length(); iterator_tables < length_tables; iterator_tables++){
       unidTables->addItem(tables.at(iterator_tables).first);
    }
    connect(unidTables, SIGNAL(activated(QString)), this, SLOT(paintCreator(QString)));

    creater_lay->addWidget(unidTables, 0, 0, 1, 2);
    //creater_lay->addWidget(frame, 1, 0, 1, 2);
    creater_lay->addWidget(closeButton, 2, 0, 1, 2);



    _tableCreator.show();

    _tableCreatorWidgets.append(creater_lay);
    _tableCreatorWidgets.append(closeButton);
    _tableCreatorWidgets.append(unidTables);
    //_tableCreatorWidgets.append(frame);
    //5 элементов
}

void Interface::clearCreator()
{
    while(!_tableCreatorWidgets.isEmpty()){
        delete _tableCreatorWidgets.last();
        _tableCreatorWidgets.pop_back();
    }
    _tableCreator.hide();
    _tableCreator.close();
    _menuActions.at(4)->setEnabled(true);
}

void Interface::paintCreator(QString nameTable)
{
    //предварительная чистка
    if(_tableCreatorWidgets.length() > 3){
        for(int i = 0, l = _tableCreatorWidgets.length(); i<l;i++){
            if(_tableCreatorWidgets[i]->objectName() == "creator_lay"){
                QGridLayout * temp = (QGridLayout*)_tableCreatorWidgets[i];
                //QScrollArea * old_frame = temp->findChild<QScrollArea*>("frame");
                QFrame * old_frame = temp->findChild<QFrame*>("frame");
                temp->removeWidget(old_frame);

            }
        }
        //qDebug() << _tableCreatorWidgets.length();
        while(_tableCreatorWidgets.length() > 3){

            QObject * temp =  _tableCreatorWidgets.last();
            _tableCreatorWidgets.pop_back();
            delete temp;
        }
    }

    //поиск таблицы и количества генерируемых столюбцов
    QList<QPair<QString, int>> tables = _iolog.getUnidentifiedTables();
    int number_generate = 0;
    for(int iterator_tables = 0, length_tables = tables.length(); iterator_tables < length_tables; iterator_tables++){
        if(tables.at(iterator_tables).first == nameTable){
            number_generate = tables.at(iterator_tables).second;
            break;
        }
    }

    //QScrollArea * area = new QScrollArea();

    QFrame * frame = new QFrame();
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //area->setWidget(frame);
    frame->show();
    //area->setGeometry(0,0,100,100);

    //frame->set
    frame->setObjectName("frame");
    QGridLayout * base = (QGridLayout*)_tableCreatorWidgets.first();
    base->addWidget(frame, 1, 0, 1, 2);

    QGridLayout * frame_lay = new QGridLayout();
    QLabel * info = new QLabel("Введите желаемое имя столбца и выберите его тип данных");

    info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frame->setLayout(frame_lay);
    //frame_lay->setSizeConstraint(QFormLayout::SetFixedSize);

    frame_lay->addWidget(info, 0,0,1,3);
    //_tableCreatorWidgets.append(area);
    _tableCreatorWidgets.append(frame);
    _tableCreatorWidgets.append(frame_lay);
    _tableCreatorWidgets.append(info);
    //генерирвоание столбцов
    for(int iterator_columns = 0; iterator_columns < number_generate; iterator_columns++){
        QLabel * label = new QLabel(QString::number(iterator_columns+1) + ". ");
        QLineEdit * edit = new QLineEdit(); edit->setObjectName("line_edit");
        QComboBox * dataType = new QComboBox();  dataType->setObjectName("data_type");
        dataType->addItem("TEXT");
        dataType->addItem("INTEGER");
        dataType->activated("TEXT");
        frame_lay->addWidget(label, iterator_columns+1, 0, 1, 1);
        frame_lay->addWidget(edit, iterator_columns+1, 1, 1, 1);
        frame_lay->addWidget(dataType, iterator_columns+1, 2, 1, 1);
        _tableCreatorWidgets.append(label);
        _tableCreatorWidgets.append(edit);
        _tableCreatorWidgets.append(dataType);

    }


    QPushButton * continueButton = new QPushButton("Сохранить");
    frame_lay->addWidget(continueButton, frame_lay->rowCount(), 0, 1, 3);
    _tableCreatorWidgets.append(continueButton);
    connect(continueButton, SIGNAL(clicked()), this, SLOT(applyCreator())); //может из-за этого рантайм быть
//    QSpacerItem * spacer = new QSpacerItem();
//    frame_lay->addWidget(spacer, frame_lay->rowCount(), 0, 1, 3);
//    _tableCreatorWidgets.append(spacer);
}

void Interface::applyCreator()
{

    QString nameTable;
    QList<QPair<QString,QString>> columns;
    for(int iterator_creator = 0, length_creator = _tableCreatorWidgets.length(); iterator_creator < length_creator; iterator_creator++){
        if(_tableCreatorWidgets.at(iterator_creator)->objectName() == "tables"){
            QComboBox * combo = (QComboBox *)_tableCreatorWidgets.at(iterator_creator);
            nameTable = combo->currentText();
        }

        if(_tableCreatorWidgets.at(iterator_creator)->objectName() == "line_edit"){
            QLineEdit * edit = (QLineEdit *)_tableCreatorWidgets.at(iterator_creator);
            columns.push_back(QPair<QString,QString>());
            columns.last().first = edit->text();
        }
        if(_tableCreatorWidgets.at(iterator_creator)->objectName() == "data_type"){
            QComboBox * type = (QComboBox *)_tableCreatorWidgets.at(iterator_creator);
            columns.last().second = type->currentText();
        }
    }
    _iolog.writeSpec(nameTable, columns);
    clearCreator();
}




