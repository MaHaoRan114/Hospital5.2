#include "consultmainwindow.h"
#include "blacklistdialog.h"
#include "invalidconsultdialog.h"
#include <QHeaderView>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QDebug>

ConsultMainWindow::ConsultMainWindow(const QString &doctorName, QWidget *parent)
    : QMainWindow(parent)
    , m_currentDoctor(doctorName)
{
    m_dbManager = new DbManager(this);

    // 创建自动结束定时器，每分钟检查一次
    m_autoEndTimer = new QTimer(this);
    m_autoEndTimer->setInterval(60000);
    connect(m_autoEndTimer, &QTimer::timeout, this, &ConsultMainWindow::checkInactiveConsults);

    setupUI();
    setupMenuBar();

    // 连接数据库
    if (m_dbManager->connectToDatabase()) {
        loadConsultRecords();
        statusBar()->showMessage("数据库连接成功", 3000);
        m_autoEndTimer->start();
    } else {
        statusBar()->showMessage("数据库连接失败: " + m_dbManager->getLastError(), 5000);
    }

    setWindowTitle("在线问诊医生端 - " + doctorName);
    resize(1100, 700);
    setMinimumSize(900, 600);
}

ConsultMainWindow::~ConsultMainWindow()
{
}

void ConsultMainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(m_mainSplitter);

    // ========== 左侧面板 ==========
    m_leftPanel = new QWidget();
    m_leftPanel->setMinimumWidth(450);
    m_leftPanel->setMaximumWidth(550);

    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);
    leftLayout->setSpacing(5);

    // 搜索和筛选区域
    QHBoxLayout *searchLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索患者姓名、就诊卡号...");
    m_searchEdit->setMinimumHeight(35);

    m_stateFilter = new QComboBox();
    m_stateFilter->addItem("全部状态", "");
    m_stateFilter->addItem("新申请", "1");
    m_stateFilter->addItem("接收中", "2");
    m_stateFilter->addItem("已结束", "3");
    m_stateFilter->setMinimumHeight(35);
    m_stateFilter->setMinimumWidth(100);

    m_refreshBtn = new QPushButton("刷新");
    m_refreshBtn->setMinimumHeight(35);
    m_refreshBtn->setMinimumWidth(60);
    m_refreshBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #1976D2; }"
        );

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_stateFilter);
    searchLayout->addWidget(m_refreshBtn);

    leftLayout->addLayout(searchLayout);

    // 咨询记录表格
    m_consultTable = new QTableWidget();
    m_consultTable->setColumnCount(6);
    m_consultTable->setHorizontalHeaderLabels({"就诊时间", "患者姓名", "就诊卡号", "状态", "金额", "订单号"});
    m_consultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_consultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_consultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_consultTable->setAlternatingRowColors(true);
    m_consultTable->horizontalHeader()->setStretchLastSection(true);
    m_consultTable->setColumnWidth(0, 150);
    m_consultTable->setColumnWidth(1, 80);
    m_consultTable->setColumnWidth(2, 100);
    m_consultTable->setColumnWidth(3, 80);
    m_consultTable->setColumnWidth(4, 70);
    m_consultTable->setColumnHidden(5, true);

    leftLayout->addWidget(m_consultTable);

    // ========== 右侧面板 ==========
    m_rightPanel = new QWidget();

    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightPanel);
    rightLayout->setContentsMargins(5, 5, 5, 5);
    rightLayout->setSpacing(5);

    // 患者信息区域
    m_patientInfoGroup = new QGroupBox("患者信息");
    QGridLayout *infoLayout = new QGridLayout(m_patientInfoGroup);

    infoLayout->addWidget(new QLabel("姓名:"), 0, 0);
    m_patientNameLabel = new QLabel("-");
    infoLayout->addWidget(m_patientNameLabel, 0, 1);

    infoLayout->addWidget(new QLabel("年龄:"), 0, 2);
    m_patientAgeLabel = new QLabel("-");
    infoLayout->addWidget(m_patientAgeLabel, 0, 3);

    infoLayout->addWidget(new QLabel("性别:"), 0, 4);
    m_patientSexLabel = new QLabel("-");
    infoLayout->addWidget(m_patientSexLabel, 0, 5);

    infoLayout->addWidget(new QLabel("就诊卡号:"), 1, 0);
    m_patientCardLabel = new QLabel("-");
    infoLayout->addWidget(m_patientCardLabel, 1, 1, 1, 2);

    infoLayout->addWidget(new QLabel("咨询状态:"), 1, 3);
    m_consultStateLabel = new QLabel("-");
    infoLayout->addWidget(m_consultStateLabel, 1, 4, 1, 2);

    infoLayout->addWidget(new QLabel("支付金额:"), 2, 0);
    m_chargeLabel = new QLabel("-");
    infoLayout->addWidget(m_chargeLabel, 2, 1);

    infoLayout->addWidget(new QLabel("就诊时间:"), 2, 2);
    m_orderDateLabel = new QLabel("-");
    infoLayout->addWidget(m_orderDateLabel, 2, 3, 1, 3);

    rightLayout->addWidget(m_patientInfoGroup);

    // 消息区域
    m_messageGroup = new QGroupBox("咨询内容");
    QVBoxLayout *messageLayout = new QVBoxLayout(m_messageGroup);

    m_messageList = new QListWidget();
    m_messageList->setWordWrap(true);
    m_messageList->setSpacing(5);
    messageLayout->addWidget(m_messageList);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    m_messageInput = new QTextEdit();
    m_messageInput->setPlaceholderText("输入回复内容...");
    m_messageInput->setMaximumHeight(60);

    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setMinimumWidth(70);
    m_sendBtn->setMinimumHeight(35);
    m_sendBtn->setStyleSheet(
        "QPushButton { background-color: #07C160; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #06ae54; }"
        );

    m_moreBtn = new QPushButton("更多操作");
    m_moreBtn->setMinimumWidth(80);
    m_moreBtn->setMinimumHeight(35);
    m_moreBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; border: none; border-radius: 3px; }"
        "QPushButton:hover { background-color: #F57C00; }"
        );

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendBtn);
    inputLayout->addWidget(m_moreBtn);

    messageLayout->addLayout(inputLayout);

    rightLayout->addWidget(m_messageGroup);

    m_mainSplitter->addWidget(m_leftPanel);
    m_mainSplitter->addWidget(m_rightPanel);
    m_mainSplitter->setSizes(QList<int>() << 500 << 600);

    // 连接信号槽
    connect(m_consultTable, &QTableWidget::cellClicked, this, &ConsultMainWindow::onConsultSelected);
    connect(m_sendBtn, &QPushButton::clicked, this, &ConsultMainWindow::onSendMessage);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &ConsultMainWindow::onSearchChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ConsultMainWindow::onRefreshClicked);
    connect(m_moreBtn, &QPushButton::clicked, this, &ConsultMainWindow::onMoreButtonClicked);

    statusBar()->showMessage("就绪");
}

void ConsultMainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    QMenu *fileMenu = menuBar->addMenu("文件");
    QMenu *toolsMenu = menuBar->addMenu("工具");
    QMenu *helpMenu = menuBar->addMenu("帮助");

    QAction *refreshAction = new QAction("刷新", this);
    connect(refreshAction, &QAction::triggered, this, &ConsultMainWindow::onRefreshAction);
    fileMenu->addAction(refreshAction);

    fileMenu->addSeparator();

    QAction *blacklistAction = new QAction("黑名单管理", this);
    connect(blacklistAction, &QAction::triggered, this, &ConsultMainWindow::onViewBlacklist);
    toolsMenu->addAction(blacklistAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("退出", this);
    connect(exitAction, &QAction::triggered, this, &ConsultMainWindow::onExitAction);
    fileMenu->addAction(exitAction);

    QAction *aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, &ConsultMainWindow::onAboutAction);
    helpMenu->addAction(aboutAction);
}

void ConsultMainWindow::loadConsultRecords()
{
    m_consultRecords = m_dbManager->getConsultRecords(m_currentDoctor);

    m_consultTable->setRowCount(m_consultRecords.size());

    for (int i = 0; i < m_consultRecords.size(); ++i) {
        const ConsultRecord &record = m_consultRecords[i];

        m_consultTable->setItem(i, 0, new QTableWidgetItem(record.orderDate.toString("yyyy-MM-dd hh:mm")));
        m_consultTable->setItem(i, 1, new QTableWidgetItem(record.patientName));
        m_consultTable->setItem(i, 2, new QTableWidgetItem(record.patientCard));

        // 正确显示状态文本
        QString stateText;
        if (record.passFlag == "0") {
            stateText = "无效咨询";
        } else if (record.clinicState == "1") {
            stateText = "新申请";
        } else if (record.clinicState == "2") {
            stateText = "接收中";
        } else if (record.clinicState == "3") {
            stateText = "已结束";
        } else {
            stateText = "未知";
        }

        QTableWidgetItem *stateItem = new QTableWidgetItem(stateText);
        if (record.passFlag == "0") {
            stateItem->setForeground(Qt::red);
        } else if (record.clinicState == "3") {
            stateItem->setForeground(Qt::gray);
        }
        m_consultTable->setItem(i, 3, stateItem);

        m_consultTable->setItem(i, 4, new QTableWidgetItem(QString::number(record.charge, 'f', 2)));
        m_consultTable->setItem(i, 5, new QTableWidgetItem(record.orderId));

        m_lastMessageTime[record.orderId] = record.orderDate;
    }
}

void ConsultMainWindow::refreshCurrentDisplay()
{
    if (m_currentRecord.orderId.isEmpty()) return;

    // 从数据库重新获取最新状态
    ConsultRecord updated = m_dbManager->getConsultRecordByOrderId(m_currentRecord.orderId);
    if (!updated.orderId.isEmpty()) {
        m_currentRecord = updated;
    }

    // 更新患者信息显示
    m_patientNameLabel->setText(m_currentRecord.patientName);
    m_patientAgeLabel->setText(m_currentRecord.patientAge);

    QString sexText = (m_currentRecord.patientSex == "1") ? "男" : "女";
    m_patientSexLabel->setText(sexText);
    m_patientCardLabel->setText(m_currentRecord.patientCard);
    m_chargeLabel->setText(QString::number(m_currentRecord.charge, 'f', 2) + "元");
    m_orderDateLabel->setText(m_currentRecord.orderDate.toString("yyyy-MM-dd hh:mm"));

    // 更新状态显示（使用 trimmed 后的值比较）
    QString stateText;
    qDebug() << "刷新显示 - clinicState:" << m_currentRecord.clinicState << " passFlag:" << m_currentRecord.passFlag;

    if (m_currentRecord.passFlag == "0") {
        stateText = "无效咨询";
        m_consultStateLabel->setStyleSheet("color: red; font-weight: bold;");
    } else if (m_currentRecord.clinicState == "1") {
        stateText = "新申请";
        m_consultStateLabel->setStyleSheet("color: blue;");
    } else if (m_currentRecord.clinicState == "2") {
        stateText = "接收中";
        m_consultStateLabel->setStyleSheet("color: green;");
    } else if (m_currentRecord.clinicState == "3") {
        stateText = "已结束";
        m_consultStateLabel->setStyleSheet("color: gray;");
    } else {
        stateText = "未知";
        m_consultStateLabel->setStyleSheet("color: orange;");
    }
    m_consultStateLabel->setText(stateText);

    // 更新表格中对应的行
    for (int i = 0; i < m_consultTable->rowCount(); ++i) {
        if (m_consultTable->item(i, 5)->text() == m_currentRecord.orderId) {
            QString newStateText;
            if (m_currentRecord.passFlag == "0") {
                newStateText = "无效咨询";
                m_consultTable->item(i, 3)->setForeground(Qt::red);
            } else if (m_currentRecord.clinicState == "1") {
                newStateText = "新申请";
                m_consultTable->item(i, 3)->setForeground(Qt::blue);
            } else if (m_currentRecord.clinicState == "2") {
                newStateText = "接收中";
                m_consultTable->item(i, 3)->setForeground(Qt::darkGreen);
            } else if (m_currentRecord.clinicState == "3") {
                newStateText = "已结束";
                m_consultTable->item(i, 3)->setForeground(Qt::gray);
            } else {
                newStateText = "未知";
            }
            m_consultTable->item(i, 3)->setText(newStateText);
            break;
        }
    }
}

void ConsultMainWindow::loadMessages(const QString &orderId, const QString &patientName)
{
    m_messageList->clear();

    m_lastMessageTime[orderId] = QDateTime::currentDateTime();

    // 检查状态
    bool isBlocked = m_dbManager->isPatientBlocked(m_currentDoctor, patientName);
    bool isEnded = (m_currentRecord.clinicState == "3");
    bool isInvalid = (m_currentRecord.passFlag == "0");

    // 显示状态提示消息
    if (isBlocked) {
        addMessageToDisplay("系统", "⚠️ 该患者已被加入黑名单，无法发送消息", false);
    }
    if (isEnded) {
        addMessageToDisplay("系统", "✅ 该咨询已结束", false);
    }
    if (isInvalid) {
        addMessageToDisplay("系统", "❌ 该咨询已被标记为无效\n原因：" + m_currentRecord.invalidReason, false);
    }

    // 模拟历史消息
    addMessageToDisplay(patientName, "医生您好，我想咨询一下病情", false);
    addMessageToDisplay("我", "您好，请详细描述您的症状", true);
    addMessageToDisplay(patientName, "最近总是头痛，特别是早上起来的时候", false);

    m_messageList->scrollToBottom();
}

void ConsultMainWindow::addMessageToDisplay(const QString &sender, const QString &content, bool isOutgoing)
{
    QListWidgetItem *item = new QListWidgetItem(m_messageList);

    QString timeStr = QDateTime::currentDateTime().toString("hh:mm");
    QString displayText;

    if (isOutgoing) {
        displayText = QString("[%1] 我: %2").arg(timeStr).arg(content);
        item->setTextAlignment(Qt::AlignRight);
        item->setForeground(Qt::blue);
        item->setBackground(QColor(230, 242, 255));
    } else {
        if (sender == "系统") {
            displayText = QString("[%1] %2").arg(timeStr).arg(content);
            item->setForeground(Qt::red);
            item->setBackground(QColor(255, 240, 240));
        } else {
            displayText = QString("[%1] %2: %3").arg(timeStr).arg(sender).arg(content);
            item->setForeground(Qt::black);
            item->setBackground(QColor(245, 245, 245));
        }
    }

    item->setText(displayText);
    item->setSizeHint(QSize(item->sizeHint().width(), 50));
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
}

void ConsultMainWindow::onConsultSelected(int row, int column)
{
    Q_UNUSED(column);

    QString orderId = m_consultTable->item(row, 5)->text();

    for (const ConsultRecord &record : m_consultRecords) {
        if (record.orderId == orderId) {
            m_currentRecord = record;
            refreshCurrentDisplay();
            loadMessages(orderId, record.patientName);
            break;
        }
    }
}

bool ConsultMainWindow::canSendMessage()
{
    // 实时从数据库获取最新状态
    ConsultRecord latestRecord = m_dbManager->getConsultRecordByOrderId(m_currentRecord.orderId);

    qDebug() << "=== 发送消息检查 ===";
    qDebug() << "患者:" << latestRecord.patientName;
    qDebug() << "clinicState (trimmed):" << latestRecord.clinicState;
    qDebug() << "passFlag (trimmed):" << latestRecord.passFlag;

    // 检查黑名单
    if (m_dbManager->isPatientBlocked(m_currentDoctor, latestRecord.patientName)) {
        QMessageBox::warning(this, "无法发送", "该患者已在黑名单中，无法发送消息");
        return false;
    }

    // 检查是否已结束 (clinicState == "3")
    if (latestRecord.clinicState == "3") {
        QMessageBox::warning(this, "无法发送", "该咨询已结束，无法发送消息");
        return false;
    }

    // 检查是否无效 (passFlag == "0")
    if (latestRecord.passFlag == "0") {
        QMessageBox::warning(this, "无法发送", "该咨询已被标记为无效，无法发送消息");
        return false;
    }

    // 更新 m_currentRecord 为最新状态
    m_currentRecord = latestRecord;

    qDebug() << "允许发送";
    return true;
}


void ConsultMainWindow::onSendMessage()
{
    QString message = m_messageInput->toPlainText().trimmed();
    if (message.isEmpty()) return;

    // 发送前先检查是否可以发送（会实时查询数据库最新状态）
    if (!canSendMessage()) {
        // 刷新界面显示，让用户看到最新状态
        refreshCurrentDisplay();
        return;
    }

    m_lastMessageTime[m_currentRecord.orderId] = QDateTime::currentDateTime();
    addMessageToDisplay("我", message, true);
    m_messageInput->clear();

    // 模拟自动回复
    QTimer::singleShot(1000, [this]() {
        // 发送后再次检查状态，防止在回复期间状态改变
        if (m_currentRecord.clinicState != "3" && m_currentRecord.passFlag != "0") {
            addMessageToDisplay(m_currentRecord.patientName, "收到您的回复，谢谢医生", false);
            m_lastMessageTime[m_currentRecord.orderId] = QDateTime::currentDateTime();
        }
    });
}

void ConsultMainWindow::onSearchChanged(const QString &text)
{
    QString state = m_stateFilter->currentData().toString();

    for (int i = 0; i < m_consultTable->rowCount(); ++i) {
        QString patientName = m_consultTable->item(i, 1)->text();
        QString cardNo = m_consultTable->item(i, 2)->text();
        QString rowState = m_consultTable->item(i, 3)->text();

        bool matchText = text.isEmpty() ||
                         patientName.contains(text, Qt::CaseInsensitive) ||
                         cardNo.contains(text);

        bool matchState = state.isEmpty() ||
                          (state == "1" && rowState == "新申请") ||
                          (state == "2" && rowState == "接收中") ||
                          (state == "3" && (rowState == "已结束" || rowState == "无效咨询"));

        m_consultTable->setRowHidden(i, !(matchText && matchState));
    }
}

void ConsultMainWindow::onRefreshClicked()
{
    loadConsultRecords();
    if (!m_currentRecord.orderId.isEmpty()) {
        refreshCurrentDisplay();
        loadMessages(m_currentRecord.orderId, m_currentRecord.patientName);
    }
    statusBar()->showMessage("数据已刷新", 2000);
}

void ConsultMainWindow::onMoreButtonClicked()
{
    QMenu menu(this);

    QAction *blacklistAction = menu.addAction("移入黑名单");
    QAction *endAction = menu.addAction("结束咨询");
    QAction *invalidAction = menu.addAction("无效咨询");

    menu.addSeparator();
    QAction *viewBlacklistAction = menu.addAction("查看黑名单");

    connect(blacklistAction, &QAction::triggered, this, &ConsultMainWindow::onAddToBlacklist);
    connect(endAction, &QAction::triggered, this, &ConsultMainWindow::onEndConsult);
    connect(invalidAction, &QAction::triggered, this, &ConsultMainWindow::onInvalidConsult);
    connect(viewBlacklistAction, &QAction::triggered, this, &ConsultMainWindow::onViewBlacklist);

    menu.exec(m_moreBtn->mapToGlobal(QPoint(0, m_moreBtn->height())));
}

void ConsultMainWindow::onAddToBlacklist()
{
    if (m_currentRecord.patientName.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("移入黑名单");
    dialog.resize(400, 200);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel(QString("确定要将患者 %1 移入黑名单吗？\n移入后该患者将无法再向您发送消息。")
                                   .arg(m_currentRecord.patientName));
    label->setWordWrap(true);
    layout->addWidget(label);

    QLabel *reasonLabel = new QLabel("加入原因:");
    layout->addWidget(reasonLabel);

    QLineEdit *reasonEdit = new QLineEdit();
    reasonEdit->setPlaceholderText("请输入加入黑名单的原因");
    layout->addWidget(reasonEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定");
    QPushButton *cancelBtn = new QPushButton("取消");

    okBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    cancelBtn->setStyleSheet("QPushButton { background-color: #9E9E9E; color: white; }");

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString reason = reasonEdit->text().trimmed();
        if (reason.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入加入原因");
            return;
        }

        QString doctorId = m_dbManager->getDoctorId(m_currentDoctor);
        QString patientId = m_dbManager->getPatientId(m_currentRecord.patientName);

        if (m_dbManager->addToBlacklist(doctorId, patientId, reason)) {
            QMessageBox::information(this, "操作成功", QString("已将 %1 移入黑名单").arg(m_currentRecord.patientName));
            refreshCurrentDisplay();
            addMessageToDisplay("系统", "⚠️ 该患者已被加入黑名单", false);
        } else {
            QMessageBox::critical(this, "操作失败", m_dbManager->getLastError());
        }
    }
}

void ConsultMainWindow::onEndConsult()
{
    if (m_currentRecord.orderId.isEmpty()) return;

    int ret = QMessageBox::question(this, "确认结束",
                                    QString("确定要结束与 %1 的咨询吗？\n结束后将无法发送消息。")
                                        .arg(m_currentRecord.patientName),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_dbManager->endConsult(m_currentRecord.orderId)) {
            QMessageBox::information(this, "操作成功", "咨询已结束");

            // 重要：重新从数据库获取最新数据，更新 m_currentRecord
            m_currentRecord = m_dbManager->getConsultRecordByOrderId(m_currentRecord.orderId);

            // 刷新界面显示
            refreshCurrentDisplay();

            // 在消息列表中添加系统提示
            addMessageToDisplay("系统", "✅ 该咨询已结束，无法再发送消息", false);

            // 更新最后消息时间
            m_lastMessageTime.remove(m_currentRecord.orderId);

            // 刷新左侧表格
            loadConsultRecords();
        } else {
            QMessageBox::critical(this, "操作失败", m_dbManager->getLastError());
        }
    }
}

void ConsultMainWindow::onInvalidConsult()
{
    if (m_currentRecord.orderId.isEmpty()) return;

    InvalidConsultDialog dialog(m_currentRecord.patientName, this);

    if (dialog.exec() == QDialog::Accepted) {
        QString reason = dialog.getReason();

        if (m_dbManager->markInvalidConsult(m_currentRecord.orderId, reason)) {
            QMessageBox::information(this, "操作成功", QString("已将咨询标记为无效\n原因：%1").arg(reason));

            // 重要：重新从数据库获取最新数据，更新 m_currentRecord
            m_currentRecord = m_dbManager->getConsultRecordByOrderId(m_currentRecord.orderId);

            // 刷新界面显示
            refreshCurrentDisplay();

            // 在消息列表中添加系统提示
            addMessageToDisplay("系统", QString("❌ 该咨询已被标记为无效，无法再发送消息\n原因：%1").arg(reason), false);

            // 更新最后消息时间
            m_lastMessageTime.remove(m_currentRecord.orderId);

            // 刷新左侧表格
            loadConsultRecords();
        } else {
            QMessageBox::critical(this, "操作失败", m_dbManager->getLastError());
        }
    }
}

void ConsultMainWindow::onViewBlacklist()
{
    BlacklistDialog dialog(m_currentDoctor, m_dbManager, this);

    connect(&dialog, &BlacklistDialog::blacklistChanged, [this]() {
        loadConsultRecords();
        if (!m_currentRecord.orderId.isEmpty()) {
            refreshCurrentDisplay();
            loadMessages(m_currentRecord.orderId, m_currentRecord.patientName);
        }
    });

    dialog.exec();
}

void ConsultMainWindow::onRefreshAction()
{
    onRefreshClicked();
}

void ConsultMainWindow::onExitAction()
{
    close();
}

void ConsultMainWindow::onAboutAction()
{
    QMessageBox::about(this, "关于",
                       "在线问诊系统医生端 v1.0\n\n"
                       "严格按照在线问诊系统方案设计\n"
                       "包含黑名单、结束咨询、无效咨询功能");
}

void ConsultMainWindow::checkInactiveConsults()
{
    QDateTime now = QDateTime::currentDateTime();

    for (auto it = m_lastMessageTime.begin(); it != m_lastMessageTime.end(); ++it) {
        QString orderId = it.key();
        QDateTime lastTime = it.value();

        if (lastTime.secsTo(now) > 1800) {  // 30分钟无消息
            for (int i = 0; i < m_consultRecords.size(); ++i) {
                if (m_consultRecords[i].orderId == orderId && m_consultRecords[i].clinicState != "3") {
                    if (m_dbManager->endConsult(orderId)) {
                        qDebug() << "自动结束咨询:" << orderId;
                        m_consultRecords[i].clinicState = "3";

                        // 更新表格
                        for (int j = 0; j < m_consultTable->rowCount(); ++j) {
                            if (m_consultTable->item(j, 5)->text() == orderId) {
                                m_consultTable->item(j, 3)->setText("已结束");
                                m_consultTable->item(j, 3)->setForeground(Qt::gray);
                                break;
                            }
                        }

                        if (m_currentRecord.orderId == orderId) {
                            m_currentRecord.clinicState = "3";
                            m_consultStateLabel->setText("已结束（自动）");
                            addMessageToDisplay("系统", "✅ 咨询已自动结束（30分钟无消息）", false);
                        }

                        m_lastMessageTime.remove(orderId);
                    }
                    break;
                }
            }
        }
    }
}

QString ConsultMainWindow::getStateText(const QString &state)
{
    if (state == "1") return "新申请";
    if (state == "2") return "接收中";
    if (state == "3") return "已结束";
    return "未知";
}

void ConsultMainWindow::closeEvent(QCloseEvent *event)
{
    m_autoEndTimer->stop();
    m_dbManager->closeDatabase();
    event->accept();
}
