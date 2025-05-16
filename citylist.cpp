#include "citylist.h"
#include "ui_citylist.h"

CityList::CityList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CityList)
{
    this->SetCities();
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建一个输入框用于搜索城市
    QLineEdit *searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("输入城市名称...");
    mainLayout->addWidget(searchBox);

    // 创建一个 QListWidget 来显示城市列表
    cityList = new QListWidget(this);

    cityList->addItems(cities);

    // 将 QListWidget 放入 QScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(cityList); // 设置 QListWidget 为滚动区域的内容
    scrollArea->setWidgetResizable(true); // 允许内容控件调整大小
    mainLayout->addWidget(scrollArea);

    // 创建一个 QLabel 来显示选中的城市信息
    cityInfoLabel = new QLabel("请选择一个城市", this);
    cityInfoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(cityInfoLabel);

    // 连接信号槽：当城市列表中的项被点击时，更新 QLabel 的内容
    connect(cityList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        cityInfoLabel->setText("您选择了: " + item->text());
        m_SelectCity=item->text();
        emit back_City();
    });

    // 连接信号槽：当输入框内容变化时，搜索匹配的城市
    connect(searchBox, &QLineEdit::textChanged, [this](const QString &text) {
        filterCities(text);
    });

    ui->setupUi(this);
}

CityList::~CityList()
{
    delete ui;
}

void CityList::SetCities()
{

    QFile file(":/s/citycode-2019-08-23.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file.";
        return;
    }
    // 读取文件内容
    QByteArray data = file.readAll();
    file.close();

    // 将JSON数据转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to parse JSON.";
        return;
    }
    // 检查是否是数组
    if (!jsonDoc.isArray()) {
        qDebug() << "JSON is not an array.";
        return;
    }
    // 获取JSON数组
    QJsonArray jsonArray = jsonDoc.array();

    // 用于存储城市名的QStringList
    // 遍历数组中的每个对象
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            // 获取city_name字段
            if (obj.contains("city_name") && obj["city_code"].toString() != "") {
                QString cityName = obj["city_name"].toString();
                this->cities.append(cityName); // 将城市名添加到QStringList中
            }
        }

}
}

void CityList::filterCities(const QString &keyword)
{
    for (int i = 0; i < cityList->count(); ++i) {
        QListWidgetItem *item = cityList->item(i);
        if (item->text().contains(keyword, Qt::CaseInsensitive)) {
            item->setHidden(false); // 显示匹配的城市
            if (keyword.isEmpty() || item->text().startsWith(keyword, Qt::CaseInsensitive)) {
                cityList->setCurrentItem(item); // 自动选中第一个匹配的城市
                cityInfoLabel->setText("您选择了: " + item->text());
            }
        } else {
            item->setHidden(true); // 隐藏不匹配的城市
        }
    }
}

