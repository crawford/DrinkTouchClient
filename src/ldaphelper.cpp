#include "ldaphelper.h"
#include <QMessageBox>
#include <QDebug>

LdapHelper::LdapHelper(QString userDN, QString password, QString address, int port) {
    this->address = address;
    this->port = port;
    this->connected = false;
    this->userDN = userDN;
    this->password = password;
}

LdapHelper::~LdapHelper() {
    if (connected) {
        disconnect();
    }
}

bool LdapHelper::connect() {
    //QString connectionString = QString("%1:%2").arg(address).arg(port);
    QString connectionString = address;
    int error = ldap_initialize(&ld, connectionString.toAscii().data());
    if (error != LDAP_SUCCESS) {
        QMessageBox::critical(0, "openLDAP", QString("Failed to initialize (error: %1)").arg(error));
        return false;
    }
    qDebug() << "Initialized " << connectionString << error;

    // set the LDAP version to be 3
    int version = LDAP_VERSION3;
    error = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    if (error != LDAP_OPT_SUCCESS)
    {
        QMessageBox::critical(0, "openLDAP", QString("Failed to set options (error: %1)").arg(error));
        return false;
    }
    qDebug() << "Set options" << connectionString << error;

    //error = ldap_simple_bind_s(ld, userDN.toAscii().data(), password.toAscii().data());
    struct berval cred;
    cred.bv_len = password.size();
    cred.bv_val = password.toAscii().data();

    struct berval *serverCred = NULL;

    error = ldap_sasl_bind_s(ld, userDN.toAscii().data(), LDAP_SASL_SIMPLE , &cred, NULL, NULL, &serverCred);
    if (error != LDAP_SUCCESS ) {
        QMessageBox::critical(0, "openLDAP", QString("Failed to bind (error: %1)").arg(error));
        return false;
    }
    qDebug() << "Bound to" << connectionString << error;

    this->connected = true;
    return true;
}

void LdapHelper::disconnect() {
    ldap_unbind_s(ld);
}
