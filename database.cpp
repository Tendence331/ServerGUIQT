#include "database.h"


DataBase::DataBase(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QPSQL");    // устанавливаем драйвер
    db.setHostName("localhost");                // хост
    db.setDatabaseName("Chat");                 // имя БД
    db.setUserName("postgres");                 // имя пользователя БД
    db.setPassword("passw");                     // пароль
}

DataBase::~DataBase()
{

}

bool DataBase::connect()
{
    if(!db.open())
    {
        emit logMessage("Ошибка подключения к базе данных: " + db.lastError().text());  // сигнал об ошибке
        return false;
    }
    emit logMessage("Успешное подключение в базе данных");      // сигнал об успехе
    return true;
}

QVector<User> DataBase::getUsers()  // получение списка пользователей из БД
{
    _users.clear();
    QSqlQuery query("SELECT id, username, status, banned FROM users");       // запрос на получение пользователей
    while(query.next())
    {
        User u;
        u._id = query.value(0).toInt();
        u._username = query.value(1).toString();
        QString status = query.value(2).toString();
        u._status = stringToUserStatus(status);
        u._baned = query.value(3).toBool();
        _users.push_back(u);
    }
    return _users;
}

QVector<Message> DataBase::getMessages()    // получение списка сообщений из БД
{
    _messages.clear();
    QSqlQuery query("SELECT sender, receiver, text, time FROM messages");
    while (query.next()) {
        Message m;
        m._sender = query.value(0).toString().toStdString();
        m._receiver = query.value(1).toString().toStdString();
        m._text = query.value(2).toString().toStdString();
        m._time = query.value(3).toDateTime().toString("yyyy-MM-dd hh:mm").toStdString();
        _messages.push_back(m);
    }
    return _messages;
}

void DataBase::updateUserStatus(const QString &username, UserStatus status) // обновление статуса пользователя
{
    QSqlQuery query;
    query.prepare("UPDATE users SET status = :status WHERE username = :username");
    query.bindValue(":status", userStatusToString(status));
    query.bindValue(":username", username);
    query.exec();
}


bool DataBase::setUserBanned(const QString &username)   // бан пользователя
{
    QSqlQuery query;
    query.prepare("UPDATE users SET banned = true WHERE username = :username");
    query.bindValue(":username", username);
    query.exec();
    return true;
}

bool DataBase::setUserUnBann(const QString &username)   // разбан пользователя
{
    QSqlQuery query;
    query.prepare("UPDATE users SET banned = false WHERE username = :username");
    query.bindValue(":username", username);
    query.exec();
    return true;
}


