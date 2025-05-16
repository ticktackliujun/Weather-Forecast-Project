#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMenu>
#include <QMouseEvent>
#include<weatherdata.h>


#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>

#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>

#include<QList>
#include<QLabel>

#include<QKeyEvent>
#include<QPainter>

#include"citylist.h"
#include"dbsqlite.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;


public:
    void getWeatherInfo(QString cityCode);
    void parseJson(QByteArray &byte);
    void AddControlList();
    void updateUI();

    bool m_InitTea;
    bool m_online;
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event) override;//重写键盘事件

     //重写eventfilter方法
    bool eventFilter(QObject*watched,QEvent*event) override;
    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();

private slots:
    void on_MenupushButton_2_clicked();
    void onCloseAction();

    bool onReplied(QNetworkReply * reply);
    void on_SearchpushButton_clicked();

    void on_CityListpushButton_clicked();

private:
    CityList *m_citylist;
    QMenu *m_menu;
    QPoint m_Offset;//窗口移动时，鼠标于窗口左上的偏移
    QNetworkAccessManager *m_NetAccessManager;

    Today m_Today;
    Day m_Day[6];


    //控件数组 星期和日期
    QList<QLabel*> m_WeekList;
    QList<QLabel*> m_DateList;

    //控件数组 天气和天气图标
    QList<QLabel*> m_TypeList;
    QList<QLabel*> m_TypeIconList;

    //天气污染指数
    QList<QLabel*> m_AqiList;

    //风力和风向
    QList<QLabel*> m_FxList;
    QList<QLabel*> m_FlList;

    //
    QMap<QString,QString> m_TypeMap;

};
#endif // MAINWINDOW_H
