#ifndef LDAPHELPER_H
#define LDAPHELPER_H

#include <ldap.h>
#include <QString>

class LdapHelper {
public:
    LdapHelper(QString, QString, QString);
    ~LdapHelper();

    bool connect();
    void disconnect();
    QString getUserFromIButton(QString);
    QString getLastError();

private:
    LDAP *ld;
    QString address;
    QString userDN;
    QString password;
    QString errString;
    bool connected;
};

#endif // LDAPHELPER_H
