#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), tcpServer(new QTcpServer(this))
{
    ui->setupUi(this);
    // подключение сигнала логов, для вывода в логБраузер
    connect(&db, &DataBase::logMessage, this, [this](const QString& text){
        ui->logBrowser->append(text);
    });
    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);
    db.connect();   // подключение к БД
    setUpTable();   // установка таблицы
    loadUsers();    // загрузка пользователей
    loadMessages(); // загрузка сообщений
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadUsers()
{
    auto users = db.getUsers();                 // получаем пользователей из БД
    ui->userTableWidget->setRowCount(0);        // обнуляем таблицу
    for (int i = 0; i < users.size(); ++i)
    {
        // вставляем в таблицу данные
        ui->userTableWidget->insertRow(i);
        ui->userTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(users[i]._id)));
        ui->userTableWidget->setItem(i, 1, new QTableWidgetItem(users[i]._username));
        ui->userTableWidget->setItem(i, 2, new QTableWidgetItem(userStatusToString(users[i]._status)));
        ui->userTableWidget->setItem(i, 3, new QTableWidgetItem(users[i]._baned ? "Да" : "Нет"));
    }
    ui->logBrowser->append("[SERVER-LOG]: Загружен список пользователей (" + QString::number(users.size()) + ")");
}

// кнопка обновить
void MainWindow::on_refreshButton_clicked()
{
    setUpTable();   // устанавливает таблицу
    loadUsers();    // загружает список пользователей
    loadMessages(); // обвновляет сообщения
}


void MainWindow::on_banButton_clicked()     // кнопка бана
{
    int row = ui->userTableWidget->currentRow();
    if(row < 0) return;
    QString username = ui->userTableWidget->item(row, 1)->text();
    if(db.setUserBanned(username))  // баним пользователя в БД
    {
        db.updateUserStatus(username, UserStatus::Banned);
        // кикаем пользователя если он онлайн
        if(userSockets.contains(username))
        {
            QTcpSocket* userSock = userSockets[username];   // записываем сокет пользователя
            userSock->disconnectFromHost();                 // закрываем соединение
            userSockets.remove(username);                   // удаляем из контенера
            clients.removeAll(userSock);                    // удаляем из списка
            db.updateUserStatus(username, UserStatus::Banned);
            ui->logBrowser->append("[SERVER-LOG]: Пользователь " + username + " забанен и отключен от сервера.");
        }
        else
        {
            ui->logBrowser->append("[SERVER-LOG]: Пользователь " + username + " забанен");
        }
        loadUsers();    // обновляем таблицу
    }
}


void MainWindow::on_kickButton_clicked()    // кнопка кик
{
    int row = ui->userTableWidget->currentRow();
    if(row < 0) return;
    QString username = ui->userTableWidget->item(row, 1)->text();

    if(userSockets.contains(username))
    {
        QTcpSocket* userSock = userSockets[username];
        userSock->disconnectFromHost();         // закрываем соединение
        userSockets.remove(username);           // удаляем из контенера
        clients.removeAll(userSock);            // удаляем из списка
        db.updateUserStatus(username, UserStatus::Offline);
        ui->logBrowser->append("[SERVER-LOG]: Пользователь " + username + " был кикнут.");
        loadUsers();
    }
    else
    {
        ui->logBrowser->append("[SERVER-LOG]: Пользователь " + username + " не в сети");
    }
}



void MainWindow::setUpTable()   // настройки таблицы
{
    // Установка кол-ва столбцов
    ui->userTableWidget->setColumnCount(4);
    // Установка заголовков
    QStringList headers;
    headers << "ID" << "Логин" << "Статус" << "Блокировка";
    ui->userTableWidget->setHorizontalHeaderLabels(headers);
    ui->userTableWidget->setVisible(true);
}

void MainWindow::loadMessages()     // загрузка сообщений в текстовый браузер
{
    ui->messageTextBrowser->clear();
    auto message = db.getMessages();
    for (const auto &m : message)
    {
        QString msgLine = QString("<b>%1</b> → <b>%2</b> [%3]: %4").arg(m._sender).arg(m._receiver).arg(m._time).arg(m._text);
        ui->messageTextBrowser->append(msgLine);
    }
    ui->logBrowser->append("[SERVER-LOG]: Загружен список сообщений (" + QString::number(message.size()) + ")");
}



void MainWindow::onNewConnection()      // подключение новых клиентов по сокету
{
    while(tcpServer->hasPendingConnections())
    {
        QTcpSocket* clientSocket = tcpServer->nextPendingConnection();
        clients.append(clientSocket);
        ui->logBrowser->append("[SERVER-LOG]: Новое подключение: " + clientSocket->peerAddress().toString());
        connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &MainWindow::deleteLater);
    }
}


void MainWindow::onClientDisconnected() // отключение клиента
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    QString username;
    for (auto it = userSockets.begin(); it != userSockets.end(); ++it)
    {
        if(it.value() == client)
        {
            username = it.key();
            userSockets.erase(it);
            db.updateUserStatus(username, UserStatus::Offline);
            ui->logBrowser->append("[SERVER-LOG]: Клиент отключился: " + client->peerAddress().toString());
            break;
        }
    }
    clients.removeAll(client);
    client->deleteLater();
    loadUsers();
}


void MainWindow::on_actionConnectTCP_triggered()
{
    QString ipStr = "127.0.0.1";
    quint16 port = 5000;
    QHostAddress address;
    if(ipStr.isEmpty() || ipStr == "0.0.0.0")   // если ip поле пустое или поле равно 0.0.0.0
    {
        address = QHostAddress::Any;        // слушать на всех интерфейсах
    }
    else if(!address.setAddress(ipStr))
    {
        ui->logBrowser->append("[SERVER-LOG]: Ошибка некорректный IP-адрес!");
        return;
    }
    if(!tcpServer->listen(address, port))
    {
        ui->logBrowser->append("[SERVER-LOG]: Ошибка запуска сервера: " + tcpServer->errorString());
        return;
    }
    ui->logBrowser->append("[SERVER-LOG]: Сервер запущен на IP: " + ipStr + ", порт: " + QString::number(port));
}


void MainWindow::on_actionDisconnectTCP_triggered()
{
    if(!tcpServer->isListening())
    {
        ui->logBrowser->append("[SERVER-LOG]: Сервер уже остановлен");
        return;
    }
    // закрываем все активные соединения
    for(QTcpSocket *client : clients)
    {
        if(client->isOpen())
        {
            client->disconnectFromHost();
            client->close();
        }
        client->deleteLater();
    }
    clients.clear();
    userSockets.clear();
    // останавливаем сам сервер
    tcpServer->close();
    ui->logBrowser->append("[SERVER-LOG]: Сервер остановлен");
}



void MainWindow::onReadyRead()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = clientSocket->readAll();      // читаем все данные
    QString message = QString::fromUtf8(data);
    if(message.startsWith("LOGIN:"))
    {
        QString username = message.mid(6).trimmed();
        // запоминаем сокет
        userSockets[username] = clientSocket;
        db.updateUserStatus(username, UserStatus::Online);
        ui->logBrowser->append("[SERVER-LOG]: Зарегистрирован сокет для " + username);
        return;
    }
    for(QTcpSocket* s : clients)
    {
        if(s != clientSocket && s->state() == QAbstractSocket::ConnectedState)
        {
            s->write(message.toUtf8());
        }
    }
}


void MainWindow::on_unBanButton_clicked() // разблокировка пользователя
{
    int row = ui->userTableWidget->currentRow();
    if(row < 0) return;
    QString username = ui->userTableWidget->item(row, 1)->text();
    if(db.setUserUnBann(username))
    {
        ui->logBrowser->append("[SERVER-LOG]: Пользователь " + username + " был разблокирован.");
        db.updateUserStatus(username, UserStatus::Disbaned);
        loadUsers();
    }
}

