#include "LogWidget.h"

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_textEdit(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    
    m_layout->addWidget(m_textEdit);
}

LogWidget::~LogWidget()
{
    // Qt的父子对象系统会自动删除子对象
}

void LogWidget::appendLog(const QString& message, bool isError)
{
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp).arg(message);
    
    QTextCharFormat format;
    if (isError) {
        format.setForeground(Qt::red);
    }
    
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText(formattedMessage, format);
    
    // 滚动到底部
    m_textEdit->verticalScrollBar()->setValue(
        m_textEdit->verticalScrollBar()->maximum()
    );
}

void LogWidget::clearLog()
{
    m_textEdit->clear();
} 