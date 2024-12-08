#include "AnimationHelper.h"
#include <QGraphicsOpacityEffect>

AnimationHelper::AnimationHelper(QObject *parent)
    : QObject(parent)
{
}

void AnimationHelper::fadeIn(QWidget* widget, int duration)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = createFadeAnimation(widget, 0.0, 1.0, duration);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::fadeOut(QWidget* widget, int duration)
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    
    QPropertyAnimation* animation = createFadeAnimation(widget, 1.0, 0.0, duration);
    connect(animation, &QPropertyAnimation::finished, widget, &QWidget::hide);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::slideIn(QWidget* widget, Qt::Edge edge, int duration)
{
    widget->show();
    QPropertyAnimation* animation = createSlideAnimation(widget, edge, true, duration);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::slideOut(QWidget* widget, Qt::Edge edge, int duration)
{
    QPropertyAnimation* animation = createSlideAnimation(widget, edge, false, duration);
    connect(animation, &QPropertyAnimation::finished, widget, &QWidget::hide);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationHelper::shake(QWidget* widget, int duration)
{
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "pos", widget);
    animation->setDuration(duration);
    
    QPoint pos = widget->pos();
    QList<QPoint> keyPoints;
    keyPoints << pos << pos + QPoint(5, 0) << pos - QPoint(5, 0) 
             << pos + QPoint(5, 0) << pos - QPoint(5, 0) << pos;
    
    for (int i = 0; i < keyPoints.size(); ++i) {
        animation->setKeyValueAt(i / static_cast<qreal>(keyPoints.size() - 1), keyPoints[i]);
    }
    
    animation->setEasingCurve(QEasingCurve::OutElastic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPropertyAnimation* AnimationHelper::createFadeAnimation(QWidget* widget, qreal start, qreal end, int duration)
{
    QGraphicsOpacityEffect* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    return animation;
}

QPropertyAnimation* AnimationHelper::createSlideAnimation(QWidget* widget, Qt::Edge edge, bool in, int duration)
{
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "geometry");
    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    
    QRect geometry = widget->geometry();
    QRect startGeometry = geometry;
    QRect endGeometry = geometry;
    
    switch (edge) {
        case Qt::LeftEdge:
            startGeometry.moveLeft(in ? -geometry.width() : geometry.x());
            endGeometry.moveLeft(in ? geometry.x() : -geometry.width());
            break;
        case Qt::RightEdge:
            startGeometry.moveLeft(in ? widget->parentWidget()->width() : geometry.x());
            endGeometry.moveLeft(in ? geometry.x() : widget->parentWidget()->width());
            break;
        case Qt::TopEdge:
            startGeometry.moveTop(in ? -geometry.height() : geometry.y());
            endGeometry.moveTop(in ? geometry.y() : -geometry.height());
            break;
        case Qt::BottomEdge:
            startGeometry.moveTop(in ? widget->parentWidget()->height() : geometry.y());
            endGeometry.moveTop(in ? geometry.y() : widget->parentWidget()->height());
            break;
    }
    
    animation->setStartValue(in ? startGeometry : geometry);
    animation->setEndValue(in ? geometry : endGeometry);
    
    return animation;
} 