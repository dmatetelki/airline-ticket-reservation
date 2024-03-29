#include "sqldatabase.h"

#include <QSqlQuery>
#include <QSqlRecord>

#include <QVariant>
#include <QDebug>

SqlDataBase::SqlDataBase(const QString type,
                         const QString host,
                         const QString pass) :
    m_type(type),
    m_host(host),
    m_pass(pass),
    m_db(0),
    m_connectionIsOpen(false)
{
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase(type));
    m_db->setHostName(m_host);
    m_db->setDatabaseName("airlinedb");
    m_db->setUserName("airline");
    m_db->setPassword(m_pass);
}

SqlDataBase::~SqlDataBase()
{
    if (m_connectionIsOpen) {
        m_db->close();
    }
}

bool SqlDataBase::writeData(const QString id, const bool taken)
{
    QString text;
    bool dummy;

    /// @note this would be the best, but the query has somwhere a syntax error
    /*
    QString text(QString("IF EXISTS (SELECT * FROM seats WHERE id='%1')"
                            "UPDATE seats SET taken=%2 WHERE id='%1' "
                        "ELSE "
                            "INSERT INTO seats VALUES ('%1', %2)").
                        arg(id).arg(taken));
    QSqlQuery query;

    // if DB support transactions, let it be atomic
    if (m_transactionsAreSupported) m_db->transaction();
    if (!query.exec(text)) {
        if (m_transactionsAreSupported) m_db->rollback();
        emit notification("SQL - write query falied");
        return false;
    }
    if (m_transactionsAreSupported) m_db->commit();
    */

    /// @todo make read&write atomic with transactions
    if (readData(id,dummy)) {
        text = QString("UPDATE seats SET taken=%2 WHERE id='%1'").arg(id).arg(taken);
    } else {
        text = QString("INSERT INTO seats (id, taken) VALUES ('%1', %2)").arg(id).arg(taken);
    }

    QSqlQuery query;
    if (!query.exec(text)) {
        emit notification("SQL - write query falied");
        return false;
    }

    return true;
}

bool SqlDataBase::readData(const QString id, bool &taken)
{
    if (!m_connectionIsOpen) {
        if (!m_db->open()) {
            emit notification("Couldn't connect to DB");
            return false;
        }
        m_connectionIsOpen = true;
    }

    QString text(QString("SELECT taken FROM seats WHERE id='%1'").arg(id));
    QSqlQuery query;
    if (!query.exec(text)) {
        emit notification("SQL - read query falied");
        return false;
    }

    QSqlRecord rec = query.record();
    int takenCol = rec.indexOf("taken");
    if (query.next()) {
        taken = query.value(takenCol).toInt();
        return true;
    }

    // not found
    return false;
}

void SqlDataBase::databaseModified()
{
    /// @todo connect to event
    /// @todo calculate diff

    // emit dataChanged(id);
}
