#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIntValidator>

/**
 * @brief 地址栏组件
 * 
 * 负责:
 * 1. 显示和编辑当前路径
 * 2. 提供路径导航功能
 * 3. 管理地址历史记录
 * 4. 支持地址自动补全
 */
class AddressBar : public QWidget
{
    Q_OBJECT
    
public:
    explicit AddressBar(QWidget *parent = nullptr);
    ~AddressBar();

    QString host() const;
    quint16 port() const;
    QPushButton* connectButton() const { return m_connectBtn; }

    QString getServerIP() const;
    uint16_t getPort() const;

private slots:
    void updateTheme();

private:
    QHBoxLayout* m_layout;
    QLineEdit* m_addressEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectBtn;
};

#endif // ADDRESSBAR_H