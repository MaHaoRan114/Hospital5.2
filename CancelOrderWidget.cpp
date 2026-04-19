#include "CancelOrderWidget.h"
#include "ui_CancelOrderWidget.h"
#include "../database/DatabaseManager.h"
#include <QMessageBox>

CancelOrderWidget::CancelOrderWidget(const QString &consultId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CancelOrderWidget)
    , m_consultId(consultId)
{
    ui->setupUi(this);
    setupConnections();

    // 设置表格列
    ui->tableOrders->setColumnCount(4);
    ui->tableOrders->setHorizontalHeaderLabels({"医嘱ID", "类型", "医嘱内容", "开具时间"});
    ui->tableOrders->horizontalHeader()->setStretchLastSection(true);
    ui->tableOrders->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableOrders->setEditTriggers(QAbstractItemView::NoEditTriggers);

    loadOrders();
}

CancelOrderWidget::~CancelOrderWidget()
{
    delete ui;
}

void CancelOrderWidget::setupConnections()
{
    connect(ui->btnCancel,  &QPushButton::clicked,
            this, &CancelOrderWidget::onCancelClicked);
    connect(ui->tableOrders, &QTableWidget::itemSelectionChanged,
            this, &CancelOrderWidget::onOrderSelectionChanged);
    connect(ui->btnRefresh, &QPushButton::clicked,
            this, &CancelOrderWidget::loadOrders);
}

void CancelOrderWidget::loadOrders()
{
    m_orders = DatabaseManager::instance()->getActiveOrders(m_consultId);
    populateTable(m_orders);
    ui->btnCancel->setEnabled(false);
}

void CancelOrderWidget::populateTable(const QList<OrderModel> &orders)
{
    ui->tableOrders->setRowCount(0);
    for (const auto &o : orders) {
        int row = ui->tableOrders->rowCount();
        ui->tableOrders->insertRow(row);
        ui->tableOrders->setItem(row, 0, new QTableWidgetItem(o.orderId));
        ui->tableOrders->setItem(row, 1, new QTableWidgetItem(o.orderType == 1 ? "药品医嘱" : "检查医嘱"));
        ui->tableOrders->setItem(row, 2, new QTableWidgetItem(o.orderType == 1 ? o.drugName : o.examName));
        ui->tableOrders->setItem(row, 3, new QTableWidgetItem(o.createTime.toString("yyyy-MM-dd hh:mm")));
    }
}

void CancelOrderWidget::onOrderSelectionChanged()
{
    ui->btnCancel->setEnabled(!ui->tableOrders->selectedItems().isEmpty());
}

void CancelOrderWidget::onCancelClicked()
{
    int row = ui->tableOrders->currentRow();
    if (row < 0) return;

    QString orderId    = ui->tableOrders->item(row, 0)->text();
    QString orderName  = ui->tableOrders->item(row, 2)->text();

    // 二次确认
    auto reply = QMessageBox::question(
        this, "确认取消",
        QString("确定要取消医嘱【%1】吗？\n取消后无法恢复。").arg(orderName),
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) return;

    QString reason = ui->editCancelReason->text().trimmed();
    if (reason.isEmpty()) {
        QMessageBox::warning(this, "提示", "请填写取消原因");
        ui->editCancelReason->setFocus();
        return;
    }

    if (DatabaseManager::instance()->cancelOrder(orderId, reason)) {
        QMessageBox::information(this, "成功", "医嘱已成功取消");
        emit orderCancelled(orderId);
        ui->editCancelReason->clear();
        loadOrders();  // 刷新列表
    } else {
        QMessageBox::critical(this, "失败", "取消医嘱失败，请重试");
    }
}

void CancelOrderWidget::refreshOrderList()
{
    loadOrders();
}
