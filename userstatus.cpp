#include "userstatus.h"

QString userStatusToString(UserStatus status)
{
    switch (status) {
    case UserStatus::Online:
        return "Онлайн";
    case UserStatus::Offline:
        return "Оффлайн";
    case UserStatus::Disconnected:
        return "Отключен";
    case UserStatus::Banned:
        return "Заблокирован";
    case UserStatus::Disbaned:
        return "Разблокирован";
    }
    return "Неизвестно";
}

UserStatus stringToUserStatus(const QString &status)
{
    if(status == "Онлайн") return UserStatus::Online;
    if(status == "Оффлайн") return UserStatus::Offline;
    if(status == "Отключен") return UserStatus::Disconnected;
    if(status == "Заблокирован") return UserStatus::Banned;
    if(status == "Разблокирован") return UserStatus::Disbaned;
    return UserStatus::Offline;
}
