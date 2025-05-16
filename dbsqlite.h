#ifndef DBSQLITE_H
#define DBSQLITE_H

#include <QObject>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QDebug>
#include<QMessageBox>

class DbSqlite : public QObject
{
    Q_OBJECT
public:
    explicit DbSqlite(QObject *parent = nullptr);
    bool init();
    static DbSqlite &getinstance();
    bool ClearTable();
    bool ShowTableWeather();
    bool SaveTeaDb(const QString &City,const QString &Calendar
                   ,const double &Avg,const double &max,const double &min);
    ~DbSqlite();
signals:

private:

    QSqlDatabase m_sqldatabase;
};

#endif // DBSQLITE_H
