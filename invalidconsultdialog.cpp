#include "invalidconsultdialog.h"
#include <QMessageBox>

InvalidConsultDialog::InvalidConsultDialog(const QString &patientName, QWidget *parent)
    : QDialog(parent)
{
    setupUI(patientName);
    setWindowTitle("无效咨询");
    resize(450, 300);
}

InvalidConsultDialog::~InvalidConsultDialog()
{
}

void InvalidConsultDialog::setupUI(const QString &patientName)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("标记无效咨询");
    QFont font = titleLabel->font();
    font.setBold(true);
    font.setPointSize(12);
    titleLabel->setFont(font);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QLabel *infoLabel = new QLabel(
        "将本次咨询标记为无效后，系统将自动为患者退款。\n"
        "请填写无效原因，以便患者了解情况。"
        );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; padding: 10px; background-color: #f5f5f5; border-radius: 5px;");
    mainLayout->addWidget(infoLabel);

    QLabel *patientLabel = new QLabel(QString("患者：%1").arg(patientName));
    patientLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(patientLabel);

    QLabel *reasonLabel = new QLabel("无效原因：");
    mainLayout->addWidget(reasonLabel);

    m_reasonEdit = new QTextEdit();
    m_reasonEdit->setPlaceholderText("请详细说明本次咨询无效的原因...");
    m_reasonEdit->setMinimumHeight(100);
    mainLayout->addWidget(m_reasonEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *okBtn = new QPushButton("确定");
    QPushButton *cancelBtn = new QPushButton("取消");

    okBtn->setMinimumHeight(35);
    cancelBtn->setMinimumHeight(35);

    okBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #da190b;"
        "}"
        );

    cancelBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #9E9E9E;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "   min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #757575;"
        "}"
        );

    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    mainLayout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, this, &InvalidConsultDialog::onOkClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &InvalidConsultDialog::onCancelClicked);
}

void InvalidConsultDialog::onOkClicked()
{
    QString reason = m_reasonEdit->toPlainText().trimmed();
    if (reason.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入无效原因");
        return;
    }

    m_reason = reason;
    accept();
}

void InvalidConsultDialog::onCancelClicked()
{
    reject();
}

QString InvalidConsultDialog::getReason() const
{
    return m_reason;
}
