#include "LoadingSpinner.h"
#include <QPainter>
#include <QTimer>
#include <cmath>

LoadingSpinner::LoadingSpinner(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_isSpinning(false)
    , m_angle(0)
    , m_color(Qt::black)
    , m_roundness(100.0)
    , m_minimumTrailOpacity(15.0)
    , m_trailFadePercentage(70.0)
    , m_revolutionsPerSecond(1.0)
    , m_numberOfLines(12)
    , m_lineLength(10)
    , m_lineWidth(2)
    , m_innerRadius(10)
{
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_angle = std::fmod(m_angle + 360.0 / (60 * m_revolutionsPerSecond), 360.0);
        update();
    });
    
    updateSize();
    hide();
}

void LoadingSpinner::start()
{
    m_isSpinning = true;
    show();
    m_timer->start(1000 / 60); // 60 FPS
}

void LoadingSpinner::stop()
{
    m_isSpinning = false;
    hide();
    m_timer->stop();
}

bool LoadingSpinner::isSpinning() const
{
    return m_isSpinning;
}

void LoadingSpinner::setColor(const QColor& color)
{
    m_color = color;
}

void LoadingSpinner::setRoundness(qreal roundness)
{
    m_roundness = roundness;
}

void LoadingSpinner::setMinimumTrailOpacity(qreal opacity)
{
    m_minimumTrailOpacity = opacity;
}

void LoadingSpinner::setTrailFadePercentage(qreal percentage)
{
    m_trailFadePercentage = percentage;
}

void LoadingSpinner::setRevolutionsPerSecond(qreal rps)
{
    m_revolutionsPerSecond = rps;
}

void LoadingSpinner::setNumberOfLines(int lines)
{
    m_numberOfLines = lines;
    updateSize();
}

void LoadingSpinner::setLineLength(int length)
{
    m_lineLength = length;
    updateSize();
}

void LoadingSpinner::setLineWidth(int width)
{
    m_lineWidth = width;
    updateSize();
}

void LoadingSpinner::setInnerRadius(int radius)
{
    m_innerRadius = radius;
    updateSize();
}

void LoadingSpinner::updateSize()
{
    int size = (m_innerRadius + m_lineLength) * 2;
    setFixedSize(size, size);
}

void LoadingSpinner::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!m_isSpinning) {
        return;
    }
    
    painter.translate(width() / 2, height() / 2);
    painter.rotate(m_angle);
    
    for (int i = 0; i < m_numberOfLines; ++i) {
        painter.rotate(360.0 / m_numberOfLines);
        qreal opacity = 1.0;
        if (m_trailFadePercentage > 0) {
            opacity = 1.0 - (i * m_trailFadePercentage / 100.0);
            if (opacity < m_minimumTrailOpacity / 100.0) {
                opacity = m_minimumTrailOpacity / 100.0;
            }
        }
        
        QColor color = m_color;
        color.setAlphaF(opacity);
        painter.setPen(QPen(color, m_lineWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(m_innerRadius, 0, m_innerRadius + m_lineLength, 0);
    }
}

void LoadingSpinner::setAngle(qreal angle)
{
    m_angle = angle;
    update();
} 