#ifndef ANIMATIONHELPER_H
#define ANIMATIONHELPER_H

#include <QObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QWidget>

/**
 * @brief 动画辅助工具类
 * 
 * 负责:
 * 1. 提供常用的动画效果封装(淡入淡出、滑动等)
 * 2. 管理动画时间曲线和持续时间
 * 3. 处理动画链和并行动画
 * 4. 提供动画状态回调接口
 */
class AnimationHelper : public QObject
{
    Q_OBJECT
    
public:
    static void fadeIn(QWidget* widget, int duration = 200);
    static void fadeOut(QWidget* widget, int duration = 200);
    static void slideIn(QWidget* widget, Qt::Edge edge, int duration = 200);
    static void slideOut(QWidget* widget, Qt::Edge edge, int duration = 200);
    static void shake(QWidget* widget, int duration = 500);
    
private:
    explicit AnimationHelper(QObject *parent = nullptr);
    static QPropertyAnimation* createFadeAnimation(QWidget* widget, qreal start, qreal end, int duration);
    static QPropertyAnimation* createSlideAnimation(QWidget* widget, Qt::Edge edge, bool in, int duration);
};

#endif // ANIMATIONHELPER_H 