#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "AppConfig.h"

/**
 * @brief 配置对话框类
 * 
 * 负责:
 * 1. 提供应用程序设置界面
 * 2. 管理用户偏好设置的保存和加载
 * 3. 实时预览设置效果
 * 4. 提供设置恢复默认值的功能
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void browseLocalPath();

private:
    void setupUI();
    void createNetworkTab();
    void createTransferTab();
    void createUITab();
    void loadConfig();
    void saveConfig();

    QTabWidget* m_tabWidget;
    
    // 网络设置
    QLineEdit* m_defaultHostEdit;
    QSpinBox* m_defaultPortSpin;
    
    // 传输设置
    QSpinBox* m_retryCountSpin;
    QSpinBox* m_retryIntervalSpin;
    QComboBox* m_speedLimitCombo;
    QCheckBox* m_autoResumeCheck;
    QComboBox* m_minResumeSizeCombo;
    
    // 界面设置
    QLineEdit* m_localPathEdit;
    QCheckBox* m_showHiddenCheck;
    
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
};

#endif // CONFIGDIALOG_H 