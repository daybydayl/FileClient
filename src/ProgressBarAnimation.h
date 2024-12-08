#ifndef PROGRESSBARANIMATION_H
#define PROGRESSBARANIMATION_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>

/**
 * @brief 进度条动画类
 * 
 * 负责:
 * 1. 提供进度条的平滑动画效果
 * 2. 管理动画的时间曲线和持续时间
 * 3. 处理进度值的插值计算
 * 4. 支持动画的暂停和恢复
 */
class ProgressBarAnimation : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
    Q_PROPERTY(qreal indeterminatePos READ indeterminatePos WRITE setIndeterminatePos)
    
public:
    explicit ProgressBarAnimation(QWidget *parent = nullptr);
    
    void setProgress(qreal progress);
    qreal progress() const { return m_progress; }
    
    void setIndeterminatePos(qreal pos);
    qreal indeterminatePos() const { return m_indeterminatePos; }
    
    void startIndeterminate();
    void stopIndeterminate();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    qreal m_progress;
    qreal m_indeterminatePos;
    QPropertyAnimation* m_indeterminateAnimation;
};

#endif // PROGRESSBARANIMATION_H 