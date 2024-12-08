#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QQueue>

/**
 * @brief 通知提示组件
 * 
 * 负责:
 * 1. 显示临时的通知消息(成功、警告、错误等)
 * 2. 管理通知的显示时长和动画效果
 * 3. 支持多条通知的队列显示
 * 4. 提供不同样式的通知主题
 */
class NotificationWidget : public QWidget
{
    Q_OBJECT
    
public:
    enum Type {
        Info,
        Success,
        Warning,
        Error
    };
    
    static NotificationWidget& instance();
    
    void showMessage(const QString& message, Type type = Info, int duration = 3000);

protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    explicit NotificationWidget(QWidget *parent = nullptr);
    void setupUI();
    void showNextMessage();
    QString getStyleForType(Type type) const;
    
    QLabel* m_messageLabel;
    QTimer* m_timer;
    QQueue<QPair<QString, Type>> m_messageQueue;
    bool m_isShowing;
};

#endif // NOTIFICATIONWIDGET_H 