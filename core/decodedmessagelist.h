/* Double Contact
 *
 * Module: Decodes SMS structures
 *
 * Copyright 2016 Mikhail Y. Zvyozdochkin aka DarkHobbit <pub@zvyozdochkin.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See COPYING file for more details.
 *
 */

#ifndef DECODEDMESSAGELIST_H
#define DECODEDMESSAGELIST_H

#include <QFlags>
#include "contactlist.h"

enum MessageSourceFlag {
    useVMessage        = 0x01,
    useVMessageArchive = 0x02,
    usePDU             = 0x04,
    usePDUArchive      = 0x08,
    useBinary          = 0x10,
    mergeDuplicates    = 0x20,
    mergeMultiParts    = 0x40
};

typedef QFlags<MessageSourceFlag> MessageSourceFlags;

struct DecodedMessage {
    QString version;
    enum MsgStatus { // X-IRMC-STATUS
        Read,
        Unread,
        Unknown // if messageformat not support read status (MPB)
    } status;
    enum MsgBox { // X-IRMC-BOX
        Inbox,
        Outbox,
        Sentbox,
        Draft,
        Trash // this value not appear in known to me vmsg files
    } box;
    QString id; // X-IRMC-LUID
    QString subFolder;
    bool delivered; // TODO maybe other X-MESSAGE-TYPE exists
    ContactList contacts;
    QDateTime when;
    QString text;
    bool isMultiPart;
    int partNumber, totalParts;
    bool isMMS;
    QString mmsSubject;
    InnerFiles mmsFiles;
    // Calculated fields
    QString contactName;
    void clear();
    QString contactsToString() const;
    MessageSourceFlags sources;
    bool operator <(const DecodedMessage& pair) const;
};

class DecodedMessageList : public QList<DecodedMessage>
{
public:
    int mmsCount, mergeDupCount, mergeMultiPartCount;
    DecodedMessageList(bool mergeDuplicates, bool mergeMultiParts);
    bool toCSV(const QString& path);
    bool saveAllMMSFiles(const QString& dirPath, QString& fatalError) const;
    static DecodedMessageList fromContactList(const ContactList& list, const MessageSourceFlags& flags, QStringList &errors);
    void bindToContacts(const ContactList& list);
    QString messageBoxes(int index) const;
    QString messageStates(int index, bool delivered) const;
    void addOrMerge(DecodedMessage& msg);
    void sort();
private:
    bool _mergeDuplicates, _mergeMultiParts;
    QStringList sMsgStatus, sMsgBox;
    QString multiText; // inter-parts buffer for addOrMerge
    static void fromVMessageList(DecodedMessageList& messages, const QStringList& src, QStringList &errors, bool fromArchive);
    static void fromPDUList(DecodedMessageList& messages, const QStringList& src, QStringList &errors, bool fromArchive);
};

#endif // DECODEDMESSAGELIST_H
