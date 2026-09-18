#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->treeWidget->setColumnWidth(0, 400);
    ui->treeWidget->setColumnWidth(1, 180);
    ui->treeWidget->setColumnWidth(2, 120);
    ui->treeWidget->clear();
    ui->treeWidget->header()->setSectionsMovable(true);
    ui->treeWidget->header()->setFirstSectionMovable(true);

    columnOfExpression = getColumnOfTitle("Expression");
    columnOfAddress = getColumnOfTitle("Address");
    columnOfType = getColumnOfTitle("Type");

    ui->comboBox->addItem("");

    readJsonFile(CONFIG_FILE);

    auto *treeWidgetItem = new QTreeWidgetItem(ui->treeWidget);
    treeWidgetItem->setFlags(treeWidgetItem->flags() | Qt::ItemIsEditable);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::closeEvent(QCloseEvent *event)
{
    writeJsonFile(CONFIG_FILE);
}

void Widget::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "",
                                                    tr("Program Files (*.out *.hex *.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        qDebug("[%s:%d] Open File: %s", __func__, __LINE__,
               fileName.toStdString().c_str());

        ui->comboBox->insertItem(1, fileName);
        ui->comboBox->setCurrentIndex(1);
    }
}

void Widget::on_comboBox_currentTextChanged(const QString &arg1)
{
    qDebug("[%s:%d] ComboBox Open File: %s", __func__, __LINE__,
           arg1.toStdString().c_str());
    if(arg1 == "") return;

    if(dbg != NULL) {
        if(filetyp == FILETYPE_COFF) {
            dwarf_coff_deinit(dbg, dw_accessData);
            dw_accessData = NULL;
        } else {
            dwarf_elf_deinit(dbg);
        }
        dbg = NULL;
    }

    if(entry != NULL) {
        dwarf_die_deinit(entry);
        entry = NULL;
    }

    int res = dwarf_elf_init(arg1.toStdString().c_str(), &dbg, &error);
    if(res != DW_DLV_OK) {
        res = dwarf_coff_init(arg1.toStdString().c_str(), &dw_accessData, &dbg, &error);
        filetyp = FILETYPE_COFF;
    } else {
        filetyp = FILETYPE_ELF;
    }

    if(res == DW_DLV_OK) {
        res = dwarf_die_init(dbg, &entry, &error);
        // res不应该返回DW_DLV_NO_ENTRY，后续修改接口
        if(res == DW_DLV_OK) {
            for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
                QTreeWidgetItem *treeWidgetItem = ui->treeWidget->topLevelItem(i);
                QString variant = treeWidgetItem->text(columnOfExpression);
                if(variant != "") {
                    st_str_t str = {0};
                    st_addr_t addr = {0};

                    res = dwarf_str_init(variant.toStdString().c_str(), &str);
                    if(res == 0) {
                        res = dwarf_addr_cal(entry, &str, &addr);
                        if(res == 0) {
                            treeWidgetItem->setText(columnOfAddress, getAddressString(addr));
                            treeWidgetItem->setText(columnOfType, getTypeString(addr));
                        } else {
                            qDebug("[%s:%d] addr err", __func__, __LINE__);
                            treeWidgetItem->setText(columnOfAddress, "");
                            treeWidgetItem->setText(columnOfType, "unknown");
                            // 具体原因提示
                        }
                    } else {
                        qDebug("[%s:%d] str err", __func__, __LINE__);
                        treeWidgetItem->setText(columnOfAddress, "");
                        treeWidgetItem->setText(columnOfType, "unknown");
                        // 具体原因提示
                    }
                } else if(i < (ui->treeWidget->topLevelItemCount() - 1)) {
                    delete treeWidgetItem; i--; // 重新查询该索引
                }
            }
        } else {
            qDebug("[%s:%d] entry err %d", __func__, __LINE__, res);
            for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
                QTreeWidgetItem *treeWidgetItem = ui->treeWidget->topLevelItem(i);
                treeWidgetItem->setText(columnOfAddress, "");
                treeWidgetItem->setText(columnOfType, "unknown");
            }
            QMessageBox::critical(this,
                                  "Load Program Failed",
                                  "Could not find target entry of file !!!",
                                  QMessageBox::Ok);
            if(dbg != NULL) {
                if(filetyp == FILETYPE_COFF) {
                    dwarf_coff_deinit(dbg, dw_accessData);
                    dw_accessData = NULL;
                } else {
                    dwarf_elf_deinit(dbg);
                }
            }
            dbg = NULL;
        }
    } else {
        qDebug("[%s:%d] file format err", __func__, __LINE__);
        for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
            QTreeWidgetItem *treeWidgetItem = ui->treeWidget->topLevelItem(i);
            treeWidgetItem->setText(columnOfAddress, "");
            treeWidgetItem->setText(columnOfType, "unknown");
        }
        QMessageBox::critical(this,
                              "Load Program Failed",
                              "Could not determine target type of file !!!",
                              QMessageBox::Ok);
    }
}

void Widget::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    if(columnOfExpression == column) {
        int cal = 0;

        qDebug("[%s:%d] Item Change: %s", __func__, __LINE__,
               item->text(columnOfExpression).toStdString().c_str());

        if((ui->treeWidget->indexOfTopLevelItem(item) + 1) == ui->treeWidget->topLevelItemCount()) {
            if(item->text(columnOfExpression) != "")
            {
                auto *treeWidgetItem = new QTreeWidgetItem(ui->treeWidget);
                treeWidgetItem->setFlags(treeWidgetItem->flags() | Qt::ItemIsEditable);
                cal = 1;
            }
        } else {
            if(item->text(columnOfExpression) == "") {
                delete item;
            } else {
                cal = 1;
            }
        }

        if((entry != NULL) && (cal == 1)) {
            st_str_t str = {0};
            st_addr_t addr = {0};

            int res = dwarf_str_init(item->text(columnOfExpression).toStdString().c_str(), &str);
            if(res == 0) {
                res = dwarf_addr_cal(entry, &str, &addr);
                if(res == 0) {
                    item->setText(columnOfAddress, getAddressString(addr));
                    item->setText(columnOfType, getTypeString(addr));
                } else {
                    qDebug("[%s:%d] addr err", __func__, __LINE__);
                    item->setText(columnOfAddress, "");
                    item->setText(columnOfType, "unknown");
                    // 具体原因提示
                }
            } else {
                qDebug("[%s:%d] str err", __func__, __LINE__);
                item->setText(columnOfAddress, "");
                item->setText(columnOfType, "unknown");
                // 具体原因提示
            }
        }
    }
}

int Widget::getColumnOfTitle(const QString &title)
{
    QTreeWidgetItem *header = ui->treeWidget->headerItem();
    for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
        if (header->text(i) == title)
            return i;
    }
    return -1;
}

QString Widget::getAddressString(const st_addr_t &addrBuf)
{
    QString str;

    if(addrBuf.bit_size > 0)
    {
        if(addrBuf.bit_size == 1)
        {
            str = QString::asprintf("0x%08X@Data bit %llu", addrBuf.addr, 15-addrBuf.bit_offset);
        }
        else
        {
            str = QString::asprintf("0x%08X@Data bit %llu-%llu", addrBuf.addr,
                                    15 - addrBuf.bit_offset - addrBuf.bit_size + 1,
                                    15-addrBuf.bit_offset);
        }
    }
    else
    {
        str = QString::asprintf("0x%08X@Data", addrBuf.addr);
    }

    return str;
}

QString Widget::getTypeString(const st_addr_t &addrBuf)
{
    QString str;

    if(addrBuf.num < addrBuf.numDef)
    {
        str = QString("%1").arg(addrBuf.typName);
        for(int d = addrBuf.num; d < addrBuf.numDef; d++)
        {
            str += QString("[%1]").arg(addrBuf.deep[d]);
        }
    }
    else if(addrBuf.bit_size > 0)
    {
        str = QString("%1:%2").arg(addrBuf.typName).arg(addrBuf.bit_size);
    }
    else
    {
        str = QString("%1").arg(addrBuf.typName);
    }

    return str;
}

void Widget::writeJsonFile(const QString &filePath)
{
    // 1、创建根对象
    QJsonObject jsonObj;

    // 2、创建文件子数组
    QJsonArray fileDirArray;
    for (int i = 0; i < ui->comboBox->count(); i++) {
        QString fileDir = ui->comboBox->itemText(i);
        if(fileDir != "") {
            fileDirArray.append(fileDir);
        }
    }
    jsonObj.insert("fileDirArray", fileDirArray);

    // 3、创建表达式子数组
    QJsonArray expressionArray;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *treeWidgetItem = ui->treeWidget->topLevelItem(i);
        QString variant = treeWidgetItem->text(columnOfExpression);
        if(variant != "") {
            expressionArray.append(variant);
        }
    }
    jsonObj.insert("expressionArray", expressionArray);

    // 4、封装为 QJsonDocument
    QJsonDocument doc(jsonObj);

    // 5、打开文件并写入（使用 UTF-8 编码，防止中文乱码）
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson()); // 默认缩进格式
        file.close();
    } else {
        qDebug("[%s:%d] file write err", __func__, __LINE__);
    }
}

void Widget::readJsonFile(const QString &filePath)
{
    // 1、打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug("[%s:%d] file read err", __func__, __LINE__);
        return;
    }

    // 2、读取文件全部内容
    QByteArray jsonData = file.readAll();
    file.close();

    // 3、解析 JSON 文档
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug("[%s:%d] parse err:%s", __func__, __LINE__,
               parseError.errorString().toStdString().c_str());
        return;
    }

    // 4、检查是否为对象并读取数据
    if (doc.isObject()) {
        QJsonObject jsonObj = doc.object();

        // 读取数组
        if (jsonObj.contains("fileDirArray") && jsonObj["fileDirArray"].isArray()) {
            QJsonArray fileDirArray = jsonObj["fileDirArray"].toArray();
            for (const QJsonValue &value : fileDirArray) {
                ui->comboBox->addItem(value.toString());
            }
        }

        if (jsonObj.contains("expressionArray") && jsonObj["expressionArray"].isArray()) {
            QJsonArray expressionArray = jsonObj["expressionArray"].toArray();
            for (const QJsonValue &value : expressionArray) {
                auto *treeWidgetItem = new QTreeWidgetItem();
                treeWidgetItem->setFlags(treeWidgetItem->flags() | Qt::ItemIsEditable);
                treeWidgetItem->setText(columnOfExpression, value.toString());
                ui->treeWidget->addTopLevelItem(treeWidgetItem);
            }
        }
    }
}
