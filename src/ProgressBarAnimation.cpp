#include "ProgressBarAnimation.h"

ProgressBarAnimation::ProgressBarAnimation(QWidget *parent)
    : QWidget(parent)
    , m_progress(0)
    , m_indeterminatePos(0)
    , m_indeterminateAnimation(nullptr)
{
    setMinimumHeight(4);
    setMaximumHeight(4);
    
    m_indeterminateAnimation = new QPropertyAnimation(this, "indeterminatePos", this);
    m_indeterminateAnimation->setDuration(1500);
    m_indeterminateAnimation->setStartValue(0.0);
    m_indeterminateAnimation->setEndValue(1.0);
    m_indeterminateAnimation->setLoopCount(-1); // 无限循环
}

void ProgressBarAnimation::setProgress(qreal progress)
{
    if (m_progress != progress) {
        m_progress = progress;
        update();
    }
}

void ProgressBarAnimation::setIndeterminatePos(qreal pos)
{
    if (m_indeterminatePos != pos) {
        m_indeterminatePos = pos;
        update();
    }
}

void ProgressBarAnimation::startIndeterminate()
{
    m_indeterminateAnimation->start();
}

void ProgressBarAnimation::stopIndeterminate()
{
    m_indeterminateAnimation->stop();
    m_indeterminatePos = 0;
    update();
}

void ProgressBarAnimation::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), QColor(200, 200, 200));
    
    if (m_indeterminateAnimation->state() == QAbstractAnimation::Running) {
        // 绘制不确定进度条
        int width = rect().width() / 3;
        int x = (rect().width() - width) * m_indeterminatePos;
        QRect progressRect(x, 0, width, height());
        painter.fillRect(progressRect, QColor(33, 150, 243));
    } else {
        // 绘制确定进度条
        int width = rect().width() * m_progress;
        QRect progressRect(0, 0, width, height());
        painter.fillRect(progressRect, QColor(33, 150, 243));
    }
} 