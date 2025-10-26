#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QVector>
#include <QDebug>
#include <QTableWidget>
#include <QDateTime>        // время
#include "user.h"
#include "message.h"

class DataBase : public QObject
{
    Q_OBJECT

public:
    explicit DataBase(QObject *parent = nullptr);
    ~DataBase();
    bool connect();                         // проверка подключения
    QVector<User> getUsers();               // получение пользователей
    QVector<Message> getMessages();         // получение сообщений
    void updateUserStatus(const QString &username, UserStatus status);  // обновление статуса пользователя
    bool setUserBanned(const QString& username);    // бан пользователя
    bool setUserUnBann(const QString& username);    // разбан пользователя
signals:
    void logMessage(const QString &text);   // вывод логов подключения

private:
    QVector<User> _users;                   // пользователи
    QVector<Message> _messages;             // сообщения
    QSqlDatabase db;                        // объект БД
};

#endif // DATABASE_H
