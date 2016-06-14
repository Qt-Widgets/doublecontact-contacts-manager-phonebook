/* Double Contact
 *
 * Module: Contact comparison dialog
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QFrame>

#include "comparedialog.h"
#include "ui_comparedialog.h"

#include <QDebug>

CompareDialog::CompareDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompareDialog),
    pFullName(0), pNames(0)
{
    ui->setupUi(this);
}

CompareDialog::~CompareDialog()
{
    delete ui;
}

void CompareDialog::setHeaders(const QString &left, const QString &right)
{
    ui->lbLeft->setText(left);
    ui->lbRight->setText(right);
}

template<class P, class T>
void CompareDialog::checkPair(const QString& title, P** pair, T& leftData, T& rightData)
{
    *pair = 0;
    if (!leftData.isEmpty() || !rightData.isEmpty())
        *pair = new P(title, ui->layPairs, leftData, rightData);
}

void CompareDialog::setData(const ContactItem &left, const ContactItem &right)
{
    ui->layPairs->setAlignment(Qt::AlignTop);

    checkPair(tr("Full name"), &pFullName, left.fullName, right.fullName);
    checkPair(tr("Names"), &pNames, left.names, right.names);
    checkPair(tr("Phones"), &pPhones, left.phones, right.phones);
    checkPair(tr("Emails"), &pEmails, left.emails, right.emails);
    checkPair(tr("Birthday"), &pBDay, left.birthday, right.birthday);
    checkPair(tr("Anniversaries"), &pAnn, left.anniversaries, right.anniversaries);
    checkPair(tr("Description"), &pDesc, left.description, right.description);
    // TODO photo*
    checkPair(tr("Organization"), &pOrg, left.organization, right.organization);
    checkPair(tr("Job title"), &pTitle, left.title, right.title);
    // TODO address
}

void CompareDialog::getData(ContactItem &left, ContactItem &right)
{
    if (pFullName)
        pFullName->getData(left.fullName, right.fullName);
    if (pNames)
        pNames->getData(left.names, right.names);
    if (pPhones)
        pPhones->getData(left.phones, right.phones);
    if (pEmails)
        pEmails->getData(left.emails, right.emails);
    if (pBDay)
        pBDay->getData(left.birthday, right.birthday);
    if (pAnn)
        pAnn->getData(left.anniversaries, right.anniversaries);
    if (pDesc)
        pDesc->getData(left.description, right.description);
    // TODO photo*
    if (pOrg)
        pOrg->getData(left.organization, right.organization);
    if (pTitle)
        pTitle->getData(left.title, right.title);
    // TODO address
}
