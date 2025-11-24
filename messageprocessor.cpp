#include "messageprocessor.h"
#include <QTextStream>
#include <QDebug>

MessageProcessor::MessageProcessor(QObject *parent)
    : QObject(parent), chatHistoryFile("chat_history.csv")
{
    ensureFileHeader();
}

MessageProcessor::~MessageProcessor()
{
    if (chatHistoryFile.isOpen()) {
        chatHistoryFile.close();
    }
}

void MessageProcessor::ensureFileHeader()
{
    if (chatHistoryFile.open(QIODevice::ReadWrite)) {
        if (chatHistoryFile.size() == 0) {
            QTextStream out(&chatHistoryFile);
            out << "IP,Date,Message,Type,File\n";
        }
        chatHistoryFile.close();
    }
}

void MessageProcessor::logMessage(const QString &ip, const QString &date,
                                  const QString &message, const QString &type,
                                  const QString &fileInfo)
{
    if (chatHistoryFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&chatHistoryFile);
        out << QString("%1,%2,%3,%4,%5\n")
                   .arg(ip)
                   .arg(date)
                   .arg(escapeCsv(message))
                   .arg(type)
                   .arg(escapeCsv(fileInfo));
        chatHistoryFile.close();
    }
}

QString MessageProcessor::getLastMessages(int count) const
{
    if (!chatHistoryFile.exists()) {
        return QString();
    }

    // Используем локальную переменную для чтения
    QFile file("chat_history.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    file.close();

    if (lines.size() <= 1) return QString();

    int startIndex = qMax(1, lines.size() - count);
    return lines.mid(startIndex).join("\n");
}

QString MessageProcessor::formatMessage(const QString &ip, const QString &date,
                                        const QString &message, const QString &type,
                                        const QString &fileInfo) const
{
    if (type == "file" && !fileInfo.isEmpty()) {
        return QString("<span style=\"color: blue; font-weight: bold;\">[%1]</span> "
                       "<span style=\"color: gray;\">[%2]</span> "
                       "<a href=\"/files/%3\" style=\"color: #667eea; text-decoration: none;\">📎 %4</a>")
            .arg(ip, date, fileInfo, message);
    } else {
        return QString("<span style=\"color: blue; font-weight: bold;\">[%1]</span> "
                       "<span style=\"color: gray;\">[%2]</span> %3")
            .arg(ip, date, message);
    }
}

QString MessageProcessor::escapeCsv(const QString &text) const
{
    QString escaped = text;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}
