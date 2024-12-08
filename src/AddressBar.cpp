#include "AddressBar.h"
#include "ThemeManager.h"

AddressBar::AddressBar(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_addressEdit(nullptr)
    , m_portEdit(nullptr)
    , m_connectBtn(nullptr)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 1, 5, 1);
    m_layout->setSpacing(0);
    
    // 创建一个容器用于地址和端口
    QWidget* addressContainer = new QWidget(this);
    QHBoxLayout* addressLayout = new QHBoxLayout(addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->setSpacing(0);
    
    // 设置地址输入框
    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setPlaceholderText(tr("请输入服务器地址"));
    m_addressEdit->setFixedHeight(22);  // 从26改为22
    
    // 添加冒号分隔符
    QLabel* colonLabel = new QLabel(":", this);
    
    // 设置端口输入框
    m_portEdit = new QLineEdit(this);
    m_portEdit->setFixedWidth(60);
    m_portEdit->setFixedHeight(22);     // 从26改为22
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));
    m_portEdit->setText("128");  // 默认FTP端口
    
    // 设置连接按钮
    m_connectBtn = new QPushButton(tr("连接"), this);
    m_connectBtn->setFixedHeight(22);   // 从26改为22
    
    // 添加到布局
    addressLayout->addWidget(m_addressEdit);
    addressLayout->addWidget(colonLabel);
    addressLayout->addWidget(m_portEdit);
    
    m_layout->addWidget(addressContainer);
    m_layout->addSpacing(5);  // 添加一点间距
    m_layout->addWidget(m_connectBtn);

    // 连接主题变化信号
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &AddressBar::updateTheme);
    
    // 初始化主题
    updateTheme();
}

void AddressBar::updateTheme()
{
    const ThemeManager& tm = ThemeManager::instance();
    bool isDark = tm.currentTheme() == ThemeManager::Dark;
    
    QString commonStyle = QString(
        "QLineEdit {"
        "    border: 1px solid %1;"
        "    background: %2;"
        "    color: %3;"
        "    padding: 2px 4px;"
        "    selection-background-color: %4;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid %5;"
        "}")
        .arg(isDark ? "#404040" : "#C0C0C0")
        .arg(isDark ? "#2A2A2A" : "#FFFFFF")
        .arg(isDark ? "#E0E0E0" : "#000000")
        .arg(isDark ? "#404040" : "#A0A0A0")
        .arg(isDark ? "#505050" : "#A0A0A0");
    
    m_addressEdit->setStyleSheet(commonStyle);
    m_portEdit->setStyleSheet(commonStyle);
    
    // 设置冒号颜色和垂直对齐
    QList<QLabel*> labels = findChildren<QLabel*>();
    if (!labels.isEmpty()) {
        labels.first()->setStyleSheet(QString(
            "color: %1;"
            "margin: 0 3px;"
            "qproperty-alignment: AlignVCenter;")
            .arg(isDark ? "#808080" : "#404040"));
    }
    
    // 设置按钮样式
    QString btnStyle = QString(
        "QPushButton {"
        "    border: 1px solid %1;"
        "    background: %2;"
        "    color: %3;"
        "    padding: 2px 10px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: %4;"
        "}"
        "QPushButton:pressed {"
        "    background: %5;"
        "}")
        .arg(isDark ? "#404040" : "#C0C0C0")
        .arg(isDark ? "#2A2A2A" : "#F0F0F0")
        .arg(isDark ? "#E0E0E0" : "#000000")
        .arg(isDark ? "#353535" : "#E5E5E5")
        .arg(isDark ? "#252525" : "#DADADA");
    
    m_connectBtn->setStyleSheet(btnStyle);
}

AddressBar::~AddressBar()
{
    // Qt的父子对象系统会自动删除子对象
}

QString AddressBar::host() const
{
    return m_addressEdit->text();
}

quint16 AddressBar::port() const
{
    return m_portEdit->text().toUShort();
}

QString AddressBar::getServerIP() const 
{
    return m_addressEdit->text().section(':', 0, 0);
}

uint16_t AddressBar::getPort() const 
{
    bool ok;
    QString portStr = m_addressEdit->text().section(':', 1, 1);
    uint16_t port = portStr.toUShort(&ok);
    return ok ? port : 128; // 默认返回21端口
}