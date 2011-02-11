#ifndef LDAPHELPER_H
#define LDAPHELPER_H

#include <ldap.h>
#include <QString>

class LdapHelper {
public:
    LdapHelper(QString, QString, QString, int = LDAPS_PORT);
    ~LdapHelper();

    bool connect();
    void disconnect();

private:
    LDAP *ld;
    QString address;
    QString userDN;
    QString password;
    int port;
    bool connected;
};

#endif // LDAPHELPER_H
