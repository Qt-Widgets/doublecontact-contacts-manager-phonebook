/* Double Contact
 *
 * Module: Nokia NBU backup file import
 *
 * Copyright 2018 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 * Some part of this code ported from NbuExplorer project
 * http://sourceforge.net/projects/nbuexplorer/
 * Author: Petr Vilem, petrusek@seznam.cz

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#include <QtAlgorithms>
#include <QDataStream>
#include <QFile>
#include "nbufile.h"

#define SUMMARY_OFFSET_OFFSET 0x00000014
#define SUMMARY_OFFSET 0x00000014
#define S_UNSUPPORTED_SECTION QObject::tr("Unsupported NBU file section type: %1")

const QString ptContacts = "Contacts";
const QString ptGroups = "Groups";
const QString ptBookmarks = "Bookmarks";
const QString ptMessages = "Messages";
const QString ptMms = "MMS";
const QString ptSettings = "Settings";

NBUSectionType nbuSectionTypes[17] =
{
{{
    0x08, 0x29, 0x4B, 0x2B, 0x0E, 0x89, 0x17, 0x4B,
    0x97, 0x73, 0x17, 0xC2, 0x4C, 0x1A, 0xDB, 0xC8 },
NBUSectionType::FileSystem, "Internal files", ""},

{{
    0xEF, 0xD4, 0x2E, 0xD0, 0xA3, 0x51, 0x38, 0x47,
    0x9D, 0xD7, 0x30, 0x5C, 0x7A, 0xF0, 0x68, 0xD3 },
NBUSectionType::Vcards, ptContacts, ""},

{{
    0x1F, 0x0E, 0x58, 0x65, 0xA1, 0x9F, 0x3C, 0x49,
    0x9E, 0x23, 0x0E, 0x25, 0xEB, 0x24, 0x0F, 0xE1 },
NBUSectionType::Groups, ptGroups, ""},

{{
    0x16, 0xCD, 0xF8, 0xE8, 0x23, 0x5E, 0x5A, 0x4E,
    0xB7, 0x35, 0xDD, 0xDF, 0xF1, 0x48, 0x12, 0x22 },
NBUSectionType::Vcards, "Calendar", ""},

{{
    0x5C, 0x62, 0x97, 0x3B, 0xDC, 0xA7, 0x54, 0x41,
    0xA1, 0xC3, 0x05, 0x9D, 0xE3, 0x24, 0x68, 0x08 },
NBUSectionType::Memos, "Memo", ""},

{{
    0x61, 0x7A, 0xEF, 0xD1, 0xAA, 0xBE, 0xA1, 0x49,
    0x9D, 0x9D, 0x15, 0x5A, 0xBB, 0x4C, 0xEB, 0x8E },
NBUSectionType::GeneralFolders, ptMessages, ""},

{{
    0x47, 0x1D, 0xD4, 0x65, 0xEF, 0xE3, 0x32, 0x40,
    0x8C, 0x77, 0x64, 0xCA, 0xA3, 0x83, 0xAA, 0x33 },
NBUSectionType::GeneralFolders, ptMms, ""},

{{
    0x7F, 0x77, 0x90, 0x56, 0x31, 0xF9, 0x57, 0x49,
    0x8D, 0x96, 0xEE, 0x44, 0x5D, 0xBE, 0xBC, 0x5A },
NBUSectionType::Vcards, ptBookmarks, ""},

{{
0x60, 0xC2, 0xCB, 0x9C, 0x7E, 0x73, 0x24, 0x41,
0x8D, 0x90, 0x2E, 0xC0, 0xD9, 0xB0, 0xB6, 0x8C },
NBUSectionType::GeneralFolders, ptSettings, "Contacts"},

{{
0x2D, 0xED, 0xC7, 0x29, 0x57, 0x68, 0x22, 0x45,
0xAE, 0xD4, 0xEB, 0x21, 0x02, 0x96, 0xA1, 0xEE },
NBUSectionType::GeneralFolders, ptSettings, "Calendar"},

{{
0x0A, 0xDF, 0x77, 0x94, 0xF7, 0x82, 0xBC, 0x48,
0xAB, 0xA3, 0x78, 0x91, 0xDB, 0xDB, 0xD0, 0xCF },
NBUSectionType::GeneralFolders, ptSettings, "Messages"},

{{
0x79, 0x00, 0x47, 0xC6, 0x6E, 0xCE, 0x7A, 0x44,
0x81, 0xFB, 0x30, 0x7C, 0xD9, 0x56, 0xAB, 0x10 },
NBUSectionType::GeneralFolders, ptSettings, "Basic"},

{{
0x77, 0xA4, 0x23, 0x6A, 0x99, 0xBA, 0x9A, 0x4B,
0xAB, 0x16, 0xD5, 0x7B, 0x76, 0x0F, 0x16, 0xD9 },
NBUSectionType::GeneralFolders, ptSettings, "Bookmarks"},

{{
0x2D, 0xF5, 0x68, 0x6B, 0x1F, 0x4B, 0x22, 0x4A,
0x92, 0x83, 0x1B, 0x06, 0xC3, 0xC3, 0x9A, 0x35 },
NBUSectionType::GeneralFolders, ptSettings, "Advanced"},

{{
0xAD, 0x3A, 0x1B, 0xEC, 0x97, 0x71, 0xB7, 0x42,
0xA5, 0x67, 0x54, 0xE2, 0xD3, 0x19, 0xF4, 0x89 },
NBUSectionType::FileSystem, "Memorycard files", ""},

{{
0x0E, 0x3D, 0x5F, 0x65, 0xAF, 0x22, 0x78, 0x48,
0x93, 0x9E, 0xCD, 0x59, 0xA4, 0x5D, 0xF1, 0x29 },
NBUSectionType::FileSystem, "Files", ""},

{{
0x23, 0x91, 0x96, 0x9F, 0x60, 0x33, 0x85, 0x43,
0xA5, 0xDE, 0x57, 0x8E, 0x10, 0x70, 0xFA, 0x97 },
NBUSectionType::Sbackup, ptSettings, "Backup",}

};

NBUFile::NBUFile()
    :FileFormat()
{
}

bool NBUFile::detect(const QString &url)
{
    // Check 4 first bytes as signature
    QFile f(url);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QDataStream ss(&f);
    ss.setByteOrder(QDataStream::LittleEndian);
    quint32 sig;
    ss >> sig;
    f.close();
    return (sig==0xFC3352cc);
}

QStringList NBUFile::supportedExtensions()
{
    return (QStringList() << "nbu" << "NBU");
}

QStringList NBUFile::supportedFilters()
{
    return (QStringList() << "Nokia NBU (*.nbu *.NBU)");
}

#include <iostream>
bool NBUFile::importRecords(const QString &url, ContactList &list, bool append)
{
    if (!detect(url))
        _errors << QObject::tr("Signature not recognized, trying anyway...");
    if (!openFile(url, QIODevice::ReadOnly))
        return false;
    if (!append)
        list.clear();
    QDataStream ss(&file);
    ss.setByteOrder(QDataStream::LittleEndian);
    if (!file.seek(SUMMARY_OFFSET_OFFSET)) {
        _fatalError = S_SEEK_ERR.arg(SUMMARY_OFFSET_OFFSET);
        closeFile();
        return false;
    }
    quint64 sumOffset;
    ss >> sumOffset;
    if (!file.seek(sumOffset+SUMMARY_OFFSET)) {
        _fatalError = S_SEEK_ERR.arg(sumOffset+SUMMARY_OFFSET);
        closeFile();
        return false;
    }
    // Common NBU attributes
    list.extra.timeStamp = getDateTime(ss);
    list.extra.imei = getString16c(ss);
    list.extra.model = getString16c(ss); // name, then model
    list.extra.model = getString16c(ss) + " " + list.extra.model;
    list.extra.firmware = getString16c(ss);
    list.extra.phoneLang = getString16c(ss);
    // NBU sections
    if (!file.seek(file.pos()+  0x14  )) {
        _fatalError = S_SEEK_ERR.arg(file.pos()+  0x14  );
        closeFile();
        return false;
    }
    // Read sections
    quint32 sectCount;
    ss >> sectCount;
std::cout << "sec cnt = " << sectCount << std::endl; //===>
    for (quint32 i=0; i<sectCount; i++) {
        char sectID[NBU_SECT_ID_SIZE];
        ss.readRawData(sectID, sizeof(sectID));
        quint64 sectStart;
        ss >> sectStart;
        if (!file.seek(file.pos()+8)) {
            _errors << S_SEEK_ERR.arg(file.pos()+8);
            closeFile();
            return true; // Partial read?
        }
        NBUSectionType* section = findSectionType(sectID);
        if (!section) {
            /*
            _fatalError = QObject::tr("Unknown NBU file section type");
            closeFile();
            return false;*/
            _errors << QObject::tr("Unknown NBU file section type");
            continue;
        }        
        quint32 count, count2;
        quint64 partPos;
        QString folderName;
std::cout << section->type << "::" << section->name.toLocal8Bit().data() << "::" << section->name2.toLocal8Bit().data() << std::endl;
        switch (section->type) {
        /*case ProcessType.FileSystem: TODO*/
        case NBUSectionType::Vcards:
            ss >> count >> count2;
std::cout << "VCards " << count << "!"<< count2 << std::endl;
            if (count2>0) {
                for (quint32 j = 0; j < count2; j++) {
                    file.seek(file.pos()+4); //folder id
                    quint64 folderAddr;
                    ss >> folderAddr;
                    partPos = file.pos();
                    file.seek(folderAddr + 4);
                    folderName = getString16c(ss);
std::cout << folderAddr << " >> " << folderName.toLocal8Bit().data() << std::endl;
                    if (!parseFolderVcard(ss, list, section->name)) {
                        _errors << QObject::tr("Unknown vcard folder structure at section %1, subsection %2")
                            .arg(i).arg(j);
                    }
                    file.seek(partPos);
                }
            }
            else {
std::cout << "No folders" << std::endl;
                quint64 partPos = file.pos();
                file.seek(sectStart + 0x2C);
                if (!parseFolderVcard(ss, list, section->name)) {
                    _errors << QObject::tr("Unknown vcard folder structure at section %1, subsection %2")
                        .arg(i).arg("0");
                }
                file.seek(partPos);
            }
            break;
        /*case ProcessType.Memos: TODO*/
        case NBUSectionType::Groups:
            ss >> count2 >> count;
            std::cout << "Groups " << count << "!"<< count2 << std::endl;
            for (quint32 j = 0; j < count; j++)
            {
                file.seek(file.pos() + 4);
                quint64 start;
                ss >> start;
                partPos = file.pos();
                file.seek(start + 4);
                folderName = getString16c(ss);
                ss >> count2;
                std::cout << " contacts in group " << count2 << std::endl;
                if (!list.isEmpty())
                {
                    for (quint32 k = 0; k < count2; k++)
                    {
                        quint32 ix; // contact index, from 1
                        ss >> ix;
                        if ((quint32)list.count() >= ix) {
                            ContactItem& item = list[ix - 1];
                            item.groups << folderName;
                        }
                        else
                            _errors << QObject::tr("Invalid index: %1").arg(ix);
                    }
                }
                file.seek(partPos);
            }
            break;
        case NBUSectionType::GeneralFolders:
        case NBUSectionType::Sbackup:
        {
            ss >> count2 >> count;
            partPos = file.pos();
            QString zipTest = "";
            //long tmpCnt = StreamUtils.Counter;
            if (!file.seek(sectStart + 73)) {
                _fatalError = S_SEEK_ERR.arg(sectStart + 73);
                closeFile();
                return false;
            }
            zipTest = getString16c(ss);
//            StreamUtils.Counter = tmpCnt;
            if (zipTest == "application/vnd.nokia-backup")
            {
                _errors << S_UNSUPPORTED_SECTION.arg(zipTest); // TODO
                return true; //===>
                /*
                           long zipOffset = file.pos();
                ZipInputStream zi = new ZipInputStream(fs);
                zi.IsStreamOwner = false;
                parseFolderZip(currentFileName, zi, zipOffset, sect.name);
                fs.Seek(partPos, SeekOrigin.Begin);*/

            }
            else if (section->name2 == "Messages" && count == 0)
            {
                _errors << S_UNSUPPORTED_SECTION.arg(zipTest); // TODO
                return true; //===>
                /*
                file.seek(sectStart);
                parseBinaryMessages(currentFileName, fs);
                file.seek(partPos);
                */
            }
            else
            {
                file.seek(partPos);
                std::cout << "Folders " << count << "!"<< count2 << std::endl;
                for (quint32 j = 0; j < count; j++)
                {
                    file.seek(file.pos() + 4);
                    quint64 start;
                    ss >> start;
                    partPos = file.pos();
                    parseFolder(ss, start, section->name);
                    file.seek(partPos);
                }
            }
            break;
        }
        default:
            _errors << S_UNSUPPORTED_SECTION.arg(section->name);
        } // switch section type
    }
    closeFile();
    return true;
}

bool NBUFile::exportRecords(const QString &, ContactList &)
{
    return false;
}

QDateTime NBUFile::getDateTime(QDataStream &stream)
{
    // NBU use swapped Windows file time
    union TimeAs64 {
        quint64 value;
        struct {
            quint32 low, high;
        };
    } tm;
    stream >> tm.value;
    qSwap(tm.low, tm.high);
    QDateTime winEpoch(QDate(1601, 1, 1), QTime(0, 0, 0));
    return winEpoch.addMSecs(tm.value/10000);
}

QString NBUFile::getString16c(QDataStream& stream)
{
    quint16 cnt;
    stream >> cnt;
    ushort* raw = new ushort[cnt];
    stream.readRawData((char*)raw, 2*cnt);
    QString res = QString::fromUtf16(raw, cnt);
    delete raw;
    return res;
}

NBUSectionType *NBUFile::findSectionType(char *sectID)
{
    for(unsigned int i=0; i<sizeof(nbuSectionTypes)/sizeof(NBUSectionType); i++) {
        NBUSectionType* cand = &nbuSectionTypes[i];
        bool found = true;
        for (int j=0; j<NBU_SECT_ID_SIZE; j++)
            if  (cand->id[j]!=sectID[j]) {
                found = false;
                break;
            }
        if (found)
            return cand;
    }
    return 0;
}

bool NBUFile::parseFolderVcard(QDataStream &stream, ContactList &list, const QString &sectName)
{
    quint32 count;
    stream >> count;
    // Valid sect names for vCard: Contacts, Bookmarks, Calendar
    if (sectName!=ptContacts && sectName!=ptBookmarks && sectName!="Calendar") {
        _errors << S_UNSUPPORTED_SECTION.arg(sectName);
        return false;
    }
std::cout << "count=" << count << std::endl;
    for (quint32 i = 0; i < count; i++)
    {
        quint32 test;
        stream >> test;
        if (test == 0x10)
        {
            stream >> test;
            if (test > 1)
                _errors << QObject::tr("Test 2 greater than 0x01: %1").arg(test, 0, 16);
        }
        else
            _errors << QObject::tr("Test 1 different than 0x10: %1").arg(test, 0, 16);
        int vcLen;
        stream >> vcLen;
//std::cout << "i=" << i << " vcLen " << vcLen << std::endl;
        char* raw = new char[vcLen];
        stream.readRawData((char*)raw, vcLen);
        QString vCard = QString::fromUtf8(raw, vcLen);
//std::cout << "vCard =" << vCard.toLocal8Bit().data() << std::endl;
        QStringList content = vCard.split("\x0d\n");
        VCardData::importRecords(content, list, true, _errors);
    }
    return true;
}

bool NBUFile::parseFolder(QDataStream &stream, long start, const QString &sectName)
{
    return false; // TODO
}