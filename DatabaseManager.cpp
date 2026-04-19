#include "DatabaseManager.h"
#include <QDebug>
#include <QUuid>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance)
        m_instance = new DatabaseManager();
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

bool DatabaseManager::connectToDatabase(const QString &host, const QString &dbName,
                                        const QString &user, const QString &password)
{
    m_db = QSqlDatabase::addDatabase("QODBC");
    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (!m_db.open()) {
        qWarning() << "数据库连接失败:" << m_db.lastError().text();
        return false;
    }
    return true;
}

// ===================== 5.2.5 药品医嘱 =====================
bool DatabaseManager::submitDrugOrder(const OrderModel &order)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO CLINIC_ORDERS "
        "(ORDER_ID, CONSULT_ID, DOCTOR_ID, PATIENT_ID, ORDER_TYPE, ORDER_STATUS, "
        " DRUG_NAME, SPEC, UNITS, AMOUNT, DOSAGE, DOSAGE_UNITS, ADMINISTRATION, "
        " FREQUENCY, PROVIDED_INDICATOR, COSTS, CHARGES, CHARGE_INDICATOR, "
        " DISPENSARY, REPETITION, ORDER_NO, SUB_ORDER_NO, FREQ_DETAIL, GETDRUG_FLAG, CREATE_TIME) "
        "VALUES "
        "(:orderId, :consultId, :doctorId, :patientId, 1, 0, "
        " :drugName, :spec, :units, :amount, :dosage, :dosageUnits, :administration, "
        " :frequency, :provided, :costs, :charges, :charged, "
        " :dispensary, :repetition, :orderNo, :subOrderNo, :freqDetail, :getDrugFlag, NOW())"
    );

    query.bindValue(":orderId",       QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":consultId",     order.consultId);
    query.bindValue(":doctorId",      order.doctorId);
    query.bindValue(":patientId",     order.patientId);
    query.bindValue(":drugName",      order.drugName);
    query.bindValue(":spec",          order.spec);
    query.bindValue(":units",         order.units);
    query.bindValue(":amount",        order.amount);
    query.bindValue(":dosage",        order.dosage);
    query.bindValue(":dosageUnits",   order.dosageUnits);
    query.bindValue(":administration",order.administration);
    query.bindValue(":frequency",     order.frequency);
    query.bindValue(":provided",      order.providedIndicator ? 1 : 0);
    query.bindValue(":costs",         order.costs);
    query.bindValue(":charges",       order.charges);
    query.bindValue(":charged",       order.chargeIndicator ? 1 : 0);
    query.bindValue(":dispensary",    order.dispensary);
    query.bindValue(":repetition",    order.repetition);
    query.bindValue(":orderNo",       order.orderNo);
    query.bindValue(":subOrderNo",    order.subOrderNo);
    query.bindValue(":freqDetail",    order.freqDetail);
    query.bindValue(":getDrugFlag",   order.getDrugFlag);

    if (!query.exec()) {
        qWarning() << "提交药品医嘱失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// ===================== 5.2.6 检查医嘱 =====================
bool DatabaseManager::submitExamOrder(const OrderModel &order)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO CLINIC_ORDERS "
        "(ORDER_ID, CONSULT_ID, DOCTOR_ID, PATIENT_ID, ORDER_TYPE, ORDER_STATUS, "
        " EXAM_NAME, EXAM_CODE, EXAM_DEPT, EXAM_NOTE, CREATE_TIME) "
        "VALUES "
        "(:orderId, :consultId, :doctorId, :patientId, 2, 0, "
        " :examName, :examCode, :examDept, :examNote, NOW())"
    );

    query.bindValue(":orderId",   QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.bindValue(":consultId", order.consultId);
    query.bindValue(":doctorId",  order.doctorId);
    query.bindValue(":patientId", order.patientId);
    query.bindValue(":examName",  order.examName);
    query.bindValue(":examCode",  order.examCode);
    query.bindValue(":examDept",  order.examDept);
    query.bindValue(":examNote",  order.examNote);

    if (!query.exec()) {
        qWarning() << "提交检查医嘱失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// ===================== 5.2.7 取消医嘱 =====================
bool DatabaseManager::cancelOrder(const QString &orderId, const QString &reason)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE CLINIC_ORDERS SET ORDER_STATUS = 1, CANCEL_TIME = NOW(), CANCEL_REASON = :reason "
        "WHERE ORDER_ID = :orderId AND ORDER_STATUS = 0"
    );
    query.bindValue(":reason",  reason);
    query.bindValue(":orderId", orderId);

    if (!query.exec() || query.numRowsAffected() == 0) {
        qWarning() << "取消医嘱失败:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<OrderModel> DatabaseManager::getActiveOrders(const QString &consultId)
{
    QList<OrderModel> list;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT ORDER_ID, ORDER_TYPE, DRUG_NAME, EXAM_NAME, CREATE_TIME "
        "FROM CLINIC_ORDERS WHERE CONSULT_ID = :cid AND ORDER_STATUS = 0"
    );
    query.bindValue(":cid", consultId);
    if (query.exec()) {
        while (query.next()) {
            OrderModel o;
            o.orderId     = query.value("ORDER_ID").toString();
            o.orderType   = query.value("ORDER_TYPE").toInt();
            o.drugName    = query.value("DRUG_NAME").toString();
            o.examName    = query.value("EXAM_NAME").toString();
            o.createTime  = query.value("CREATE_TIME").toDateTime();
            list.append(o);
        }
    }
    return list;
}

QList<OrderModel> DatabaseManager::getDrugOrders(const QString &consultId)
{
    QList<OrderModel> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM CLINIC_ORDERS WHERE CONSULT_ID=:cid AND ORDER_TYPE=1");
    query.bindValue(":cid", consultId);
    if (query.exec()) {
        while (query.next()) {
            OrderModel o;
            o.orderId   = query.value("ORDER_ID").toString();
            o.drugName  = query.value("DRUG_NAME").toString();
            o.dosage    = query.value("DOSAGE").toDouble();
            o.frequency = query.value("FREQUENCY").toString();
            list.append(o);
        }
    }
    return list;
}

QList<OrderModel> DatabaseManager::getExamOrders(const QString &consultId)
{
    QList<OrderModel> list;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM CLINIC_ORDERS WHERE CONSULT_ID=:cid AND ORDER_TYPE=2");
    query.bindValue(":cid", consultId);
    if (query.exec()) {
        while (query.next()) {
            OrderModel o;
            o.orderId  = query.value("ORDER_ID").toString();
            o.examName = query.value("EXAM_NAME").toString();
            o.examDept = query.value("EXAM_DEPT").toString();
            list.append(o);
        }
    }
    return list;
}
