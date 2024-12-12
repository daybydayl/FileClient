#include <QApplication>
#include "FileClient.h"
#include <QScreen>

int main(int argc, char *argv[])
{
    // 启用高 DPI 缩放
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    QApplication app(argc, argv);
    
    // 根据屏幕 DPI 调整基础字体大小
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    if (dpi > 96) {  // 标准 DPI 是 96
        QFont font = app.font();
        font.setPointSize(font.pointSize() * (dpi / 96.0 * 0.85));  // 设置适中的缩放因子
        app.setFont(font);
    }
    
    FileClient::instance()->show();
    
    return app.exec();
}