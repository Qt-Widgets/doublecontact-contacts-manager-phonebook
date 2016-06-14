/* Double Contact
 *
 * Module: Global definitions
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QHash>
#include <QItemDelegate>
#include <QObject>
#include <QStringList>

// Common strings for translation unify
#define S_ERROR QObject::tr("Error")
#define S_WARNING QObject::tr("Warning")
#define S_CONFIRM QObject::tr("Confirmation")
#define S_NEW_LIST QObject::tr("New contact list")
// Spec.value for combined phone/mail types
#define S_MIXED_TYPE QObject::tr("mixed...")
// Col
#define S_FIRST_NAME QObject::tr("Firstname")
#define S_LAST_NAME QObject::tr("Lastname")
#define S_MIDDLE_NAME QObject::tr("Middlename")
#define S_FULL_NAME QObject::tr("Full name")
#define S_GENERIC_NAME QObject::tr("Generic name")
#define S_PHONE QObject::tr("Phone")
#define S_EMAIL QObject::tr("Email")

// Visible columns
enum ContactColumn {
  ccFirstName, ccLastName, ccMiddleName, // names
  ccFullName, ccGenericName, // formatted names
  ccPhone, // first or preferred phone
  ccEMail, // first or preferred email
  ccLast
};

// Visible columns headers
// (It was a simple string array, but its translation not worked)
class ContactColumnHeaders: public QStringList
{
public:
    ContactColumnHeaders();
    void fill();
};

extern ContactColumnHeaders contactColumnHeaders;

class ContactColumnList: public QList<ContactColumn>
{};

// Standart support types (phone, email, address) storage and checker
class StandardTypes: public QHash<QString, QString> {
    public:
    // untranslated keys stored in lowercase
    QString translate(const QString& key, bool* isStandard = 0) const;
    QString unTranslate(const QString& value) const;
    QStringList displayValues;
};

// Delegate for making table widgets read only
// Thanx to Axis - http://axis.bplaced.net/news/460
class ReadOnlyTableDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ReadOnlyTableDelegate(QObject* parent = 0);
    virtual QWidget* createEditor(
        QWidget*, const QStyleOptionViewItem&, const QModelIndex &) const;
};


#endif // GLOBALS_H
