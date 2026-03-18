#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>

class PersonalCenter : public QWidget {
    Q_OBJECT
public:
    explicit PersonalCenter(QWidget* parent = nullptr);

private:
    QTabWidget* mainTabs;

    // 所有的页面生成函数都得在这里声明
    QWidget* createSettingPage();      // 接诊设置 [cite: 708-710]
    QWidget* createIncomePage();       // 我的收入 [cite: 1525]
    QWidget* createOrderPage();        // 医嘱记录 [cite: 1581]
    QWidget* createBlacklistPage();    // 黑名单管理 [cite: 1605]
    QWidget* createRefundPage();       // 新增：退款审核 

    // 在 class PersonalCenter : public QWidget { ... } 内部修改
private:
    QTableWidget* blacklistTable; // 提取出来作为成员
    QLineEdit* blacklistSearchEdit;

    // 声明逻辑处理函数

    void refreshBlacklistTable(bool showOnlyBlacklisted = false);

private slots: // 按钮点击事件处理
    void onSearchClicked();
    void onAddBlacklistClicked();
    void onRemoveBlacklistClicked();
};