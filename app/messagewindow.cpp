/* Double Contact
 *
 * Module: SMS Window
 *
 * Copyright 2020 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QShortcut>
#include <QStringList>

#include "messagewindow.h"
#include "ui_messagewindow.h"
#include "configmanager.h"
#include "corehelpers.h"
#include "helpers.h"
#include "logwindow.h"

MessageWindow::MessageWindow(ContactList* contacts) :
    QDialog(0),
    ui(new Ui::MessageWindow),
    model(0)
{
    ui->setupUi(this);
    // Status bar
    statusBar = new QStatusBar(this);
    lbCount = new QLabel(0);
    lbMMSCount = new QLabel(0);
    lbMode = new QLabel(0);
    lbDups = new QLabel(0);
    lbMultiParts = new QLabel(0);
    statusBar->addWidget(lbCount);
    statusBar->addWidget(lbMMSCount);
    statusBar->addWidget(lbMode);
    statusBar->addWidget(lbDups);
    statusBar->addWidget(lbMultiParts);
    layout()->addWidget(statusBar);
    // Table
    ui->tvMessages->horizontalHeader()->setStretchLastSection(true);
    // Check button access
    ui->cbPDU->setEnabled(!contacts->extra.pduSMS.isEmpty());
    ui->cbPDUArchive->setEnabled(!contacts->extra.pduSMSArchive.isEmpty());
    ui->cbVmessage->setEnabled(!contacts->extra.vmsgSMS.isEmpty());
    ui->cbVmessageArchive->setEnabled(!contacts->extra.vmsgSMSArchive.isEmpty());
    ui->cbBinary->setEnabled(!contacts->extra.binarySMS.isEmpty());
    // Load conf
    configManager.readMessageViewConfig(srcFlags);
    ui->cbPDU->setChecked(srcFlags.testFlag(usePDU) && ui->cbPDU->isEnabled());
    ui->cbPDUArchive->setChecked(srcFlags.testFlag(usePDUArchive) && ui->cbPDUArchive->isEnabled());
    ui->cbVmessage->setChecked(srcFlags.testFlag(useVMessage) && ui->cbVmessage->isEnabled());
    ui->cbVmessageArchive->setChecked(srcFlags.testFlag(useVMessageArchive) && ui->cbVmessageArchive->isEnabled());
    ui->cbBinary->setChecked(srcFlags.testFlag(useBinary) && ui->cbBinary->isEnabled());
    ui->cbMergeDups->setChecked(srcFlags.testFlag(mergeDuplicates));
    ui->cbMergeMultiparts->setChecked(srcFlags.testFlag(mergeMultiParts));
    checkMergeButton();
    // Model
    model = new MessageModel(this, contacts);
    proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(-1);
    proxy->setSortRole(SortStringRole);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); // Driver == driver
    ui->tvMessages->setModel(proxy);
    connect(ui->tvMessages->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(onSortIndicatorChanged(int,Qt::SortOrder)));
    connect(ui->tvMessages->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(selectionChanged()));
    connect(ui->tvMessages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged()));
    // Shortcuts
    QShortcut* shcSortToggle = new QShortcut(Qt::Key_F4, this);
    connect(shcSortToggle, SIGNAL(activated()), this, SLOT(toggleSort()));
    // Context menu
    ui->tvMessages->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->tvMessages->addAction(ui->actionCopy_text);
    ui->tvMessages->addAction(ui->actionSave_MMS_Files);
    ui->tvMessages->addAction(ui->actionShow_MMS_Files);
    ui->tvMessages->addAction(ui->actionProperties);
    menuMMSFiles = new QMenu(this);
    ui->actionShow_MMS_Files->setMenu(menuMMSFiles);
    // Calc!
    updateModel();
    ui->tvMessages->resizeColumnsToContents();
    readTableSortConfig(ui->tvMessages, false);
}

MessageWindow::~MessageWindow()
{
    delete ui;
}

void MessageWindow::selectionChanged()
{
    QModelIndex sel = selectedRecord(false);
    // Context menu permissions
    if (!sel.isValid()) {
        ui->actionProperties->setEnabled(false);
        ui->actionSave_MMS_Files->setEnabled(false);
        ui->actionShow_MMS_Files->setEnabled(false);
    }
    else {
        ui->actionProperties->setEnabled(true);
        const DecodedMessage& msg = model->item(sel.row());
        ui->actionSave_MMS_Files->setEnabled(msg.isMMS);
        ui->actionShow_MMS_Files->setEnabled(msg.isMMS);
        if (msg.isMMS) {
            menuMMSFiles->clear();
            int index = 0;
            foreach(const InnerFile& f, msg.mmsFiles) {
                QAction* a = new QAction(f.name, this);
                a->setData(index);
                connect(a, SIGNAL(triggered(bool)), this, SLOT(onShowMMSFile(bool)));
                menuMMSFiles->addAction(a);
                index++;
            }
        }
    }
}

void MessageWindow::checkButtons()
{
    configManager.writeMessageViewConfig(srcFlags);
    if (model)
        updateModel();
    ui->tvMessages->resizeRowsToContents();
}

void MessageWindow::checkMergeButton()
{
    short srcCount = 0;
    if (ui->cbPDU->isChecked()) srcCount++;
    if (ui->cbPDUArchive->isChecked()) srcCount++;
    if (ui->cbVmessage->isChecked()) srcCount++;
    if (ui->cbVmessageArchive->isChecked()) srcCount++;
    if (ui->cbBinary->isChecked()) srcCount++;
    ui->cbMergeDups->setEnabled(srcCount>0);// Initially >1, but dups can be even in one source
    ui->cbMergeMultiparts->setEnabled(srcCount>0);
}

QModelIndex MessageWindow::selectedRecord(bool errorIfNoSelected)
{
    // check Selection
    QModelIndexList proxySelection = ui->tvMessages->selectionModel()->selectedRows();
    if (proxySelection.count()==0) {
        if (errorIfNoSelected)
            QMessageBox::critical(0, S_ERROR, S_REC_NOT_SEL);
        return QModelIndex();
    }
    if (proxySelection.count()>1) {
        QMessageBox::critical(0, S_ERROR, S_ONLY_ONE_REC);
        return QModelIndex();
    }
    // Single selection
    return proxy->mapToSource(proxySelection.first());
}

void MessageWindow::on_cbPDU_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, usePDU, ui->cbPDU->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbPDUArchive_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, usePDUArchive,  ui->cbPDUArchive->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbBinary_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, useBinary,  ui->cbBinary->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbVmessage_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, useVMessage,  ui->cbVmessage->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbVmessageArchive_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, useVMessageArchive,  ui->cbVmessageArchive->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbMergeDups_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, mergeDuplicates,  ui->cbMergeDups->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::on_cbMergeMultiparts_stateChanged(int)
{
    checkMergeButton();
    setQFlag(srcFlags, mergeMultiParts,  ui->cbMergeMultiparts->isChecked());
    ui->btnApply->setEnabled(true);
}

void MessageWindow::updateModel()
{
    QStringList errors;
    model->update(srcFlags, errors);
    if (!errors.isEmpty()) {
        LogWindow* w = new LogWindow(0);
        w->setData(tr(" with messages"), model->rowCount(), errors);
        w->exec();
        delete w;
    }
    updateStatus();
}

void MessageWindow::updateStatus()
{
    lbCount->setText(tr("Records: %1").arg(model->rowCount()));
    lbMMSCount->setText(tr("Including MMS: %1").arg(model->mmsCount()));
    lbDups->setText(tr("Merged dups: %1").arg(model->mergeDupCount()));
    lbMultiParts->setText(tr("Merged multiparted: %1").arg(model->mergeMultiPartCount()));
    lbMode->setText(SS_MODE +
                    (ui->tvMessages->isSortingEnabled() ? SS_SORT_ON : SS_SORT_OFF));
}

void MessageWindow::toggleSort()
{
    bool needSort = !ui->tvMessages->isSortingEnabled();
    readTableSortConfig(ui->tvMessages, true, needSort);
    writeTableSortConfig(ui->tvMessages);
    updateStatus();
}

void MessageWindow::onSortIndicatorChanged(int, Qt::SortOrder)
{
    QHeaderView* header = dynamic_cast<QHeaderView*>(sender());
    if (header)
        writeTableSortConfig(header);
}

void MessageWindow::on_actionCopy_text_triggered()
{
    QModelIndex ind = ui->tvMessages->selectionModel()->currentIndex();
    ind = proxy->mapToSource(ind);
    QString text = model->data(ind, Qt::DisplayRole).toString();
    qApp->clipboard()->setText(text);
}

void MessageWindow::on_actionProperties_triggered()
{
    QModelIndex sel = selectedRecord();
    if (!sel.isValid())
        return;
    const DecodedMessage& msg = model->item(sel.row());
    // Message sources
    QStringList srcs;
    if (msg.sources.testFlag(useVMessage))
        srcs << ui->cbVmessage->text();
    if (msg.sources.testFlag(useVMessageArchive))
        srcs << ui->cbVmessageArchive->text();
    if (msg.sources.testFlag(usePDU))
        srcs << ui->cbPDU->text();
    if (msg.sources.testFlag(usePDUArchive))
        srcs << ui->cbPDUArchive->text();
    if (msg.sources.testFlag(useBinary))
        srcs << ui->cbBinary->text();
    // Show info
    QString info = tr("id: %1\nSources: %2")
        .arg(msg.id).arg(srcs.join(", "));
    if (msg.isMMS)
        info += tr("\n\nMMS properties:\n%1").arg(msg.text);
    QMessageBox::information(0, S_INFORM, info);
}

void MessageWindow::on_leFilter_textChanged(const QString &newText)
{
    if (newText.isEmpty())
        proxy->setFilterWildcard("");
    else
        proxy->setFilterWildcard(newText);
}

void MessageWindow::on_btnSaveAs_clicked()
{
    QFileInfo fi(configManager.lastContactFile());
    QString path = fi.path() + QDir::separator() + fi.baseName() + "-messages.csv";
    QString selectedFilter;
    path = QFileDialog::getSaveFileName(0, tr("Save messages file"),
        path, "*.csv",
        &selectedFilter);
    if (!path.isEmpty()) {
        model->saveToCSV(path);
        if (model->mmsCount()>0) {
            int btn = QMessageBox::question(0, S_CONFIRM,
                 tr("Can you also save media files from MMS (%1 messages)?").arg(model->mmsCount()),
                 QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
            if (btn==QMessageBox::Yes) {
                QString path = QFileDialog::getExistingDirectory(this,
                    S_SELECT_MMS_DIR_TITLE, configManager.lastContactFile(),
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                if (!path.isEmpty()) {
                    QString fatalError;
                    if (!model->saveAllMMSFiles(path, fatalError))
                        QMessageBox::critical(0, S_ERROR, fatalError);
                }
            }
        }
    }
}

void MessageWindow::showEvent(QShowEvent *)
{
    updateTableConfig(ui->tvMessages);
    ui->tvMessages->selectRow(0);
}

void MessageWindow::on_actionSave_MMS_Files_triggered()
{
    QModelIndex sel = selectedRecord();
    if (!sel.isValid())
        return;
    const DecodedMessage& msg = model->item(sel.row());
    QString path = QFileDialog::getExistingDirectory(this,
        S_SELECT_MMS_DIR_TITLE, configManager.lastContactFile(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!path.isEmpty()) {
        QString fatalError;
        if (!msg.mmsFiles.saveAll(path, fatalError))
            QMessageBox::critical(0, S_ERROR, fatalError);
    }
}

void MessageWindow::onShowMMSFile(bool)
{
    QModelIndex sel = selectedRecord(false);
    if (!sel.isValid())
        return;
    const DecodedMessage& msg = model->item(sel.row());
    QAction* a = dynamic_cast<QAction*>(sender());
    if (a) {
        int index = a->data().toInt();
        if (index>=0 && index<msg.mmsFiles.count())
            showInnerFile(msg.mmsFiles[index]);
    }
}

void MessageWindow::on_btnApply_clicked()
{
    checkButtons();
    ui->btnApply->setEnabled(false);
}

void MessageWindow::on_btnHardSort_clicked()
{
    model->hardSort();
    // Warning if sort view switched on
    if (ui->tvMessages->isSortingEnabled())
        if (QMessageBox::question(0, S_CONFIRM, S_SORTMASK_CONFIRM,
                 QMessageBox::Yes, QMessageBox::No)==QMessageBox::Yes)
        {
            readTableSortConfig(ui->tvMessages, true, false);
            writeTableSortConfig(ui->tvMessages);
            updateStatus();
        }
    model->updateView();
}

