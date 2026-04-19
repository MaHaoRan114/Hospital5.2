#include "ExamOrderWidget.h"
#include "ui_ExamOrderWidget.h"
#include "../database/DatabaseManager.h"
#include <QMessageBox>

ExamOrderWidget::ExamOrderWidget(const QString &consultId,
                                 const QString &doctorId,
                                 const QString &patientId,
                                 QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExamOrderWidget)
    , m_consultId(consultId)
    , m_doctorId(doctorId)
    , m_patientId(patientId)
{
    ui->setupUi(this);
    setupConnections();

    // 初始化检查科室选项
    ui->comboExamDept->addItems({"放射科", "超声科", "检验科", "心电图室",
                                  "内镜室", "病理科", "核医学科"});
}

ExamOrderWidget::~ExamOrderWidget()
{
    delete ui;
}

void ExamOrderWidget::setupConnections()
{
    connect(ui->btnSubmit, &QPushButton::clicked, this, &ExamOrderWidget::onSubmitClicked);
    connect(ui->btnClear,  &QPushButton::clicked, this, &ExamOrderWidget::onClearClicked);
}

void ExamOrderWidget::onSubmitClicked()
{
    if (!validateForm()) return;

    OrderModel order = collectFormData();
    if (DatabaseManager::instance()->submitExamOrder(order)) {
        QMessageBox::information(this, "成功", "检查医嘱开具成功！");
        emit orderSubmitted(order);
        onClearClicked();
    } else {
        QMessageBox::critical(this, "失败", "检查医嘱提交失败，请检查网络连接。");
    }
}

void ExamOrderWidget::onClearClicked()
{
    ui->editExamName->clear();
    ui->editExamCode->clear();
    ui->editExamNote->clear();
    ui->comboExamDept->setCurrentIndex(0);
}

bool ExamOrderWidget::validateForm()
{
    if (ui->editExamName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入检查项目名称");
        ui->editExamName->setFocus();
        return false;
    }
    return true;
}

OrderModel ExamOrderWidget::collectFormData()
{
    OrderModel order;
    order.consultId  = m_consultId;
    order.doctorId   = m_doctorId;
    order.patientId  = m_patientId;
    order.orderType  = 2;
    order.orderStatus= 0;
    order.examName   = ui->editExamName->text().trimmed();
    order.examCode   = ui->editExamCode->text().trimmed();
    order.examDept   = ui->comboExamDept->currentText();
    order.examNote   = ui->editExamNote->toPlainText().trimmed();
    return order;
}
