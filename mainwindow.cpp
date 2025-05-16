#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<weatherTool.h>


#define INCREMENT 5//温度每升高/降低1°，y轴坐标的增量
#define POINT_RADIUS 3//曲线描点大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    this->m_citylist=new CityList();
    m_citylist->setWindowTitle("城市列表");

    ui->setupUi(this);

    this->setWindowFlag(Qt::FramelessWindowHint);//设置无边框
    this->setFixedSize(width(),height());//设置固定大小

    m_menu=new QMenu(this);
    m_menu->addAction("关闭页面");
    m_menu->addAction("保存到数据库");
    m_menu->addAction("查看天气表数据");
    m_menu->addAction("清空数据库");
    m_InitTea=false;
    connect(m_menu->actions()[0], &QAction::triggered, this, &MainWindow::onCloseAction);
    connect(m_menu->actions()[1], &QAction::triggered, [&]()
            {
           //如果未执行过初始化，并且http请求成功，则执行
        if(!m_InitTea&&m_online){
            bool a=DbSqlite::getinstance().init();
            //如果初始执行成功了
            m_InitTea=a;
        }
        //如果初始化执行成功,则执行插入操作
        if(m_InitTea && m_online)
        {
            bool ret0=DbSqlite::getinstance().SaveTeaDb(m_Today.city,m_Today.date,m_Today.wendu.toDouble(),m_Today.high,m_Today.low);
            if(ret0)
            {
                QMessageBox::information(this,"保存提示","保存数据成功");
            }
            else{
                QMessageBox::information(this,"保存提示","失败，城市和日期(作为联合主键)请保持唯一性,每天每个城市只能保存一次数据");
            }
        }

    });

     connect(m_menu->actions()[2], &QAction::triggered, [&]()
    {
        bool a=false;
        //如果未执行过，并且有请求json成功，则执行初始化
        if(!m_InitTea && m_online){
            bool a=DbSqlite::getinstance().init();
            //如果初始执行成功了
            m_InitTea=a;
        }

        //如果请求成功并且初始化过了则
        if(m_InitTea && m_online){
            DbSqlite::getinstance().ShowTableWeather();
        }

    });
    connect(m_menu->actions()[3], &QAction::triggered, [&]()
             {
         //如果未执行过初始化，并且http请求成功，则执行
         if(!m_InitTea&&m_online){
             bool a=DbSqlite::getinstance().init();
             //如果初始执行成功了
             m_InitTea=a;
         }
        if(m_InitTea && m_online)
        {
             bool w=DbSqlite::getinstance().ClearTable();
            if(w){
                 QMessageBox::information(this,"执行提示","清空数据库成功");
            }
            else{
                  QMessageBox::information(this,"执行提示","清空数据库失败");
            }
        }

     });
    m_NetAccessManager =new QNetworkAccessManager;

    connect(m_NetAccessManager,&QNetworkAccessManager::finished
            ,this,&MainWindow::onReplied);
    //直接在构造中请求天气数据，
    //北京的城市编码
    getWeatherInfo("北京");

    AddControlList();//将控件加入容器


    //给标签添加事件过滤器
    //参数指定为this，也就是当前窗口对象，Mainwindow


    ui->ShowlHighLabel->installEventFilter(this);
    ui->ShowLowLabel->installEventFilter(this);
    connect(m_citylist,&CityList::back_City,[=](){
        ui->InputCitylineEdit->setText(m_citylist->m_SelectCity);
        m_citylist->hide();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::parseJson(QByteArray &byte)
{
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(byte,&err);

    if(err.error!=QJsonParseError::NoError)
    {
        return;
    }

    QJsonObject rootobj=doc.object();

    //1.解析日期和城市
    m_Today.date=rootobj.value("date").toString();

    m_Today.city=rootobj.value("cityInfo").
                   toObject().value("city").
                    toString();

    //2.解析yesterday
    QJsonObject objData=rootobj.value("data").toObject();
    QJsonObject objYesterDay=objData.value("yesterday").toObject();

    m_Day[0].week=objYesterDay.value("week").toString();
    m_Day[0].date=objYesterDay.value("ymd").toString();
    m_Day[0].type=objYesterDay.value("type").toString();

    m_Day[0].aqi=objYesterDay.value("aqi").toInt();

    //风向风力
    m_Day[0].fx=objYesterDay.value("fx").toString();
    m_Day[0].fl=objYesterDay.value("fl").toString();

    QString s = objYesterDay.value("high").toString();  // "high": "高温 10℃"
    QStringList parts1 = s.split(" ");  // 分割成 ["高温", "10℃"]
    int hi = parts1[1].replace("℃", "").toInt();  // 提取数字并转换为 int
    m_Day[0].high = hi;  // 存储高温值

    // 获取 "low" 字段并处理
    QString s1 = objYesterDay.value("low").toString();  // "low": "低温 -5℃"
    QStringList parts2 = s1.split(" ");  // 分割成 ["低温", "-5℃"]
    int lo = parts2[1].replace("℃", "").toInt();  // 提取数字并转换为 int
    m_Day[0].low = lo;  // 存储低温值




    //3.解析forcast中5天的数据，今天，到未来4天
    QJsonArray forcastArr=objData.value("forecast").toArray();

    for(int i=0;i<5;i++)
    {
        QJsonObject objForcast=forcastArr[i].toObject();

        m_Day[i+1].week=objForcast.value("week").toString();
        m_Day[i+1].date=objForcast.value("ymd").toString();
        m_Day[i+1].type=objForcast.value("type").toString();
        m_Day[i+1].fx=objForcast.value("fx").toString();
        m_Day[i+1].fl=objForcast.value("fl").toString();
        m_Day[i+1].aqi=objForcast.value("aqi").toInt();


        QString s = objForcast.value("high").toString();  // "high": "高温 10℃"
        QStringList parts1 = s.split(" ");  // 分割成 ["高温", "10℃"]
        int hi = parts1[1].replace("℃", "").toInt();  // 提取数字并转换为 int
        m_Day[i+1].high = hi;  // 存储高温值

        // 获取 "low" 字段并处理
        QString s1 = objForcast.value("low").toString();  // "low": "低温 -5℃"
        QStringList parts2 = s1.split(" ");  // 分割成 ["低温", "-5℃"]
        int lo = parts2[1].replace("℃", "").toInt();  // 提取数字并转换为 int
        m_Day[i+1].low = lo;  // 存储低温值
    }


    //4.解析今天的数据
    m_Today.ganmao=objData.value("ganmao").toString();
    m_Today.wendu=objData.value("wendu").toString();
    m_Today.shidu=objData.value("shidu").toString();
    m_Today.pm25=objData.value("pm25").toInt();
    m_Today.quality=objData.value("quality").toString();


    //5.forcast中第一个数组元素也是今天的数据，也需要加入
    m_Today.type=m_Day[1].type;
    m_Today.fx=m_Day[1].fx;
    m_Today.fl=m_Day[1].fl;
    m_Today.high=m_Day[1].high;
    m_Today.low=m_Day[1].low;
    m_Today.type=m_Day[1].type;

    updateUI();
}

void MainWindow::AddControlList()
{
    //将控件添加到控件数组里面
    m_WeekList.append((ui->W_1));
    m_WeekList.append((ui->W_2));
    m_WeekList.append((ui->W_3));
    m_WeekList.append((ui->W_4));
    m_WeekList.append((ui->W_5));
    m_WeekList.append((ui->W_6));

    m_DateList.append((ui->D_1));
    m_DateList.append((ui->D_2));
    m_DateList.append((ui->D_3));
    m_DateList.append((ui->D_4));
    m_DateList.append((ui->D_5));
    m_DateList.append((ui->D_6));

    m_TypeIconList.append((ui->P_1));
    m_TypeIconList.append((ui->P_2));
    m_TypeIconList.append((ui->P_3));
    m_TypeIconList.append((ui->P_4));
    m_TypeIconList.append((ui->P_5));
    m_TypeIconList.append((ui->P_6));

    m_TypeList.append((ui->P_word1));
    m_TypeList.append((ui->P_word2));
    m_TypeList.append((ui->P_word3));
    m_TypeList.append((ui->P_word4));
    m_TypeList.append((ui->P_word5));
    m_TypeList.append((ui->P_word6));


    m_AqiList.append((ui->L1));
    m_AqiList.append((ui->L2));
    m_AqiList.append((ui->L3));
    m_AqiList.append((ui->L4));
    m_AqiList.append((ui->L5));
    m_AqiList.append((ui->L6));

    m_FxList.append((ui->B1));
    m_FxList.append((ui->B2));
    m_FxList.append((ui->B3));
    m_FxList.append((ui->B4));
    m_FxList.append((ui->B5));
    m_FxList.append((ui->B6));

    m_FlList.append((ui->R1));
    m_FlList.append((ui->R2));
    m_FlList.append((ui->R3));
    m_FlList.append((ui->R4));
    m_FlList.append((ui->R5));
    m_FlList.append((ui->R6));

}

void MainWindow::updateUI()
{
    //1. 更新日期,将 QDateTime 转换为带斜杠的格式
    QDateTime dateTime = QDateTime::fromString(m_Today.date, "yyyyMMdd");
    QString formattedDate = dateTime.toString("yyyy/MM/dd");
    ui->Datalabel->setText(formattedDate+" "+m_Day[0].week);

    //更新左边城市
    ui->Citylabel->setText(m_Today.city);


    //2.更新今天
    ui->Temperaturelabel->setText(m_Today.wendu+"°C");
    //更新advise
    ui->Adviselabel->setText(m_Today.ganmao);

    ui->Weatherlabel->setText(m_Today.type+"  "+QString::number(m_Today.low) +"~"+QString::number(m_Today.high)+"°C");

    ui->TT1->setText(m_Today.fx);
    ui->Rank_1->setText(m_Today.fl);

    ui->Rank_2->setText(QString::number(m_Today.pm25));

    ui->Rank_3->setText(m_Today.shidu);

    ui->Rank_4->setText(m_Today.quality);

    //更新左边天气图标
    m_TypeMap.insert("暴雪",":/res/type/BaoXue.png");
    m_TypeMap.insert("暴雨",":/res/type/BaoYu.png");
    m_TypeMap.insert("暴雨到大暴雨",":/res/type/BaoYuDaoDaBaoYu.png");
    m_TypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    m_TypeMap.insert("大暴雨到特大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    m_TypeMap.insert("大到暴雨",":/res/type/DaDaoBaoXue.png");
    m_TypeMap.insert("大到暴雪",":/res/type/DaDaoBaoYu.png");
    m_TypeMap.insert("大雪",":/res/type/DaXue.png");
    m_TypeMap.insert("大雨",":/res/type/QiangShaChenBao.png");
    m_TypeMap.insert("浮沉",":/res/type/FuChen.png");
    m_TypeMap.insert("多云",":/res/type/DuoYun.png");
    m_TypeMap.insert("冻雨",":/res/type/DongYu.png");

    m_TypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    m_TypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    m_TypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");

    m_TypeMap.insert("晴",":/res/type/Qing.png");
    m_TypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    m_TypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    m_TypeMap.insert("霾",":/res/type/Mai.png");
    m_TypeMap.insert("雾",":/res/type/Wu.png");
    m_TypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    m_TypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    m_TypeMap.insert("小雪",":/res/type/XiaoXue.png");
    m_TypeMap.insert("小雨",":/res/type/XiaoYu.png");
    m_TypeMap.insert("雪",":/res/type/Xue.png");
    m_TypeMap.insert("扬沙",":/res/type/YangSha.png");
    m_TypeMap.insert("阴",":/res/type/Yin.png");
    m_TypeMap.insert("雨",":/res/type/Yu.png");
    m_TypeMap.insert("雨加雪",":/res/type/YuJiaXue.png");
    m_TypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    m_TypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    m_TypeMap.insert("中到大雪",":/res/type/ZhongDaoDaXue.png");
    m_TypeMap.insert("中到大雨",":/res/type/ZhongDaoDaYu.png");
    m_TypeMap.insert("中雪",":/res/type/ZhongXue.png");
    m_TypeMap.insert("中雨",":/res/type/ZhongYu.png");


    ui->WeaPicturelabel->setStyleSheet
        ("border-image: url(" + m_TypeMap[m_Today.type] + ");");
    //更新六天
    for(int i=0;i<6;i++)
    {
        //1.更新周信息
        QFont font = m_WeekList[i]->font();  // 获取当前字体
        font.setPointSize(12);  // 设置字体大小为12
        m_WeekList[i]->setFont(font);  // 应用新的字体设置

        QPalette palette = m_WeekList[i]->palette();  // 获取当前调色板
        palette.setColor(QPalette::WindowText, Qt::white);  // 设置字体颜色为白色
        m_WeekList[i]->setPalette(palette);  // 应用新的颜色设置

        m_WeekList[i]->setAlignment(Qt::AlignCenter);  // 设置文本居中对齐
        QString wek=m_Day[i].week;
        QString Wek=wek[2];
        m_WeekList[i]->setText("周"+ Wek);

        //2.更新日期信息
        QString dateStr = m_Day[i].date;
        // 将字符串转换为 QDate 对象
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        // 格式化为 "MM/dd" 格式
        QString formattedDate = date.toString("MM/dd");

        m_DateList[i]->setText(formattedDate);

        // 设置字体大小为 12
        QFont font1 = m_DateList[i]->font();
        font.setPointSize(12);
        m_DateList[i]->setFont(font);

        // 设置字体颜色为白色
        QPalette palette1 =  m_DateList[i]->palette();
        palette.setColor(QPalette::WindowText, Qt::white);
        m_DateList[i]->setPalette(palette);

        // 设置文本居中对齐
        m_DateList[i]->setAlignment(Qt::AlignCenter);

        //3.更新天气类型
        QFont font2 = m_TypeList[i]->font();
        font2.setPointSize(12);
        m_TypeList[i]->setFont(font2);
        QPalette palette2 = m_TypeList[i]->palette();  // 获取当前调色板
        palette2.setColor(QPalette::WindowText, Qt::white);  // 设置字体颜色为白色
        m_TypeList[i]->setPalette(palette2);  // 应用新的颜色设置

        m_TypeList[i]->setAlignment(Qt::AlignCenter);  // 设置文本居中对齐

        m_TypeList[i]->setText(m_Day[i].type);

        //4.更新空气质量
        QFont font3=m_AqiList[i]->font();
        font3.setPointSize(11);
        m_AqiList[i]->setFont(font3);
        QPalette palette3=m_AqiList[i]->palette();
        palette3.setColor(QPalette::WindowText, Qt::white);
        m_AqiList[i]->setPalette(palette3);
        m_AqiList[i]->setAlignment(Qt::AlignCenter);
        //优0~50
        if(m_Day[i].aqi>=0 &&m_Day[i].aqi<=50)
        {
            m_AqiList[i]->setText("优");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(170, 234, 0);");
        }
        //良
        else if(m_Day[i].aqi>50 &&m_Day[i].aqi<=100)
        {
            m_AqiList[i]->setText("良");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(255, 173, 102);");
        }
        //轻度
        else if(m_Day[i].aqi>100 &&m_Day[i].aqi<=150)
        {
            m_AqiList[i]->setText("轻度");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(255, 119, 171);");
        }
        //中度
        else if(m_Day[i].aqi>150 &&m_Day[i].aqi<=200)
        {
            m_AqiList[i]->setText("中度");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(255, 80, 147);");
        }
        //重度
        else if(m_Day[i].aqi>200 &&m_Day[i].aqi<=250)
        {
            m_AqiList[i]->setText("重度");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(255, 5, 51);");
        }
        //严重
        else
        {
            m_AqiList[i]->setText("严重");
            m_AqiList[i]->setStyleSheet
                ("background-color: rgb(197, 0, 3);");
        }

        QFont font4= m_TypeList[i]->font();
        font4.setPointSize(11);
        m_TypeList[i]->setFont(font4);
        QPalette palette4= m_TypeList[i]->palette();
        palette4.setColor(QPalette::WindowText, Qt::white);
        m_TypeList[i]->setPalette(palette4);
        m_TypeList[i]->setAlignment(Qt::AlignCenter);
        m_TypeList[i]->setText(m_Day[i].type);



        QString T=m_Day[i].type;//今天的天气
        QString sheet=m_TypeMap[T];
        m_TypeIconList[i]->setStyleSheet ("border-image: url(" + sheet + ");");



        QFont font5=m_FxList[i]->font();
        font5.setPointSize(12);
        QPalette palette5=  m_FxList[i]->palette();
        palette5.setColor(QPalette::WindowText, Qt::white);
        m_FxList[i]->setFont(font5);
        m_FxList[i]->setPalette(palette5);
        m_FxList[i]->setAlignment(Qt::AlignCenter);
        m_FxList[i]->setText(m_Day[i].fx);


        QFont font6= m_FlList[i]->font();
        font6.setPointSize(12);
        QPalette palette6= m_FlList[i]->palette();
        palette6.setColor(QPalette::WindowText, Qt::white);
        m_FlList[i]->setFont(font6);
        m_FlList[i]->setPalette(palette6);
        m_FlList[i]->setAlignment(Qt::AlignCenter);
        m_FlList[i]->setText(m_Day[i].fl);
    }

    ui->ShowlHighLabel->update();
    ui->ShowLowLabel->update();

}



void MainWindow::getWeatherInfo(QString CityName)
{
    WeatherTool Tool;
    QString cityCode=Tool.getcitycode(CityName);

    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    m_NetAccessManager->get(QNetworkRequest(url));
}

bool MainWindow::onReplied(QNetworkReply *reply)
{

    bool lastvalue0;
    int status_code=reply->attribute
                      (QNetworkRequest::HttpStatusCodeAttribute)
                          .toInt();//获取响应码，200时为成功

    if(200==status_code)
    {
        qDebug()<<"onRelied sucess!";
        QByteArray bytearray=reply->readAll();
        parseJson(bytearray);
        lastvalue0=true;
    }

    else
    {
        qDebug()<<"onRelied failed";
        lastvalue0=false;
    }
    m_online=lastvalue0;
    qDebug()<<m_online;
    return lastvalue0;
    reply->deleteLater();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{

    m_Offset=event->globalPos()-this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-m_Offset);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        // 按下回车键时显示一个消息框
        QString input=ui->InputCitylineEdit->text();
        getWeatherInfo(input);
    }

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->ShowlHighLabel &&event->type()==QEvent::Paint)
    {
        paintHighCurve();
    }
    if(watched==ui->ShowLowLabel &&event->type()==QEvent::Paint)
    {
        paintLowCurve();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->ShowlHighLabel);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    //1.获取x坐标
    int X_pos[6]={0};
    for(int i=0;i<6;i++)
    {
        X_pos[i]=m_WeekList[i]->pos().x()+m_WeekList[i]->width()/2;//控件中间位置为x
    }
    // 2. 获取y坐标
    int tempSum = 0;
    int tempAverage = 0;
    for (int i = 0; i < 6; i++) {
        tempSum += m_Day[i].high;
    }
    tempAverage = tempSum / 6; // 最高温度的平均值

    // 3. 计算y轴坐标
    int Y_pos[6] = {0};
    int yCenter = ui->ShowlHighLabel->height() / 2; // 获取标签中间的值
    for (int i = 0; i < 6; i++) {
        // 计算每天温度与平均温度的差值
        // 如果 m_Day[i].high 比 tempAverage 高，Y_pos[i] 会相对较小，表示绘制的点会在图表的上方。
        // 如果 m_Day[i].high 比 tempAverage 低，Y_pos[i] 会相对较大，表示绘制的点会在图表的下方。
        // y轴方向是从上往下为正向偏移
        Y_pos[i] = yCenter - ((m_Day[i].high - tempAverage) * INCREMENT);
    }
    // 4. 开始绘制
    // 4.1 初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255, 170, 0)); // 设置画笔的颜色
    painter.setPen(pen);
    painter.setBrush(QColor(255, 170, 0)); // 设置画刷颜色-内部填充的颜色
    // 4.2 画点，写文本
    for (int i = 0; i < 6; i++) {
        // 显示点
        painter.drawEllipse(QPoint(X_pos[i], Y_pos[i]), POINT_RADIUS, POINT_RADIUS);
        // 显示温度文本，文本偏移
        painter.drawText(X_pos[i] - TEXT_OFFSET_X, Y_pos[i] - TEXT_OFFSET_Y, QString::number(m_Day[i].high) + "°");
    }

    // 4.3 绘制（连接）曲线
    for (int i = 0; i < 5; i++) {
        painter.drawLine(X_pos[i], Y_pos[i], X_pos[i + 1], Y_pos[i + 1]);
    }



}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->ShowLowLabel);
    painter.setRenderHint(QPainter::Antialiasing);
    // 1. 获取x坐标
    int X_pos[6] = {0};
    for (int i = 0; i < 6; i++) {
        X_pos[i] = m_WeekList[i]->pos().x() + m_WeekList[i]->width() / 2; // 控件中间位置为x
    }

    // 2. 获取y坐标
    int tempSum = 0;
    int tempAverage = 0;
    for (int i = 0; i < 6; i++) {
        tempSum += m_Day[i].low; // 使用低温数据 m_Day[i].low
    }
    tempAverage = tempSum / 6; // 最低温度的平均值


    // 3. 计算y轴坐标
    int Y_pos[6] = {0};
    int yCenter = ui->ShowLowLabel->height() / 2; // 获取标签中间的值

    for (int i = 0; i < 6; i++) {
        // 计算每天低温与平均低温的差值
        // 如果 m_Day[i].low 比 tempAverage 高，Y_pos[i] 会相对较小，表示绘制的点会在图表的上方。
        // 如果 m_Day[i].low 比 tempAverage 低，Y_pos[i] 会相对较大，表示绘制的点会在图表的下方。
        // y轴方向是从上往下为正向偏移
        Y_pos[i] = yCenter - ((m_Day[i].low - tempAverage) * INCREMENT);
    }

    // 4. 开始绘制
    // 4.1 初始化画笔
    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0, 170, 255)); // 设置画笔的颜色（蓝色）
    painter.setPen(pen);
    painter.setBrush(QColor(0, 170, 255)); // 设置画刷颜色-内部填充的颜色（蓝色）

    // 4.2 画点，写文本
    for (int i = 0; i < 6; i++) {
        // 显示点
        painter.drawEllipse(QPoint(X_pos[i], Y_pos[i]), POINT_RADIUS, POINT_RADIUS);
        // 显示低温文本，文本偏移
        painter.drawText(X_pos[i] - TEXT_OFFSET_X, Y_pos[i] - TEXT_OFFSET_Y, QString::number(m_Day[i].low) + "°");
    }

    // 4.3 绘制（连接）曲线
    for (int i = 0; i < 5; i++) {
        painter.drawLine(X_pos[i], Y_pos[i], X_pos[i + 1], Y_pos[i + 1]);
    }

}

void MainWindow::on_MenupushButton_2_clicked()
{
    m_menu->exec((ui->MenupushButton_2->mapToGlobal(QPoint(0, ui->MenupushButton_2->height()))));

}

void MainWindow::onCloseAction()
{
    this->close();
}


void MainWindow::on_SearchpushButton_clicked()
{

    QString input=ui->InputCitylineEdit->text();
    getWeatherInfo(input);
}

//转到城市列表索引

void MainWindow::on_CityListpushButton_clicked()
{


    this->m_citylist->show();

}

