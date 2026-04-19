#include "dbmanager.h"

DbManager::DbManager(QObject *parent) : QObject(parent)
{
    m_database = QSqlDatabase::addDatabase("QODBC");
}

DbManager::~DbManager()
{
    closeDatabase();
}

bool DbManager::connectToDatabase()
{
    QString connectionString = QString(
        "DRIVER={SQL Server};"
        "SERVER=LAPTOP-M8OP3I62\\SQLEXPRESS;"
        "DATABASE=JKDB;"
        "Trusted_Connection=yes;"
        );
    m_database.setDatabaseName(connectionString);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qDebug() << "数据库连接失败:" << m_lastError;
        return false;
    }

    qDebug() << "数据库连接成功!";
    return true;
}

void DbManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "数据库连接已关闭";
    }
}

bool DbManager::isOpen() const
{
    return m_database.isOpen();
}

QString DbManager::getLastError() const
{
    return m_lastError;
}

QString DbManager::getDoctorId(const QString &doctorName)
{
    QString doctorId;
    QSqlQuery query;
    query.prepare("SELECT user_id FROM users WHERE username = :username");
    query.bindValue(":username", doctorName);

    if (query.exec() && query.next()) {
        doctorId = query.value(0).toString();
    }
    return doctorId;
}

QString DbManager::getPatientId(const QString &patientName)
{
    QString patientId;
    QSqlQuery query;
    query.prepare("SELECT patient_id FROM patient WHERE patient_name = :name");
    query.bindValue(":name", patientName);

    if (query.exec() && query.next()) {
        patientId = query.value(0).toString();
    }
    return patientId;
}

QList<ConsultRecord> DbManager::getConsultRecords(const QString &doctorName)
{
    QList<ConsultRecord> records;

    QSqlQuery query;
    query.prepare(
        "SELECT o.order_id, o.clinic_id, o.serial_no, o.patient_name, "
        "o.patient_card, o.dept_name, o.username, o.order_date, o.charge, "
        "o.pass_flag, o.invalid_reason, o.reply_date, o.clinic_state, "
        "m.patient_age, m.patient_sex "
        "FROM clinic_orders o "
        "LEFT JOIN clinic_master m ON o.clinic_id = m.clinic_id "
        "WHERE o.username = :doctorName "
        "ORDER BY o.order_date DESC"
        );
    query.bindValue(":doctorName", doctorName);

    if (!query.exec()) {
        qDebug() << "查询咨询记录失败:" << query.lastError().text();
        return records;
    }

    while (query.next()) {
        ConsultRecord record;
        record.orderId = query.value(0).toString();
        record.clinicId = query.value(1).toString();
        record.serialNo = query.value(2).toString();
        record.patientName = query.value(3).toString();
        record.patientCard = query.value(4).toString();
        record.deptName = query.value(5).toString();
        record.username = query.value(6).toString();
        record.orderDate = query.value(7).toDateTime();
        record.charge = query.value(8).toDouble();
        // 去除空格
        record.passFlag = query.value(9).toString().trimmed();
        record.invalidReason = query.value(10).toString();
        record.replyDate = query.value(11).toInt();
        // 去除空格
        record.clinicState = query.value(12).toString().trimmed();
        record.patientAge = query.value(13).toString();
        record.patientSex = query.value(14).toString();

        qDebug() << "=== 读取咨询记录 ===";
        qDebug() << "患者:" << record.patientName;
        qDebug() << "clinicState:" << record.clinicState;
        qDebug() << "passFlag:" << record.passFlag;

        records.append(record);
    }

    return records;
}

ConsultRecord DbManager::getConsultRecordByOrderId(const QString &orderId)
{
    ConsultRecord record;

    QSqlQuery query;
    query.prepare(
        "SELECT o.order_id, o.clinic_id, o.serial_no, o.patient_name, "
        "o.patient_card, o.dept_name, o.username, o.order_date, o.charge, "
        "o.pass_flag, o.invalid_reason, o.reply_date, o.clinic_state, "
        "m.patient_age, m.patient_sex "
        "FROM clinic_orders o "
        "LEFT JOIN clinic_master m ON o.clinic_id = m.clinic_id "
        "WHERE o.order_id = :orderId"
        );
    query.bindValue(":orderId", orderId);

    if (query.exec() && query.next()) {
        record.orderId = query.value(0).toString();
        record.clinicId = query.value(1).toString();
        record.serialNo = query.value(2).toString();
        record.patientName = query.value(3).toString();
        record.patientCard = query.value(4).toString();
        record.deptName = query.value(5).toString();
        record.username = query.value(6).toString();
        record.orderDate = query.value(7).toDateTime();
        record.charge = query.value(8).toDouble();
        // 去除空格
        record.passFlag = query.value(9).toString().trimmed();
        record.invalidReason = query.value(10).toString();
        record.replyDate = query.value(11).toInt();
        // 去除空格
        record.clinicState = query.value(12).toString().trimmed();
        record.patientAge = query.value(13).toString();
        record.patientSex = query.value(14).toString();

        qDebug() << "=== 获取单条记录 ===";
        qDebug() << "患者:" << record.patientName;
        qDebug() << "clinicState:" << record.clinicState;
        qDebug() << "passFlag:" << record.passFlag;
    }

    return record;
}

bool DbManager::updateConsultState(const QString &orderId, const QString &state)
{
    QSqlQuery query;
    query.prepare("UPDATE clinic_orders SET clinic_state = :state WHERE order_id = :orderId");
    query.bindValue(":state", state);
    query.bindValue(":orderId", orderId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "更新咨询状态失败:" << m_lastError;
        return false;
    }

    qDebug() << "咨询状态已更新: orderId=" << orderId << ", state=" << state;
    return true;
}

bool DbManager::endConsult(const QString &orderId)
{
    QSqlQuery query;
    query.prepare("UPDATE clinic_orders SET clinic_state = '3' WHERE order_id = :orderId");
    query.bindValue(":orderId", orderId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "结束咨询失败:" << m_lastError;
        return false;
    }

    qDebug() << "咨询已结束: orderId=" << orderId;

    // 验证更新是否成功
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT clinic_state FROM clinic_orders WHERE order_id = :orderId");
    checkQuery.bindValue(":orderId", orderId);
    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "验证: 更新后的clinic_state=" << checkQuery.value(0).toString();
    }

    return true;
}

bool DbManager::markInvalidConsult(const QString &orderId, const QString &reason)
{
    QSqlQuery query;
    query.prepare("UPDATE clinic_orders SET pass_flag = '0', invalid_reason = :reason WHERE order_id = :orderId");
    query.bindValue(":reason", reason);
    query.bindValue(":orderId", orderId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "标记无效咨询失败:" << m_lastError;
        return false;
    }

    qDebug() << "咨询已标记为无效: orderId=" << orderId << ", reason=" << reason;

    // 验证更新是否成功
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT pass_flag, invalid_reason FROM clinic_orders WHERE order_id = :orderId");
    checkQuery.bindValue(":orderId", orderId);
    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "验证: 更新后的pass_flag=" << checkQuery.value(0).toString()
            << ", invalid_reason=" << checkQuery.value(1).toString();
    }

    return true;
}
bool DbManager::addToBlacklist(const QString &doctorId, const QString &patientId, const QString &reason)
{
    QString blackId = "BLK" + QString::number(QDateTime::currentMSecsSinceEpoch()).right(8);

    QSqlQuery query;
    query.prepare(
        "INSERT INTO blacklist (black_id, user_id, patient_id, reason, create_time, enable_flag) "
        "VALUES (:blackId, :doctorId, :patientId, :reason, GETDATE(), '0')"
        );
    query.bindValue(":blackId", blackId);
    query.bindValue(":doctorId", doctorId);
    query.bindValue(":patientId", patientId);
    query.bindValue(":reason", reason);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "添加到黑名单失败:" << m_lastError;
        return false;
    }

    qDebug() << "已添加到黑名单";
    return true;
}

bool DbManager::removeFromBlacklist(const QString &blackId)
{
    QSqlQuery query;
    // 物理删除黑名单记录
    query.prepare("DELETE FROM blacklist WHERE black_id = :blackId");
    query.bindValue(":blackId", blackId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "移出黑名单失败:" << m_lastError;
        return false;
    }

    qDebug() << "已移出黑名单: blackId=" << blackId;
    return true;
}

QList<QMap<QString, QString>> DbManager::getBlacklist(const QString &doctorName)
{
    QList<QMap<QString, QString>> blacklist;

    QString doctorId = getDoctorId(doctorName);
    if (doctorId.isEmpty()) return blacklist;

    QSqlQuery query;
    query.prepare(
        "SELECT b.black_id, b.reason, b.create_time, p.patient_name, p.patient_mobile "
        "FROM blacklist b "
        "JOIN patient p ON b.patient_id = p.patient_id "
        "WHERE b.user_id = :doctorId AND b.enable_flag = '0'"
        );
    query.bindValue(":doctorId", doctorId);

    if (!query.exec()) {
        qDebug() << "查询黑名单失败:" << query.lastError().text();
        return blacklist;
    }

    while (query.next()) {
        QMap<QString, QString> item;
        item["black_id"] = query.value("black_id").toString();
        item["patient_name"] = query.value("patient_name").toString();
        item["patient_mobile"] = query.value("patient_mobile").toString();
        item["reason"] = query.value("reason").toString();
        item["create_time"] = query.value("create_time").toDateTime().toString("yyyy-MM-dd hh:mm");
        blacklist.append(item);
    }

    return blacklist;
}

bool DbManager::isPatientBlocked(const QString &doctorName, const QString &patientName)
{
    QString doctorId = getDoctorId(doctorName);
    QString patientId = getPatientId(patientName);

    if (doctorId.isEmpty() || patientId.isEmpty()) return false;

    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM blacklist "
        "WHERE user_id = :doctorId AND patient_id = :patientId AND enable_flag = '0'"
        );
    query.bindValue(":doctorId", doctorId);
    query.bindValue(":patientId", patientId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}
