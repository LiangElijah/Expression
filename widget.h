#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
extern "C" {
#include "dwarf_die.h"
#include "dwarf_method.h"
#include "dwarf_elf.h"
#include "dwarf_coff.h"
#include "dwarf_str.h"
#include "dwarf_addr.h"
}

#define CONFIG_FILE "config.json"

typedef enum {
    FILETYPE_COFF,
    FILETYPE_ELF
} enum_fileTyp_t;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_pushButton_clicked();

    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

private:
    Ui::Widget *ui;
    int columnOfExpression;
    int columnOfAddress;
    int columnOfType;
    Dwarf_Debug dbg = NULL;
    Dwarf_Error error = NULL;
    Dwarf_Obj_Access_Data *dw_accessData = NULL;
    st_dieNode_t *entry = NULL;
    enum_fileTyp_t filetyp = FILETYPE_COFF;

    int getColumnOfTitle(const QString &title);
    QString getAddressString(const st_addr_t &addrBuf);
    QString getTypeString(const st_addr_t &addrBuf);

    void writeJsonFile(const QString &filePath);
    void readJsonFile(const QString &filePath);
};
#endif // WIDGET_H
