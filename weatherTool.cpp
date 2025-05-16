#include<weatherTool.h>

QString WeatherTool::getcitycode(QString cityName)
{
    return m_cityMap[cityName];
}

void WeatherTool::FileWeatherl()
{

        //1.读取文件
    QString fp=":/s/citycode-2019-08-23.json";
    QFile file1(fp);
    file1.open(QIODeviceBase::ReadOnly);
    QByteArray json1=file1.readAll();
    file1.close();


    //2.解析，并写入到map
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(json1,&err);

    if(err.error!=QJsonParseError::NoError)
    {
        qDebug()<<"cityjson error!";
        return;
    }


    QJsonArray cityArray = doc.array();

    for(int i=0;i<cityArray.size();i++)
    {
        QString City=cityArray[i].toObject().value("city_name").toString();
        QString Code=cityArray[i].toObject().value("city_code").toString();
        if(Code.size()>0)
        {
            m_cityMap.insert(City,Code);
        }
    }
}
