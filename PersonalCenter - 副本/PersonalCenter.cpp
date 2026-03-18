#include "PersonalCenter.h"
#include "DataManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView> 
#include <QCheckBox>
#include <QComboBox>
#include <QSpacerItem>
#include <QMessageBox>

PersonalCenter::PersonalCenter(QWidget* parent) : QWidget(parent) {
    mainTabs = new QTabWidget(this);
    this->resize(1000, 700);

    // 按照计划书模块添加主页签 [cite: 1521-1523, 1564-1579]
    mainTabs->addTab(createSettingPage(), "设置");
    mainTabs->addTab(createIncomePage(), "我的收入");
    mainTabs->addTab(createOrderPage(), "医嘱记录");
    mainTabs->addTab(createBlacklistPage(), "黑名单管理");
    mainTabs->addTab(createRefundPage(), "退款审核");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainTabs);
    this->setLayout(mainLayout);
    this->setStyleSheet(
        // 1. 设置主背景色为浅灰色
        "PersonalCenter { background-color: #f5f5f5; }"

        // 2. 美化标签页（QTabWidget）
        "QTabWidget::pane { border: 1px solid #dcdcdc; background: white; }"
        "QTabBar::tab { background: #eeeeee; padding: 10px 20px; border: 1px solid #dcdcdc; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:selected { background: white; border-bottom: 2px solid #07c160; color: #07c160; font-weight: bold; }" // 经典的微信绿

        // 3. 美化输入框
        "QLineEdit { border: 1px solid #dcdcdc; border-radius: 4px; padding: 5px; background: white; }"
        "QLineEdit:focus { border: 1px solid #07c160; }"

        // 4. 美化按钮
        "QPushButton { background-color: white; border: 1px solid #dcdcdc; border-radius: 4px; padding: 6px 15px; min-width: 60px; }"
        "QPushButton:hover { background-color: #f2f2f2; border-color: #07c160; color: #07c160; }"
        "QPushButton#blueBtn { background-color: #07c160; color: white; border: none; font-weight: bold; }" // 突出按钮
        "QPushButton#blueBtn:hover { background-color: #06ae56; }"

        // 5. 表格美化 [cite: 1614-1622]
        "QTableWidget { gridline-color: #f0f0f0; border: none; selection-background-color: #e8f7ed; }"
        "QHeaderView::section { background-color: #fafafa; border: none; border-bottom: 1px solid #dcdcdc; padding: 5px; color: #666; font-weight: bold; }"
    );
}

// 1. 接诊设置页 [cite: 1524, 1566-1572]
QWidget* PersonalCenter::createSettingPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(30, 30, 30, 30); // 设置页边距
    mainLayout->setSpacing(20); // 设置控件间距

    // 1. 三个核心开关设置 [cite: 3265, 3289, 3294]
    QCheckBox* checkGraphic = new QCheckBox("接受图文咨询");
    QCheckBox* checkPhone = new QCheckBox("接受电话咨询");
    QCheckBox* checkAutoEnd = new QCheckBox("自动结束订单");

    // 设置初始状态（根据需要可从数据库 users 表读取 drug_falg 等字段） 
    mainLayout->addWidget(checkGraphic);
    mainLayout->addWidget(checkPhone);
    mainLayout->addWidget(checkAutoEnd);

    // 2. 上线时间说明 [cite: 3293]
    QHBoxLayout* timeLayout = new QHBoxLayout();
    timeLayout->addWidget(new QLabel("上线时间说明"));
    QLineEdit* timeEdit = new QLineEdit();
    timeEdit->setFixedWidth(150);
    timeEdit->setPlaceholderText("例如：每天18:00");
    timeLayout->addWidget(timeEdit);
    timeLayout->addStretch(); // 推向左侧
    mainLayout->addLayout(timeLayout);

    // 3. 微信消息提醒设置 [cite: 3292]
    QHBoxLayout* msgLayout = new QHBoxLayout();
    msgLayout->addWidget(new QLabel("微信消息提醒"));
    QComboBox* msgCombo = new QComboBox();
    msgCombo->addItem("所有咨询及时推送"); // 对应方案中的设置项 [cite: 3298]
    msgCombo->addItem("仅重要咨询推送");
    msgCombo->setFixedWidth(150);
    msgLayout->addWidget(msgCombo);
    msgLayout->addStretch();
    mainLayout->addLayout(msgLayout);

    // 4. CA认证状态显示 [cite: 3296-3297]
    QLabel* caStatusLabel = new QLabel("CA认证状态：未认证");
    // 后期可根据数据库 examine_falg 状态动态修改文字 
    mainLayout->addWidget(caStatusLabel);

    // 5. 底部弹簧，将所有内容推向顶部
    mainLayout->addStretch();

    page->setLayout(mainLayout);
    return page;
}

// 2. 我的收入页 [cite: 1525, 1573-1579]
QWidget* PersonalCenter::createIncomePage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    QLabel* totalLabel = new QLabel("¥ 0.01"); // 对应截图金额 [cite: 1578]
    totalLabel->setStyleSheet("font-size: 36px; color: red; font-weight: bold;");
    layout->addWidget(new QLabel("本月总收入 (元)"));
    layout->addWidget(totalLabel);
    layout->addStretch();
    return page;
}

// 3. 医嘱记录页：完善“药品”与“检验”双页签 [cite: 1581, 1591-1592]
QWidget* PersonalCenter::createOrderPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    QTabWidget* subTabs = new QTabWidget();

    // 1. 创建药品医嘱表格
    QTableWidget* drugTable = new QTableWidget(0, 5);
    drugTable->setHorizontalHeaderLabels({ "开单时间", "患者姓名", "性别", "年龄", "历史诊断" });
    drugTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 2. 从 DataManager 获取数据并填充
    auto& data = DataManager::instance().patients;
    for (int i = 0; i < data.size(); ++i) {
        drugTable->insertRow(i);
        drugTable->setItem(i, 0, new QTableWidgetItem("2026-03-16")); // 模拟当前日期
        drugTable->setItem(i, 1, new QTableWidgetItem(data[i].name));
        drugTable->setItem(i, 2, new QTableWidgetItem(data[i].gender));
        drugTable->setItem(i, 3, new QTableWidgetItem(QString::number(data[i].age)));
        drugTable->setItem(i, 4, new QTableWidgetItem(data[i].lastDIAG));
    }

    // 3. 将表格加入子标签页
    subTabs->addTab(drugTable, "药品医嘱");
    subTabs->addTab(new QWidget(), "检验医嘱"); // 预留检验医嘱

    layout->addWidget(subTabs);
    return page;
}

// 4. 黑名单管理页：完善“过滤器”切换逻辑
QWidget* PersonalCenter::createBlacklistPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    // --- 顶部：搜索与过滤切换区 ---
    QHBoxLayout* topLayout = new QHBoxLayout();

    this->blacklistSearchEdit = new QLineEdit();
    this->blacklistSearchEdit->setPlaceholderText("输入患者姓名进行搜索...");

    QPushButton* queryBtn = new QPushButton("查询");

    // 关键改动：将“新增”按钮改为“过滤器”按钮
    QPushButton* filterBtn = new QPushButton("只看黑名单");
    filterBtn->setCheckable(true); // 让按钮可以像开关一样保持按下状态
    filterBtn->setMinimumWidth(100);

    topLayout->addWidget(this->blacklistSearchEdit);
    topLayout->addWidget(queryBtn);
    topLayout->addStretch();
    topLayout->addWidget(filterBtn);

    // --- 中部：患者管理表格 ---
    this->blacklistTable = new QTableWidget(0, 5);
    this->blacklistTable->setHorizontalHeaderLabels({ "患者姓名", "性别", "年龄", "历史诊断", "操作" });
    this->blacklistTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- 信号与槽连接 ---
    // 连线 1：点击查询按钮
    connect(queryBtn, &QPushButton::clicked, this, &PersonalCenter::onSearchClicked);

    // 连线 2：点击过滤器按钮（Lambda表达式实现逻辑切换）
    connect(filterBtn, &QPushButton::toggled, [=](bool checked) {
        if (checked) {
            filterBtn->setText("显示全部");
            filterBtn->setStyleSheet("background-color: #e8f7ed; color: #07c160; font-weight: bold;");
        }
        else {
            filterBtn->setText("只看黑名单");
            filterBtn->setStyleSheet("");
        }
        // 调用刷新函数，传入当前是否处于“只看黑名单”模式
        this->refreshBlacklistTable(checked);
        });

    layout->addLayout(topLayout);
    layout->addWidget(this->blacklistTable);

    // 初始化：默认显示所有医治过的患者
    refreshBlacklistTable(false);

    return page;
}
QWidget* PersonalCenter::createRefundPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);

    // 1. 顶部提示信息 [cite: 1663]
    QLabel* notice = new QLabel("你有一笔退款申请，请处理");
    notice->setStyleSheet("color: #fa5151; font-weight: bold; padding: 10px; background: #fff1f0; border-radius: 4px;");
    layout->addWidget(notice);

    // 2. 退款列表表格 [cite: 1664, 1685-1687]
    QTableWidget* refundTable = new QTableWidget(1, 5);
    refundTable->setHorizontalHeaderLabels({ "申请时间", "患者姓名", "退款金额", "退款原因", "操作" });
    refundTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 模拟一行数据 [cite: 1676, 1689-1690]
    refundTable->setItem(0, 0, new QTableWidgetItem("2017-11-28"));
    refundTable->setItem(0, 1, new QTableWidgetItem("张海龙"));
    refundTable->setItem(0, 2, new QTableWidgetItem("¥ 0.01"));
    refundTable->setItem(0, 3, new QTableWidgetItem("没有解决问题"));

    // 3. 操作按钮：驳回 & 通过 [cite: 1688, 1692]
    QWidget* btnGroup = new QWidget();
    QHBoxLayout* hLayout = new QHBoxLayout(btnGroup);
    QPushButton* rejectBtn = new QPushButton("驳回");
    QPushButton* approveBtn = new QPushButton("通过");

    // 给按钮上色 [cite: 1692]
    approveBtn->setObjectName("blueBtn"); // 复用之前的微信绿样式
    rejectBtn->setStyleSheet("color: #666;");

    hLayout->addWidget(rejectBtn);
    hLayout->addWidget(approveBtn);
    hLayout->setContentsMargins(0, 0, 0, 0);
    refundTable->setCellWidget(0, 4, btnGroup);

    layout->addWidget(refundTable);
    return page;
}


// --- 黑名单逻辑的核心修正版 ---

void PersonalCenter::refreshBlacklistTable(bool showOnlyBlacklisted) {
    if (!this->blacklistTable) return;
    this->blacklistTable->setRowCount(0);

    auto& data = DataManager::instance().patients;

    // 获取搜索框内容实现搜索过滤
    QString filterText = this->blacklistSearchEdit->text().trimmed();

    int row = 0;
    for (int i = 0; i < data.size(); ++i) {
        // --- 修正1：实现“只看黑名单”过滤逻辑 ---
        if (showOnlyBlacklisted && !data[i].isBlacklisted) continue;

        // --- 修正2：实现“搜索框”过滤逻辑 ---
        if (!filterText.isEmpty() && !data[i].name.contains(filterText)) continue;

        this->blacklistTable->insertRow(row);
        this->blacklistTable->setItem(row, 0, new QTableWidgetItem(data[i].name));
        this->blacklistTable->setItem(row, 1, new QTableWidgetItem(data[i].gender));
        this->blacklistTable->setItem(row, 2, new QTableWidgetItem(QString::number(data[i].age)));
        this->blacklistTable->setItem(row, 3, new QTableWidgetItem(data[i].lastDIAG));

        QPushButton* actionBtn = new QPushButton();
        if (data[i].isBlacklisted) {
            actionBtn->setText("移除");
            actionBtn->setStyleSheet("color: #fa5151; border: 1px solid #fa5151;"); // 警示红
            connect(actionBtn, &QPushButton::clicked, this, &PersonalCenter::onRemoveBlacklistClicked);
        }
        else {
            actionBtn->setText("加入黑名单");
            actionBtn->setStyleSheet("color: #07c160; border: 1px solid #07c160;"); // 微信绿
            connect(actionBtn, &QPushButton::clicked, this, &PersonalCenter::onAddBlacklistClicked);
        }

        this->blacklistTable->setCellWidget(row, 4, actionBtn);
        row++;
    }
}

// 修正3：搜索按钮现在能真正过滤名字了
void PersonalCenter::onSearchClicked() {
    // 默认按照当前状态刷新，不强制改变“只看黑名单”的开关
    refreshBlacklistTable(false);
}

// 【加入黑名单】修正：确保状态切换后 UI 刷新
void PersonalCenter::onAddBlacklistClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int row = this->blacklistTable->indexAt(btn->pos()).row();
    QString name = this->blacklistTable->item(row, 0)->text();

    for (auto& p : DataManager::instance().patients) {
        if (p.name == name) {
            p.isBlacklisted = true;
            break;
        }
    }
    // 保持当前的搜索/过滤状态刷新
    refreshBlacklistTable(false);
}

// 【移除黑名单】修正：确保状态切换后 UI 刷新
void PersonalCenter::onRemoveBlacklistClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int row = this->blacklistTable->indexAt(btn->pos()).row();
    QString name = this->blacklistTable->item(row, 0)->text();

    for (auto& p : DataManager::instance().patients) {
        if (p.name == name) {
            p.isBlacklisted = false;
            break;
        }
    }
    refreshBlacklistTable(false);
}