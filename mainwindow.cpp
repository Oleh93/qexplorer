#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "archiver.h"
#include "QDebug"


#if defined(_WIN32)
QString mPath = "C:\\";
#endif
#if defined(unix) || defined(__unix__) || defined(__unix)
QString mPath = "/";
#endif
#if defined(__APPLE__)
QString mPath = "/";
#endif


QModelIndex chosenFile;
QModelIndex copiedFile;
QList<QModelIndex> chosenFiles;
QList<QModelIndex> copiedFiles;
bool to_cut = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model_2 = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath(mPath);
    model_2->setFilter(QDir::QDir::AllEntries);
    model_2->setRootPath(mPath);
    ui->lineEdit_1->setText(mPath);
    ui->lineEdit_2->setText(mPath);
    ui->listView_1->setModel(model);
    ui->listView_2->setModel(model_2);

    QModelIndex idx = model->index(model->rootPath());
    QModelIndex idx_2 = model_2->index(model_2->rootPath());
    ui->listView_1->setRootIndex(idx);
    ui->listView_2->setRootIndex(idx_2);
    ui->search_1->setVisible(false);
    ui->search_2->setVisible(false);
    connect(ui->listView_1, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_listView_1_doubleClicked(QModelIndex)));
    connect(ui->listView_2, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_listView_1_doubleClicked(QModelIndex)));
    connect(ui->listView_1, SIGNAL(clicked(QModelIndex)), this, SLOT(click(QModelIndex)));
    connect(ui->listView_2, SIGNAL(clicked(QModelIndex)), this, SLOT(click(QModelIndex)));
    connect(ui->listView_2, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));
    connect(ui->listView_1, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));
    connect(ui->lineEdit_1, SIGNAL(returnPressed()), SLOT(lineEditEnter()));
    connect(ui->lineEdit_2, SIGNAL(returnPressed()), SLOT(lineEditEnter()));
    connect(ui->search_button_1, SIGNAL (released()),this, SLOT (show_hide_search_1()));
    connect(ui->search_button_2, SIGNAL (released()),this, SLOT (show_hide_search_2()));
    connect(ui->search_1, SIGNAL(returnPressed()), SLOT(searchEnter()));
    connect(ui->search_2, SIGNAL(returnPressed()), SLOT(searchEnter()));
    ui->listView_1->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView_2->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView_1->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listView_2->setSelectionMode(QAbstractItemView::ExtendedSelection);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this, SLOT(copy_file()));
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close_search()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::click(const QModelIndex &index){
    chosenFile = index;
}

void MainWindow::on_listView_1_doubleClicked(const QModelIndex &index)
{
    this->setCursor(QCursor(Qt::WaitCursor));
    QListView* listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.isFile()) {
        QDesktopServices::openUrl(QUrl(fileInfo.absoluteFilePath().prepend("file:///")));
        this->setCursor(QCursor(Qt::ArrowCursor));
        return;
    }
    if(listView == ui->listView_1){
        ui->lineEdit_1->setText(fileInfo.absoluteFilePath());
        if (fileInfo.fileName() == ".."){
            QDir dir = fileInfo.dir();
            dir.cdUp();
            listView->setRootIndex(model->index(dir.absolutePath()));
        }else if (fileInfo.fileName() == "."){
            listView->setRootIndex(model->index(""));
        }
        else if (fileInfo.isDir()){
            listView->setRootIndex(index);
        }
    }else{
        ui->lineEdit_2->setText(fileInfo.absoluteFilePath());
        if (fileInfo.fileName() == ".."){
            QDir dir = fileInfo.dir();
            dir.cdUp();
            listView->setRootIndex(model_2->index(dir.absolutePath()));
        }else if (fileInfo.fileName() == "."){
            listView->setRootIndex(model_2->index(""));
        }
        else if (fileInfo.isDir()){
            listView->setRootIndex(index);
        }
    }

    this->setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::show_hide_search_1(){
    if(ui->search_1->isVisible()){
        ui->search_1->setVisible(false);
    } else{
        ui->search_1->setVisible(true);
    }
}

void MainWindow::show_hide_search_2(){
    if(ui->search_2->isVisible()){
        ui->search_2->setVisible(false);
    } else{
        ui->search_2->setVisible(true);
    }
}

void MainWindow::delete_file(){
    if(chosenFiles.size()>=2){
        QMessageBox::StandardButton reply;
        QString question = "Are you sure you want to delete " + QString::number(chosenFiles.size()) +
                " files?";
        reply = QMessageBox::question(this, "", question,
                                      QMessageBox::Yes|QMessageBox::No);
        for(auto &c_file: chosenFiles){
            if(model->fileInfo(c_file).isDir()){
                QDir dir(model->fileInfo(c_file).absoluteFilePath());
                if (reply == QMessageBox::Yes) {
                    this->setCursor(QCursor(Qt::WaitCursor));
                    QFileInfo f(dir.absolutePath());
                    QString own = f.owner();
                    if (f.permission(QFile::WriteUser) && own != "root"){
                        dir.removeRecursively();
                    }else{
                        QMessageBox messageBox;
                        messageBox.setText("No permission");
                        messageBox.setFixedSize(500,200);
                        messageBox.exec();
                    }

                    this->setCursor(QCursor(Qt::ArrowCursor));
                }
            }else{
                QFile file(model->fileInfo(c_file).absoluteFilePath());
                if (reply == QMessageBox::Yes) {
                    this->setCursor(QCursor(Qt::WaitCursor));
                    QFileInfo f(file);
                    QString own = f.owner();
                    if (f.permission(QFile::WriteUser) && own != "root"){
                        file.remove();
                    }else{
                        QMessageBox messageBox;
                        messageBox.setText("No permission");
                        messageBox.setFixedSize(500,200);
                        messageBox.exec();
                    }
                    this->setCursor(QCursor(Qt::ArrowCursor));
                }
            }
        }
    }
    else{
        QMessageBox::StandardButton reply;
        QString question = "Are you sure you want to delete ";
        question.append(model->fileInfo(chosenFile).baseName());
        if(!(model->fileInfo(chosenFile).completeSuffix() == "")){
            question.append(".");
        }
        question.append(model->fileInfo(chosenFile).completeSuffix());
        question.append("?");
        reply = QMessageBox::question(this, "", question,
                                      QMessageBox::Yes|QMessageBox::No);
        if(model->fileInfo(chosenFile).isDir()){
            QDir dir(model->fileInfo(chosenFile).absoluteFilePath());
            if (reply == QMessageBox::Yes) {
                this->setCursor(QCursor(Qt::WaitCursor));
                QFileInfo f(dir.absolutePath());
                QString own = f.owner();
                if (f.permission(QFile::WriteUser) && own != "root"){
                    dir.removeRecursively();
                }else{
                    QMessageBox messageBox;
                    messageBox.setText("No permission");
                    messageBox.setFixedSize(500,200);
                    messageBox.exec();
                }
                this->setCursor(QCursor(Qt::ArrowCursor));
            }
        }else{
            QFile file(model->fileInfo(chosenFile).absoluteFilePath());
            if (reply == QMessageBox::Yes) {
                this->setCursor(QCursor(Qt::WaitCursor));
                QFileInfo f(file);
                QString own = f.owner();
                if (f.permission(QFile::WriteUser) && own != "root"){
                    file.remove();
                }else{
                    QMessageBox messageBox;
                    messageBox.setText("No permission");
                    messageBox.setFixedSize(500,200);
                    messageBox.exec();
                }
                this->setCursor(QCursor(Qt::ArrowCursor));
            }
        }
    }
    chosenFiles.clear();
}

void MainWindow::rename_file(){
    int counter = 1;
    bool result;
    QFileInfo info = model->fileInfo(chosenFile);
    QString name = info.baseName();
    if(!(info.completeSuffix() == "")){
        name.append(".");
    }
    name.append(info.completeSuffix());
    QString text;
    if (info.permission(QFile::WriteUser)){
        text = QInputDialog::getText(this, tr(""),
                                             tr("New name:"), QLineEdit::Normal,
                                             name, &result);
    } else
    {
        QMessageBox msg;
        msg.setText("No permission");
        msg.setFixedSize(500,200);
        msg.exec();
    }

    if(result){
        if(info.isDir()){
            QDir dir(info.absoluteFilePath());
            QString new_path = info.absolutePath();
            new_path.append("/");
            new_path.append(text);
            while(!dir.rename(info.absoluteFilePath(), new_path)){
                new_path = info.absolutePath();
                new_path.append("/");
                QStringList text_list = text.split('.');
                text_list[0].append("(");
                text_list[0].append(QString::number(counter));
                text_list[0].append(")");
                new_path.append(text_list.join("."));
                counter++;
            }
        }else{
            QFile file(info.absoluteFilePath());
            QString new_path = info.absolutePath();
            new_path.append("/");
            new_path.append(text);
            while(!file.rename(new_path)){
                new_path = info.absolutePath();
                new_path.append("/");
                QStringList text_list = text.split('.');
                text_list[0].append("(");
                text_list[0].append(QString::number(counter));
                text_list[0].append(")");
                new_path.append(text_list.join("."));
                counter++;
            }
        }
    }
}

void MainWindow::get_properties(){
        this->setCursor(QCursor(Qt::WaitCursor));
        QFileInfo info = model->fileInfo(chosenFile);
        QMessageBox properties_window(this);
        properties_window.setWindowTitle("Properties");
        properties_window.setStandardButtons(QMessageBox::Close);
        QString properties_text;
        QStringList properties_list = {"Name: ", "Type: ", "Size: ", "Parent folder: ",
                                       "Group: ", "Owner: ", "Created: ", "Last modified: "};
        properties_text.append(properties_list[0]);
        QString name = info.baseName();
        if(!(info.completeSuffix() == "")){
            name.append(".");
        }
        name.append(info.completeSuffix());
        properties_text.append(name);
        properties_text.append("\n");
        QString type;
        properties_text.append(properties_list[1]);
        if(info.isDir()){
            type = "directory";
        }else if(info.isExecutable()){
            type = "executable";
        }else if(info.isSymLink()){
            type = "symbolic link";
        }else if(info.isBundle()){
            type = "bundle";
        }else{
            type = "file";
        }
        properties_text.append(type);
        properties_text.append("\n");
        properties_text.append(properties_list[2]);
        qint64 size;
        if(info.isDir()){
            size = dirSize(info.absoluteFilePath());
        }else{
            size = info.size()/1000;
        }
        properties_text.append(QString::number(size));
        properties_text.append(" kB");
        properties_text.append("\n");
        properties_text.append(properties_list[3]);
        properties_text.append(info.absolutePath());
        properties_text.append("\n");
        properties_text.append(properties_list[4]);
        properties_text.append(info.group());
        properties_text.append("\n");
        properties_text.append(properties_list[5]);
        properties_text.append(info.owner());
        properties_text.append("\n");
        properties_text.append(properties_list[6]);
        properties_text.append(info.lastModified().toString(Qt::SystemLocaleLongDate));
        properties_text.append("\n");
        properties_text.append(properties_list[7]);
        properties_text.append(info.created().toString(Qt::SystemLocaleLongDate));
        properties_window.setText(properties_text);
        this->setCursor(QCursor(Qt::ArrowCursor));
        properties_window.exec();
    }
//    QFileInfo info = model->fileInfo(chosenFile);
//    QMessageBox properties_window(this);
//    properties_window.setWindowTitle("Properties");
//    properties_window.setStandardButtons(QMessageBox::Close);
//    properties_window.setCursor(QCursor(Qt::WaitCursor));
//    properties_window.exec();
//    QString properties_text;
//    QStringList properties_list = {"Name: ", "Type: ", "Size: ", "Parent folder: ",
//                                   "Group: ", "Owner: ", "Created: ", "Last modified: "};
//    properties_text.append(properties_list[0]);
//    QString name = info.baseName();
//    if(!(info.completeSuffix() == "")){
//        name.append(".");
//    }
//    name.append(info.completeSuffix());
//    properties_text.append(name);
//    properties_text.append("\n");
//    QString type;
//    properties_text.append(properties_list[1]);
//    if(info.isDir()){
//        type = "directory";
//    }else if(info.isExecutable()){
//        type = "executable";
//    }else if(info.isSymLink()){
//        type = "symbolic link";
//    }else if(info.isBundle()){
//        type = "bundle";
//    }else{
//        type = "file";
//    }
//    properties_text.append(type);
//    properties_text.append("\n");
//    properties_text.append(properties_list[2]);
//    qint64 size;
//    if(info.isDir()){
//        size = dirSize(info.absoluteFilePath());
//    }else{
//        size = info.size()/1000;
//    }
//    properties_text.append(QString::number(size));
//    properties_text.append(" kB");
//    properties_text.append("\n");
//    properties_text.append(properties_list[3]);
//    properties_text.append(info.absolutePath());
//    properties_text.append("\n");
//    properties_text.append(properties_list[4]);
//    properties_text.append(info.group());
//    properties_text.append("\n");
//    properties_text.append(properties_list[5]);
//    properties_text.append(info.owner());
//    properties_text.append("\n");
//    properties_text.append(properties_list[6]);
//    properties_text.append(info.lastModified().toString(Qt::SystemLocaleLongDate));
//    properties_text.append("\n");
//    properties_text.append(properties_list[7]);
//    properties_text.append(info.created().toString(Qt::SystemLocaleLongDate));
//    properties_window.setText(properties_text);
//    properties_window.setCursor(QCursor(Qt::ArrowCursor));


void MainWindow::copy_file(){
    copiedFiles = chosenFiles;
}

void MainWindow::cut_file(){
    copiedFiles = chosenFiles;
    to_cut = true;
}

void MainWindow::paste_file(){
    this->setCursor(QCursor(Qt::WaitCursor));
    for (auto& c_file: copiedFiles){
        QFileInfo copy_info = model->fileInfo(c_file);
        QFileInfo chosen_info = model->fileInfo(chosenFile);
        if(copy_info.isDir()){
            QDir dir_to_copy(copy_info.absoluteFilePath());
            if(dir_to_copy.exists()){
                QString new_path = chosen_info.absolutePath();
                QString src_path = copy_info.absoluteFilePath();
                new_path.append("/");
                new_path.append(copy_info.baseName());
                if(!copy_info.completeSuffix().isEmpty()){
                    new_path.append(".");
                }
                new_path.append(copy_info.completeSuffix());
                copyPath(src_path, new_path);
            }
        }else{
            QFile file_to_copy(copy_info.absoluteFilePath());
            if(file_to_copy.exists()){
                QString new_path = chosen_info.absolutePath();
                new_path.append("/");
                new_path.append(copy_info.baseName());
                if(copy_info.completeSuffix().size() != 0){
                    new_path.append(".");
                    new_path.append(copy_info.completeSuffix());
                }
                int counter = 1;
                while(!file_to_copy.copy(new_path)){
                    new_path = chosen_info.absolutePath();
                    new_path.append("/");
                    QString text = copy_info.baseName();
                    if(copy_info.completeSuffix().size() != 0){
                        text.append(".");
                        text.append(copy_info.completeSuffix());
                    }
                    QStringList text_list = text.split('.');
                    text_list[0].append("(");
                    text_list[0].append(QString::number(counter));
                    text_list[0].append(")");
                    new_path.append(text_list.join("."));
                    counter++;
                }
            }
        }
        if(to_cut){
            if(model->fileInfo(c_file).isDir()){
                QDir dir(model->fileInfo(c_file).absoluteFilePath());
                this->setCursor(QCursor(Qt::WaitCursor));
                dir.removeRecursively();
                this->setCursor(QCursor(Qt::ArrowCursor));
            }else{
                QFile file(model->fileInfo(c_file).absoluteFilePath());
                this->setCursor(QCursor(Qt::WaitCursor));
                file.remove();
                this->setCursor(QCursor(Qt::ArrowCursor));
            }
        }
    }
    to_cut = false;
    this->setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::create_file(){
    int counter = 1;
    bool result;
    QString text = QInputDialog::getText(this, tr(""),
                                         tr("New file:"), QLineEdit::Normal,
                                         "", &result);
    if(result){
        QString path = model->fileInfo(chosenFile).absolutePath();
        path.append("/");
        path.append(text);
        QFile newFile(path);
        if(newFile.exists()){
            while(true){
                path = model->fileInfo(chosenFile).absolutePath();
                path.append("/");
                QStringList text_list = text.split('.');
                text_list[0].append("(");
                text_list[0].append(QString::number(counter));
                text_list[0].append(")");
                path.append(text_list.join("."));
                QFile newFile(path);
                if(!newFile.exists()){
                    newFile.open(QFile::WriteOnly);
                    break;
                }
                counter++;
            }
        }
        else{
            newFile.open(QFile::WriteOnly);
        }
        newFile.close();
    }
}

void MainWindow::create_folder(){
    int counter = 1;
    bool result;
    QString text = QInputDialog::getText(this, tr(""),
                                         tr("New folder:"), QLineEdit::Normal,
                                         "", &result);
    if(result){
        QString path = model->fileInfo(chosenFile).absolutePath();
        path.append("/");
        path.append(text);
        QDir newFolder(path);
        if(newFolder.exists()){
            while(true){
                path = model->fileInfo(chosenFile).absolutePath();
                path.append("/");
                QStringList text_list = text.split('.');
                text_list[0].append("(");
                text_list[0].append(QString::number(counter));
                text_list[0].append(")");
                path.append(text_list.join("."));
                QDir newFolder(path);
                if(!newFolder.exists()){
                    newFolder.mkpath(".");
                    break;
                }
                counter++;
            }
        }
        else{
            newFolder.mkpath(".");
        }
    }
}

void MainWindow::customMenuRequested(const QPoint &pos){
    QListView* listView = (QListView*)sender();
    QModelIndex index=listView->indexAt(pos);
    QModelIndexList list = listView->selectionModel()->selectedIndexes();
    chosenFiles.clear();
    foreach(const QModelIndex &indexx, list){
        chosenFiles.append(indexx);
    }
    if(!(model->fileInfo(index).completeBaseName() == "." || model->fileInfo(index).completeBaseName() == "")){
        if(chosenFiles.size() >= 2){
            chosenFile = index;
            QMenu *menu=new QMenu(this);
            menu->setObjectName("menu");
            QMenu *create_menu = menu->addMenu("Create");
            QAction *create_folder_action = new QAction("Folder", this);
            QAction *create_file_action = new QAction("File", this);
            create_menu->addAction(create_folder_action);
            create_menu->addAction(create_file_action);
            QAction *copy_action = new QAction("Copy", this);
            menu->addAction(copy_action);
            QAction *cut_action = new QAction("Cut", this);
            menu->addAction(cut_action);
            QAction *paste_action = new QAction("Paste", this);
            menu->addAction(paste_action);
            paste_action->setObjectName("paste_action");
            QAction *compress_action = new QAction("Archive", this);
            menu->addAction(compress_action);
            QAction *delete_action = new QAction("Delete", this);
            menu->addAction(delete_action);
            connect(create_file_action, SIGNAL(triggered()), this, SLOT(create_file()));
            connect(create_folder_action, SIGNAL(triggered()), this, SLOT(create_folder()));
            connect(copy_action, SIGNAL(triggered()), this, SLOT(copy_file()));
            connect(cut_action, SIGNAL(triggered()), this, SLOT(cut_file()));
            connect(paste_action, SIGNAL(triggered()), this, SLOT(paste_file()));
            connect(compress_action, SIGNAL(triggered()), this, SLOT(compress_files()));
            connect(delete_action, SIGNAL(triggered()), this, SLOT(delete_file()));
            menu->popup(listView->viewport()->mapToGlobal(pos));
        }
        else{
            chosenFile = index;
            QFileInfo fileInfo = model->fileInfo(index);
            QMenu *menu=new QMenu(this);
            menu->setObjectName("menu");
            if(fileInfo.isFile()){
                QAction *open_action = new QAction("Open", this);
                menu->addAction(open_action);
                connect(open_action, SIGNAL(triggered()), this, SLOT(open_file()));
            }
            QMenu *create_menu = menu->addMenu("Create");
            QAction *create_folder_action = new QAction("Folder", this);
            QAction *create_file_action = new QAction("File", this);
            create_menu->addAction(create_folder_action);
            create_menu->addAction(create_file_action);
            QAction *copy_action = new QAction("Copy", this);
            menu->addAction(copy_action);
            QAction *cut_action = new QAction("Cut", this);
            menu->addAction(cut_action);
            QAction *paste_action = new QAction("Paste", this);
    //        paste_action->setDisabled(true);
            menu->addAction(paste_action);
            paste_action->setObjectName("paste_action");
            if (is_archive(fileInfo.suffix())){
                QAction *unarchive_action = new QAction("Unarchive", this);
                menu->addAction(unarchive_action);
                connect(unarchive_action, SIGNAL(triggered()), this, SLOT(unarhive()));
            }
            QAction *compress_action = new QAction("Archive", this);
            menu->addAction(compress_action);
            QAction *delete_action = new QAction("Delete", this);
            menu->addAction(delete_action);
            QAction *rename_action = new QAction("Rename", this);
            menu->addAction(rename_action);
            QAction *properties_action = new QAction("Properties", this);
            menu->addAction(properties_action);
            connect(create_file_action, SIGNAL(triggered()), this, SLOT(create_file()));
            connect(create_folder_action, SIGNAL(triggered()), this, SLOT(create_folder()));
            connect(copy_action, SIGNAL(triggered()), this, SLOT(copy_file()));
            connect(cut_action, SIGNAL(triggered()), this, SLOT(cut_file()));
            connect(paste_action, SIGNAL(triggered()), this, SLOT(paste_file()));
            connect(delete_action, SIGNAL(triggered()), this, SLOT(delete_file()));
            connect(rename_action, SIGNAL(triggered()), this, SLOT(rename_file()));
            connect(compress_action, SIGNAL(triggered()), this, SLOT(compress_files()));
            connect(properties_action, SIGNAL(triggered()), this, SLOT(get_properties()));
            menu->popup(listView->viewport()->mapToGlobal(pos));
        }
    }
}

qint64 MainWindow::dirSize(QString dirPath) {
    qint64 KILOBYTE_CONVERSION = 1000;
    qint64 size = 0;
    QDir dir(dirPath);
    QDir::Filters fileFilters = QDir::Files|QDir::System|QDir::Hidden;
    for(QString filePath : dir.entryList(fileFilters)) {
        QFileInfo fi(dir, filePath);
        size+= fi.size();
    }
    QDir::Filters dirFilters = QDir::Dirs|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
    for(QString childDirPath : dir.entryList(dirFilters))
        size+= dirSize(dirPath + QDir::separator() + childDirPath);
    return size/KILOBYTE_CONVERSION;
}

void MainWindow::copyPath(QString src, QString dst){
    QDir dir(src);
    if (! dir.exists())
        return;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyPath(src+ QDir::separator() + d, dst_path);
    }

    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
}

void MainWindow::lineEditEnter(){
    QLineEdit* line = (QLineEdit*)sender();
    QString path = line->text();
    QDir dir(path);
    QModelIndex idx;
    if(line == ui->search_1){
        idx = model->index(path);
    }
    else{
        idx = model_2->index(path);
    }
    if(dir.exists()){
        if(line == ui->lineEdit_1){
            ui->listView_1->setRootIndex(idx);
        }
        else{
            ui->listView_2->setRootIndex(idx);
        }
    }
}

void MainWindow::searchEnter(){
    QLineEdit* line = (QLineEdit*)sender();
    QString item_name = line->text();
    QStringList filters;
    QString request = "*";
    request.append(item_name);
    request.append("*");
    filters << request;
    filters << ".";
    filters << "..";
    if(line == ui->search_1){
        model->setNameFilters(filters);
        model->setNameFilterDisables(false);
        ui->listView_1->setRootIndex(model->index(ui->lineEdit_1->text()));
    } else if(line == ui->search_2){
        model_2->setNameFilters(filters);
        model_2->setNameFilterDisables(false);
        ui->listView_2->setRootIndex(model_2->index(ui->lineEdit_2->text()));
    }
}

void MainWindow::close_search(){
    ui->search_1->setVisible(false);
    ui->search_2->setVisible(false);
    QStringList filters;
    model->setNameFilters(filters);
    model_2->setNameFilters(filters);
}

void MainWindow::unarhive(){
    QFileInfo fileInfo = model->fileInfo(chosenFile);

    if (!(is_archive(fileInfo.suffix()))){
        return;
    }

    try {
        extract(fileInfo.absoluteFilePath().toStdString().c_str(), fileInfo.absolutePath()+"/");
    } catch (std::exception) {
        qDebug("exception!");
    }
}


void MainWindow::open_file(){
    this->setCursor(QCursor(Qt::WaitCursor));
    QFileInfo fileInfo = model->fileInfo(chosenFile);
    QDesktopServices::openUrl(QUrl(fileInfo.absoluteFilePath().prepend("file:///")));
    this->setCursor(QCursor(Qt::ArrowCursor));
}

void MainWindow::compress_files () {
    QStringList paths;
    bool result;

    QString name = model->fileInfo(chosenFiles[0]).absolutePath() + "/Archive.zip";
//    qDebug() << model->fileInfo(chosenFiles[0]).absolutePath();

    QString edited_name = QInputDialog::getText(this, tr("Create archive"),
                                         tr("Achive name:"), QLineEdit::Normal,
                                          name,
                                         &result);
    for (auto& c_file: chosenFiles){
           paths.append(model->fileInfo(c_file).filePath());
    }
    for(auto p: paths){
        qDebug() <<"p:"<< p;
    }

    compress(paths, edited_name, 'z');
    qDebug() << "after";
}

