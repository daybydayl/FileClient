#ifndef LOADINGSPINNER_H
#define LOADINGSPINNER_H

#include <QWidget>
#include <QTimer>
#include <QColor>

/**
 * @brief 加载动画组件
 * 
 * 负责:
 * 1. 显示旋转加载动画效果
 * 2. 支持自定义动画参数(速度、大小、颜色等)
 * 3. 提供开始/停止动画的控制接口
 * 4. 支持不同主题下的样式适配
 */
class LoadingSpinner : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)
    
public:
    explicit LoadingSpinner(QWidget *parent = nullptr);
    
    void start();
    void stop();
    bool isSpinning() const;
    
    void setColor(const QColor& color);
    void setRoundness(qreal roundness);
    void setMinimumTrailOpacity(qreal opacity);
    void setTrailFadePercentage(qreal percentage);
    void setRevolutionsPerSecond(qreal rps);
    void setNumberOfLines(int lines);
    void setLineLength(int length);
    void setLineWidth(int width);
    void setInnerRadius(int radius);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    qreal angle() const { return m_angle; }
    void setAngle(qreal angle);
    
    QTimer* m_timer;
    bool m_isSpinning;
    qreal m_angle;
    QColor m_color;
    qreal m_roundness;
    qreal m_minimumTrailOpacity;
    qreal m_trailFadePercentage;
    qreal m_revolutionsPerSecond;
    int m_numberOfLines;
    int m_lineLength;
    int m_lineWidth;
    int m_innerRadius;
    void updateSize();
};

#endif // LOADINGSPINNER_H 