#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"
#include <QMap>
#include <QTcpServer>       // слушает определенный порт и принимает новые подключения
#include <QTcpSocket>       // используется для отправки и приема данных
#include "userstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadUsers();                   // обновить таблицу пользователей БД
    void setUpTable();                  // настройки таблицы БД
    void loadMessages();                // обновить список сообщений БД
private slots:
    void on_refreshButton_clicked();    // обновить данные из БД
    void on_banButton_clicked();        // забанить пользователя БД
    void on_kickButton_clicked();       // кикнуть пользователя БД
    void on_unBanButton_clicked();      // разблокировать пользователя
    void onNewConnection();             // когда подключается новый клиент
    void onReadyRead();                 // чтение данных от клиента
    void onClientDisconnected();        // когда клиент отключился
    void on_actionConnectTCP_triggered();   // подключение TCP
    void on_actionDisconnectTCP_triggered();// отключение TCP



private:
    Ui::MainWindow *ui;
    DataBase db;                        // База данных
    QTcpServer* tcpServer;              // Сервер
    QList<QTcpSocket*> clients;         // список подключившихся клиентов
    QMap<QString, QTcpSocket*> userSockets;     // сокеты пользователей
};
#endif // MAINWINDOW_H
