#include "ThemeManager.h"
#include <QFile>
#include <QTextStream>
#include <QPalette>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(System)
{
    // 加载主题样式表
    m_styleSheets[Light] = loadStyleSheet(":/themes/themes/light.qss");
    m_styleSheets[Dark] = loadStyleSheet(":/themes/themes/dark.qss");
    
    // 输出调试信息
    qDebug() << "Light theme loaded:" << !m_styleSheets[Light].isEmpty();
    qDebug() << "Dark theme loaded:" << !m_styleSheets[Dark].isEmpty();
    
    // 应用初始主题
    applySystemTheme();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        
        // 设置应用程序样式
        qApp->setStyle(QStyleFactory::create("Fusion"));
        
        if (theme == System) {
            applySystemTheme();
        } else {
            QString styleSheet = m_styleSheets[theme];
            if (styleSheet.isEmpty()) {
                qDebug() << "Warning: Empty stylesheet for theme" << theme;
            }
            
            // 设置调色板
            QPalette palette;
            if (theme == Dark) {
                palette.setColor(QPalette::Window, QColor(45, 45, 45));
                palette.setColor(QPalette::WindowText, Qt::white);
                palette.setColor(QPalette::Base, QColor(54, 54, 54));
                palette.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
                palette.setColor(QPalette::ToolTipBase, Qt::white);
                palette.setColor(QPalette::ToolTipText, Qt::white);
                palette.setColor(QPalette::Text, Qt::white);
                palette.setColor(QPalette::Button, QColor(54, 54, 54));
                palette.setColor(QPalette::ButtonText, Qt::white);
                palette.setColor(QPalette::Link, QColor(42, 130, 218));
                palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
                palette.setColor(QPalette::HighlightedText, Qt::white);
            } else {
                palette.setColor(QPalette::Window, QColor(245, 245, 245));
                palette.setColor(QPalette::WindowText, Qt::black);
                palette.setColor(QPalette::Base, Qt::white);
                palette.setColor(QPalette::AlternateBase, QColor(250, 250, 250));
                palette.setColor(QPalette::ToolTipBase, Qt::black);
                palette.setColor(QPalette::ToolTipText, Qt::black);
                palette.setColor(QPalette::Text, Qt::black);
                palette.setColor(QPalette::Button, Qt::white);
                palette.setColor(QPalette::ButtonText, Qt::black);
                palette.setColor(QPalette::Link, QColor(33, 150, 243));
                palette.setColor(QPalette::Highlight, QColor(33, 150, 243));
                palette.setColor(QPalette::HighlightedText, Qt::white);
            }
            
            qApp->setPalette(palette);
            qApp->setStyleSheet(styleSheet);
        }
        
        emit themeChanged(theme);
    }
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

QString ThemeManager::getStyleSheet() const
{
    return m_styleSheets[m_currentTheme];
}

QString ThemeManager::loadStyleSheet(const QString& fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open stylesheet file:" << fileName;
        return QString();
    }
    
    QTextStream in(&file);
    QString styleSheet = in.readAll();
    file.close();
    
    if (styleSheet.isEmpty()) {
        qDebug() << "Warning: Empty stylesheet loaded from" << fileName;
    }
    
    return styleSheet;
}

void ThemeManager::applySystemTheme()
{
    // 根据系统主题设置样式
    QPalette palette = qApp->style()->standardPalette();
    if (palette.color(QPalette::Window).lightness() < 128) {
        setTheme(Dark);
    } else {
        setTheme(Light);
    }
}