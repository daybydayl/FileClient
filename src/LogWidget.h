#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QScrollBar>
#include <QTime>

// 前向声明
class QTextEdit;

/**
 * @brief 日志显示组件
 * 
 * 负责:
 * 1. 显示应用程序运行日志
 * 2. 支持不同级别日志的颜色区分
 * 3. 提供日志过滤和搜索功能
 * 4. 管理日志的最大显示条数
 */
class LogWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit LogWidget(QWidget *parent = nullptr);
    ~LogWidget();

public slots:
    void appendLog(const QString& message, bool isError = false);
    void clearLog();

private:
    QVBoxLayout* m_layout;
    QTextEdit* m_textEdit;
};

#endif // LOGWIDGET_H 