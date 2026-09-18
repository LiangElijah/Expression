#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->treeWidget->setColumnWidth(0, 400);
    ui->treeWidget->setColumnWidth(1, 150);
    ui->treeWidget->setColumnWidth(2, 150);
    ui->treeWidget->clear();
    ui->treeWidget->header()->setSectionsMovable(true);
    ui->treeWidget->header()->setFirstSectionMovable(true);

    columnOfExpression = getColumnOfTitle("Expression");
    columnOfAddress = getColumnOfTitle("Address");
    columnOfType = getColumnOfTitle("Type");

    auto *treeWidgetItem = new QTreeWidgetItem(ui->treeWidget);
    treeWidgetItem->setFlags(treeWidgetItem->flags() | Qt::ItemIsEditable);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "",
                                                    tr("Program Files (*.out *.hex *.txt);;All Files (*)"));
    if (!fileName.isEmpty()) {
        qDebug("Open File: %s", qUtf8Printable(fileName));

        ui->comboBox->insertItem(0, fileName);
        ui->comboBox->setCurrentIndex(0);
    }
}

void Widget::on_comboBox_currentTextChanged(const QString &arg1)
{
    qDebug("ComboBox File: %s", arg1.toStdString().c_str());

    int res = dwarf_elf_init(arg1.toStdString().c_str(), &dbg, &error);
    if(res != DW_DLV_OK)
    {
        res = dwarf_coff_init(arg1.toStdString().c_str(), &dw_accessData, &dbg, &error);
        if(res != DW_DLV_OK)
        {
            qDebug("file format err\r\n");
        }
    }

    if(res == DW_DLV_OK) {
        res = dwarf_die_init(dbg, &entry, &error);
        if((res != DW_DLV_ERROR) && (entry != NULL)) {
            for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
                QTreeWidgetItem *treeWidgetItem = ui->treeWidget->topLevelItem(i);
                QString variant = treeWidgetItem->text(columnOfExpression);
                if(variant != "")
                {
                    st_str_t str = {0};
                    st_addr_t addr = {0};

                    res = dwarf_str_init(variant.toStdString().c_str(), &str);
                    if(res == 0) {
                        res = dwarf_addr_cal(entry, &str, &addr);
                        if(res == 0) {
                            treeWidgetItem->setText(columnOfAddress, getAddressString(addr));
                            treeWidgetItem->setText(columnOfType, getTypeString(addr));
                        } else {
                            qDebug("addr err\r\n");
                        }
                    } else {
                        qDebug("str err\r\n");
                    }
                }
            }
        } else {
            qDebug("entry err %d\r\n", res);
        }
    }
}

void Widget::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    if(columnOfExpression == column) {
        int cal = 0;

        if((ui->treeWidget->indexOfTopLevelItem(item) + 1) == ui->treeWidget->topLevelItemCount()) {
            if(item->text(column) != "")
            {
                auto *treeWidgetItem = new QTreeWidgetItem(ui->treeWidget);
                treeWidgetItem->setFlags(treeWidgetItem->flags() | Qt::ItemIsEditable);
                cal = 1;
            }
        } else {
            if(item->text(column) == "")
                delete item;
            else cal = 1;
        }

        if(cal == 1) {
            st_str_t str = {0};
            st_addr_t addr = {0};

            int res = dwarf_str_init(item->text(columnOfExpression).toStdString().c_str(), &str);
            if(res == 0) {
                res = dwarf_addr_cal(entry, &str, &addr);
                if(res == 0) {
                    item->setText(columnOfAddress, getAddressString(addr));
                    item->setText(columnOfType, getTypeString(addr));
                } else {
                    qDebug("addr err\r\n");
                }
            } else {
                qDebug("str err\r\n");
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
