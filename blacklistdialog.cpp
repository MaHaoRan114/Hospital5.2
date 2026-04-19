#include "blacklistdialog.h"
#include <QHeaderView>
#include <QMessageBox>

BlacklistDialog::BlacklistDialog(const QString &doctorName, DbManager *dbManager, QWidget *parent)
    : QDialog(parent)
    , m_doctorName(doctorName)
    , m_dbManager(dbManager)
{
    setupUI();
    loadBlacklist();

    setWindowTitle("黑名单管理");
    resize(600, 400);
}

BlacklistDialog::~BlacklistDialog()
{
}

void BlacklistDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("黑名单列表");
    QFont font = titleLabel->font();
    font.setBold(true);
    font.setPointSize(14);
    titleLabel->setFont(font);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    m_blacklistTable = new QTableWidget();
    m_blacklistTable->setColumnCount(5);
    m_blacklistTable->setHorizontalHeaderLabels({"黑名单ID", "患者姓名", "手机号", "加入原因", "加入时间"});
    m_blacklistTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_blacklistTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_blacklistTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_blacklistTable->setAlternatingRowColors(true);
    m_blacklistTable->horizontalHeader()->setStretchLastSection(true);
    m_blacklistTable->setColumnHidden(0, true);

    mainLayout->addWidget(m_blacklistTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_refreshBtn = new QPushButton("刷新");
    m_removeBtn = new QPushButton("移出黑名单");
    m_closeBtn = new QPushButton("关闭");

    buttonLayout->addWidget(m_refreshBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_removeBtn);
    buttonLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(buttonLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &BlacklistDialog::onRefreshClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &BlacklistDialog::onRemoveClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &BlacklistDialog::onCloseClicked);
}

void BlacklistDialog::loadBlacklist()
{
    QList<QMap<QString, QString>> blacklist = m_dbManager->getBlacklist(m_doctorName);

    m_blacklistTable->setRowCount(blacklist.size());

    for (int i = 0; i < blacklist.size(); ++i) {
        const QMap<QString, QString> &item = blacklist[i];

        m_blacklistTable->setItem(i, 0, new QTableWidgetItem(item["black_id"]));
        m_blacklistTable->setItem(i, 1, new QTableWidgetItem(item["patient_name"]));
        m_blacklistTable->setItem(i, 2, new QTableWidgetItem(item["patient_mobile"]));
        m_blacklistTable->setItem(i, 3, new QTableWidgetItem(item["reason"]));
        m_blacklistTable->setItem(i, 4, new QTableWidgetItem(item["create_time"]));
    }
}

void BlacklistDialog::onRefreshClicked()
{
    loadBlacklist();
}

void BlacklistDialog::onRemoveClicked()
{
    int currentRow = m_blacklistTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要移出的患者");
        return;
    }

    QString blackId = m_blacklistTable->item(currentRow, 0)->text();
    QString patientName = m_blacklistTable->item(currentRow, 1)->text();

    int ret = QMessageBox::question(this, "确认移出",
                                    QString("确定要将 %1 移出黑名单吗？").arg(patientName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_dbManager->removeFromBlacklist(blackId)) {
            QMessageBox::information(this, "操作成功", "已移出黑名单");
            emit blacklistChanged();
            loadBlacklist();
        } else {
            QMessageBox::critical(this, "操作失败", m_dbManager->getLastError());
        }
    }
}

void BlacklistDialog::onCloseClicked()
{
    accept();
}
