#ifndef USER_H
#define USER_H

#include <string>
#include "sha1.h"
#include <QString>
#include <userstatus.h>
using namespace std;

class User
{
public:
    static int userCounter;
    int _id;
    QString _login;
    QString _username;
    Hash _passwordHash;
    bool _baned;
    UserStatus _status;
public:
    User();
    User(QString username, Hash passHash);
    int checklogin(const string& login, const Hash& passHash) const;
    QString& getuserName();
    int getUserID() const;
};

#endif // USER_H
