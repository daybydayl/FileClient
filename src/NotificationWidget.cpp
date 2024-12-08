#include "NotificationWidget.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include "AnimationHelper.h"

NotificationWidget& NotificationWidget::instance()
{
    static NotificationWidget widget;
    return widget;
}

NotificationWidget::NotificationWidget(QWidget *parent)
    : QWidget(parent)
    , m_messageLabel(nullptr)
    , m_timer(nullptr)
    , m_isShowing(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setupUI();
}

void NotificationWidget::setupUI()
{
    resize(300, 80);
    
    m_messageLabel = new QLabel(this);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setContentsMargins(20, 10, 20, 10);
    m_messageLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-size: 14px;"
        "}"
    );
    
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        AnimationHelper::fadeOut(this);
        m_isShowing = false;
        QTimer::singleShot(300, this, &NotificationWidget::showNextMessage);
    });
}

void NotificationWidget::showMessage(const QString& message, Type type, int duration)
{
    m_messageQueue.enqueue({message, type});
    if (!m_isShowing) {
        showNextMessage();
    }
}

void NotificationWidget::showNextMessage()
{
    if (m_messageQueue.isEmpty()) {
        return;
    }
    
    auto message = m_messageQueue.dequeue();
    m_messageLabel->setText(message.first);
    setStyleSheet(getStyleForType(message.second));
    
    // 计算显示位置
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    move(screenGeometry.right() - width() - 20,
         screenGeometry.bottom() - height() - 20);
    
    m_isShowing = true;
    show();
    AnimationHelper::slideIn(this, Qt::RightEdge);
    m_timer->start(3000);
}

void NotificationWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制圆角矩形背景
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.fillPath(path, QColor(0, 0, 0, 180));
    
    QWidget::paintEvent(event);
}

QString NotificationWidget::getStyleForType(Type type) const
{
    QString baseStyle = "QWidget { background-color: rgba(%1, %2, %3, 180); }";
    
    switch (type) {
        case Info:
            return baseStyle.arg("33").arg("150").arg("243"); // 蓝色
        case Success:
            return baseStyle.arg("76").arg("175").arg("80");  // 绿色
        case Warning:
            return baseStyle.arg("255").arg("152").arg("0");  // 橙色
        case Error:
            return baseStyle.arg("244").arg("67").arg("54");  // 红色
        default:
            return baseStyle.arg("97").arg("97").arg("97");   // 灰色
    }
} 