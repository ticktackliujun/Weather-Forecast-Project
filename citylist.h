#ifndef CITYLIST_H
#define CITYLIST_H


#include <QApplication>
#include <QVBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include<QFile>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
namespace Ui {
class CityList;
}

class CityList : public QWidget
{
    Q_OBJECT

signals:
    void back_City();


public:
    explicit CityList(QWidget *parent = nullptr);
    ~CityList();
    QString m_SelectCity;
    void SetCities();
private:
    QStringList cities;
    Ui::CityList *ui;
    QListWidget *cityList; // 城市列表
    QLabel *cityInfoLabel; // 显示城市信息的标签
    void filterCities(const QString &keyword);
};

#endif // CITYLIST_H
