/* Double Contact
 *
 * Module: Comma-separated values (CSV) file export/import
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */
#include <QStringList>

#include <QTextCodec>
#include "csvfile.h"
#include "corehelpers.h"
#include "../profiles/explaybm50profile.h"
#include "../profiles/explaytv240profile.h"
#include "../profiles/genericcsvprofile.h"
#include "../profiles/osmoprofile.h"

CSVFile::CSVFile()
    :FileFormat(), currentProfile(0),
      _encoding(""), _separator(",")
{
    profiles << new ExplayBM50Profile;
    profiles << new ExplayTV240Profile;
    profiles << new GenericCSVProfile;
    profiles << new OsmoProfile;
}

CSVFile::~CSVFile()
{
    foreach (CSVProfileBase* _profile, profiles)
        delete _profile;
}

bool CSVFile::detect(const QString &url)
{
    QFile f(url);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QTextStream stream(&f);
    QString s1 = stream.readLine();
    bool res = s1.contains(","); // TODO bad method. M.b. make CSVProfile::detect and call for all profiles
    f.close();
    return res;
}

QStringList CSVFile::supportedExtensions()
{
    return (QStringList() << "csv" << "CSV");
}

QStringList CSVFile::supportedFilters()
{
    return (QStringList() << "CSV (*.csv *.CSV)");
}

QStringList CSVFile::availableProfiles()
{
    QStringList ap;
    foreach (CSVProfileBase* _profile, profiles)
        ap << _profile->name();
    return ap;
}

bool CSVFile::setProfile(const QString &name)
{
    currentProfile = 0;
    foreach (CSVProfileBase* _profile, profiles)
        if (_profile->name()==name){
            currentProfile = _profile;
            return true;
        }
    return false;
}

void CSVFile::setEncoding(const QString &encoding)
{
    _encoding = encoding;
}

void CSVFile::setSeparator(const QString &separator)
{
    _separator = separator;
}

QString CSVFile::profile()
{
    if (currentProfile)
        return currentProfile->name();
    else
        return "";
}

bool CSVFile::importRecords(const QString &url, ContactList &list, bool append)
{
    if (!currentProfile)
        return false;
    if (!openFile(url, QIODevice::ReadOnly))
        return false;
    _errors.clear();
    QList<QStringList> rows;
    QTextStream stream(&file);
    if (_encoding.isEmpty())
        _encoding = currentProfile->charSet();
    stream.setCodec(_encoding.toLatin1().data());
    do {
        QString line = stream.readLine();
        bool inQuotes = false;
        QString val = "";
        QStringList row;
        foreach(const QChar& c, line.trimmed()) {
            switch (c.toLatin1()) {
            case '\"':
                inQuotes = !inQuotes;
                break;
            case ';':
            case ',':
                if (!inQuotes) {
                    row << val;
                    val.clear();
                    break;
                } // fall through
            case 0:
            default:
                val += c;
                break;
            }
        }
        row << val;
        rows << row;
    } while (!stream.atEnd());
    closeFile();
    int firstLine = currentProfile->hasHeader() ? 1 : 0;
    if (currentProfile->hasHeader())
        currentProfile->parseHeader(rows[0]);
    if (!append)
        list.clear();
    for (int i=firstLine; i<rows.count(); i++) {
        ContactItem item;
        item.originalFormat = "CSV";
        list.originalProfile = currentProfile->name();
        currentProfile->importRecord(rows[i], item, _errors);
        item.calculateFields();
        list << item;
    }
    // For new profiles debug
    /* std::cout << url.toLocal8Bit().data() << std::endl;
     std::cout << rows[0].count() << " " << rows[1].count()
              << "|" << rows[0].last().toLocal8Bit().data() << std::endl; */
    // Ready
    return (!list.isEmpty());
}

bool CSVFile::exportRecords(const QString &url, ContactList &list)
{
    if (!currentProfile)
        return false;
    if (!currentProfile->prepareExport(list))
        return false;
    QList<QStringList> rows;
    foreach (const ContactItem& item, list) {
        QStringList row;
        currentProfile->exportRecord(row, item, _errors);
        rows << row;
    }
    if (!openFile(url, QIODevice::WriteOnly))
        return false;
    QTextStream stream(&file);
    if (_encoding.isEmpty())
        _encoding = currentProfile->charSet();
    stream.setCodec(_encoding.toLatin1().data());
    stream.setGenerateByteOrderMark(currentProfile->hasBOM());
    // Header
    if (currentProfile->hasHeader())
        putLine(stream, currentProfile->makeHeader());
    // Items
    foreach (const QStringList& row, rows)
        putLine(stream, row);
    closeFile();
    return true;
}
void CSVFile::putLine(QTextStream& stream, const QStringList &source)
{
    // Quoting
    switch (currentProfile->quotingPolicy()) {
    case CSVProfileBase::AlwaysQuote:
        stream << QString("\"%1\"").arg(source.join("\"%1\"").arg(_separator));
        break;
    case CSVProfileBase::NeverQuote:
        stream << source.join(_separator);
        break;
    case CSVProfileBase::QuoteIfNeed:
        QString line;
        foreach (const QString& cell, source) {
            if (cell.contains(_separator))
                line += QString("\"%1\"").arg(cell);
            else
                line += cell;
            line += _separator;
        }
        if (line.right(1)==_separator)
            line.remove(line.length()-_separator.length(), _separator.length());
        stream << line;
    }
    // Line ending
    if (currentProfile->lineEnding()==CSVProfileBase::CRLFEnding)
        stream << (char)13;
    stream << ENDL;
}
