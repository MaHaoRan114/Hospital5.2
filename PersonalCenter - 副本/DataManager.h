#ifndef DATAMANAGER_H  // 必须有这两行，解决“多次包含”
#define DATAMANAGER_H

#include <QString>
#include <QList>

// 统一结构体定义
struct PatientRecord {
    QString name;
    QString gender;
    int age;
    QString lastDIAG;   // 诊断/医嘱记录 [注意：如果你代码其他地方叫 lastConsult，请改回来]
    bool isBlacklisted;
};

class DataManager {
public:
    static DataManager& instance() {
        static DataManager manager;
        return manager;
    }
    QList<PatientRecord> patients;

private:
    DataManager() {
        // 模拟医治过的患者名单
        patients << PatientRecord{ "张海龙", "男", 45, "感冒发烧", false };
        patients << PatientRecord{ "徐思源", "男", 30, "急性胃炎", false };
        patients << PatientRecord{ "杨雪儿", "女", 27, "轻微流产征兆", true };
        patients << PatientRecord{ "王大柱", "男", 50, "高血压", false };
    }
};

#endif // DATAMANAGER_H