#include "dbsqlite.h"
#include <QDir>
DbSqlite::DbSqlite(QObject *parent)
    : QObject{parent}
{

}

bool DbSqlite::init()
{
    QString currentPath = QDir::currentPath();
    QDir dir(currentPath);
    // 切换到上级目录
    dir.cdUp();  // 去掉 "build/Desktop_Qt_6_5_3_MSVC2019_64bit-Debug"
    dir.cdUp();  // 去掉 "build"
    QString targetPath = dir.path();
    targetPath=targetPath+"/myweather.db";
    bool a=false;
    m_sqldatabase=QSqlDatabase::addDatabase("QSQLITE");
    m_sqldatabase.setHostName("localhost");
    m_sqldatabase.setDatabaseName(targetPath);
    if(m_sqldatabase.open())
    {
        a=true;
        QSqlQuery query;
        QString sql="SELECT *FROM weather;";
        query.exec(sql);
        while(query.next())
        {
            QString data=QString("%1 %2 %3 %4 %5 %6")
            .arg(query.value(0).toString())
                .arg(query.value(1).toString())
                .arg(query.value(2).toString())
                .arg(query.value(3).toString())
                .arg(query.value(4).toString())
                .arg(query.value(5).toString());
        }
        return a;
    }
    else{

        QMessageBox::warning(NULL,"连接提示","连接失败");
        return a;
    }
    return a;
}

DbSqlite &DbSqlite::getinstance()
{
        DbSqlite instance;
        return instance;
}

bool DbSqlite::ClearTable()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "确认清空", "确定要清空数据库吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    // 如果用户选择“否”，则直接返回
    if (reply == QMessageBox::No) {
        return false;
    }

    // 用户选择“是”，执行清空操作
    QSqlQuery query;
    QString sql = "DELETE FROM weather;";
    QString sql1 = "DELETE FROM sqlite_sequence;";

    // 执行 SQL 语句
    if (!query.exec(sql)) {
        qDebug() << "清空 weather 表失败：" ;
        return false;
    }
    if (!query.exec(sql1)) {
        qDebug() << "清空 sqlite_sequence 表失败：" ;
        return false;
    }
    qDebug() << "数据库已清空";
    return true;
}

bool DbSqlite::ShowTableWeather()
{
    bool lastvalue;
    QSqlQuery query;
    QString sql="SELECT *FROM weather;";
    query.exec(sql);
    while(query.next())
    {
        QString data=QString("%1 %2 %3 %4 %5 %6")
            .arg(query.value(0).toString())
            .arg(query.value(1).toString())
            .arg(query.value(2).toString())
            .arg(query.value(3).toString())
            .arg(query.value(4).toString())
            .arg(query.value(5).toString());
        qDebug()<<data;
    }
    return lastvalue;
}

bool DbSqlite::SaveTeaDb(const QString &City, const QString &Calendar, const double &Avg, const double &max, const double &min)
{
    bool lastvalue;
    QSqlQuery query;
    QString data=QString("INSERT INTO weather (city, calendar, avg_temperature, max_temperature, min_temperature)"
                           "values('%1','%2',%3,%4,%5)").arg(City).arg(Calendar).arg(Avg).arg(max).arg(min);
    if(query.exec(data))
    {
        lastvalue=true;
        qDebug()<<"保存数据成功";
        while(query.next())
        {
            QString outpt;
            outpt=QString("%1 %2 %3 %4 %5")
                               .arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString())
                               .arg(query.value(3).toString())
                               .arg(query.value(4).toString())
                               ;
            qDebug()<<outpt;
        }
    }
    else{
        lastvalue=false;
    }
    return lastvalue;
}

DbSqlite::~DbSqlite()
{
    m_sqldatabase.close();
}
