#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include<QString>
#include<QMap>
#include<QFile>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonValue>
#include<QJsonParseError>

class WeatherTool
{
public:
    QString getcitycode(QString cityName);
    void FileWeatherl();
    QMap<QString,QString> m_cityMap;
    void initCityMap();
    WeatherTool()
    {
        FileWeatherl();
    }

};



#endif // WEATHERTOOL_H

