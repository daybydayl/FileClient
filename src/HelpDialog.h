#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>

/**
 * @brief 帮助对话框类
 * 
 * 负责:
 * 1. 显示应用程序使用帮助文档
 * 2. 提供帮助内容的导航和搜索
 * 3. 支持富文本格式的帮助内容
 * 4. 管理帮助文档的本地化显示
 */
class HelpDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog();

private:
    void setupUI();
    void loadHelpContent();
    QString loadHtmlFile(const QString& fileName) const;

    QTabWidget* m_tabWidget;
    QTextBrowser* m_overviewBrowser;
    QTextBrowser* m_transferBrowser;
    QTextBrowser* m_configBrowser;
    QPushButton* m_closeBtn;
};

#endif // HELPDIALOG_H 