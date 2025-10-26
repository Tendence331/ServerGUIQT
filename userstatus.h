#ifndef USERSTATUS_H
#define USERSTATUS_H

#include <QString>

enum UserStatus
{
    Offline,
    Online,
    Disconnected,
    Banned,
    Disbaned
};

QString userStatusToString(UserStatus status);          // статус подьзователя из Enum в строку
UserStatus stringToUserStatus(const QString &status);   // статус пользователя из строки в Enum

#endif // USERSTATUS_H
