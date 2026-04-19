#include "DrugOrderWidget.h"
#include "ui_DrugOrderWidget.h"
#include "../database/DatabaseManager.h"
#include <QMessageBox>
#include <QDoubleValidator>

DrugOrderWidget::DrugOrderWidget(const QString &consultId,
                                 const QString &doctorId,
                                 const QString &patientId,
                                 QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DrugOrderWidget)
    , m_consultId(consultId)
    , m_doctorId(doctorId)
    , m_patientId(patientId)
{
    ui->setupUi(this);
    setupConnections();

    // 初始化下拉框选项
    ui->comboAdministration->addItems({"口服", "静脉注射", "肌肉注射", "外用", "吸入"});
    ui->comboFrequency->addItems({"每日一次(QD)", "每日两次(BID)", "每日三次(TID)",
                                   "每隔8小时(Q8H)", "必要时(PRN)", "顿服(ST)"});
    ui->comboDosageUnits->addItems({"mg", "g", "ml", "片", "粒", "支", "贴"});
    ui->comboUnits->addItems({"盒", "瓶", "支", "片", "粒"});
    ui->comboDrugFlag->addItems({"取药", "不取药"});

    // 数值校验
    ui->editAmount->setValidator(new QDoubleValidator(0.01, 9999.99, 2, this));
    ui->editDosage->setValidator(new QDoubleValidator(0.01, 9999.99, 2, this));
}

DrugOrderWidget::~DrugOrderWidget()
{
    delete ui;
}

void DrugOrderWidget::setupConnections()
{
    connect(ui->btnSubmit, &QPushButton::clicked, this, &DrugOrderWidget::onSubmitClicked);
    connect(ui->btnClear,  &QPushButton::clicked, this, &DrugOrderWidget::onClearClicked);
    connect(ui->chkProvided, &QCheckBox::toggled, this, &DrugOrderWidget::onProvidedToggled);
}

void DrugOrderWidget::onSubmitClicked()
{
    if (!validateForm()) return;

    OrderModel order = collectFormData();
    if (DatabaseManager::instance()->submitDrugOrder(order)) {
        QMessageBox::information(this, "成功", "药品医嘱开具成功！");
        emit orderSubmitted(order);
        onClearClicked();
    } else {
        QMessageBox::critical(this, "失败", "药品医嘱提交失败，请检查网络连接。");
    }
}

void DrugOrderWidget::onClearClicked()
{
    ui->editDrugName->clear();
    ui->editSpec->clear();
    ui->editAmount->clear();
    ui->editDosage->clear();
    ui->editFreqDetail->clear();
    ui->spinRepetition->setValue(1);
    ui->chkProvided->setChecked(false);
    ui->comboAdministration->setCurrentIndex(0);
    ui->comboFrequency->setCurrentIndex(0);
}

void DrugOrderWidget::onProvidedToggled(bool checked)
{
    // 自备药时，隐藏摆药药局选项
    ui->labelDispensary->setVisible(!checked);
    ui->editDispensary->setVisible(!checked);
}

bool DrugOrderWidget::validateForm()
{
    if (ui->editDrugName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入药品名称");
        ui->editDrugName->setFocus();
        return false;
    }
    if (ui->editAmount->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入药品数量");
        ui->editAmount->setFocus();
        return false;
    }
    if (ui->editDosage->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入单次用量");
        ui->editDosage->setFocus();
        return false;
    }
    return true;
}

OrderModel DrugOrderWidget::collectFormData()
{
    OrderModel order;
    order.consultId        = m_consultId;
    order.doctorId         = m_doctorId;
    order.patientId        = m_patientId;
    order.orderType        = 1;
    order.orderStatus      = 0;

    order.drugName         = ui->editDrugName->text().trimmed();
    order.spec             = ui->editSpec->text().trimmed();
    order.units            = ui->comboUnits->currentText();
    order.amount           = ui->editAmount->text().toDouble();
    order.dosage           = ui->editDosage->text().toDouble();
    order.dosageUnits      = ui->comboDosageUnits->currentText();
    order.administration   = ui->comboAdministration->currentText();
    order.frequency        = ui->comboFrequency->currentText();
    order.providedIndicator= ui->chkProvided->isChecked();
    order.dispensary       = ui->editDispensary->text().trimmed();
    order.repetition       = ui->spinRepetition->value();
    order.freqDetail       = ui->editFreqDetail->toPlainText().trimmed();
    order.getDrugFlag      = ui->comboDrugFlag->currentIndex() == 0 ? "1" : "2";

    return order;
}
