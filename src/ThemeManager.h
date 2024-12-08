#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStyle>

/**
 * @brief 主题管理器类
 * 
 * 负责:
 * 1. 管理应用程序主题样式
 * 2. 提供主题切换功能
 * 3. 处理系统主题变更事件
 * 4. 维护自定义主题配置
 */
class ThemeManager : public QObject
{
    Q_OBJECT
    
public:
    enum Theme {
        Light,
        Dark,
        System
    };
    Q_ENUM(Theme)

    static ThemeManager& instance();
    
    void setTheme(Theme theme);
    Theme currentTheme() const;
    QString getStyleSheet() const;

signals:
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    
    QString loadStyleSheet(const QString& fileName) const;
    void applySystemTheme();
    
    Theme m_currentTheme;
    QMap<Theme, QString> m_styleSheets;
};

#endif // THEMEMANAGER_H 