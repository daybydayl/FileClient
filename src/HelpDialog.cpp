#include "HelpDialog.h"
#include <QFile>
#include <QTextStream>

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
    , m_overviewBrowser(nullptr)
    , m_transferBrowser(nullptr)
    , m_configBrowser(nullptr)
    , m_closeBtn(nullptr)
{
    setupUI();
    loadHelpContent();
}

HelpDialog::~HelpDialog()
{
}

void HelpDialog::setupUI()
{
    setWindowTitle(tr("帮助"));
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget(this);
    
    // 概述页面
    m_overviewBrowser = new QTextBrowser(this);
    m_tabWidget->addTab(m_overviewBrowser, tr("概述"));
    
    // 传输页面
    m_transferBrowser = new QTextBrowser(this);
    m_tabWidget->addTab(m_transferBrowser, tr("传输"));
    
    // 配置页面
    m_configBrowser = new QTextBrowser(this);
    m_tabWidget->addTab(m_configBrowser, tr("配置"));
    
    mainLayout->addWidget(m_tabWidget);
    
    // 关闭按钮
    m_closeBtn = new QPushButton(tr("关闭"), this);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(buttonLayout);
}

void HelpDialog::loadHelpContent()
{
    // 加载帮助文档内容
    m_overviewBrowser->setHtml(loadHtmlFile(":/help/overview.html"));
    m_transferBrowser->setHtml(loadHtmlFile(":/help/transfer.html"));
    m_configBrowser->setHtml(loadHtmlFile(":/help/config.html"));
}

QString HelpDialog::loadHtmlFile(const QString& fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("<h1>无法加载帮助文档</h1>");
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    return in.readAll();
} 