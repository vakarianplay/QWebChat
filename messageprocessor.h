#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

#include <QObject>
#include <QFile>

class MessageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MessageProcessor(QObject *parent = nullptr);
    ~MessageProcessor();

    void logMessage(const QString &ip, const QString &date,
                    const QString &message, const QString &type = "text",
                    const QString &fileInfo = "");
    QString getLastMessages(int count = 50) const;
    QString formatMessage(const QString &ip, const QString &date,
                          const QString &message, const QString &type = "text",
                          const QString &fileInfo = "") const;

private:
    mutable QFile chatHistoryFile;  // Добавляем mutable

    void ensureFileHeader();
    QString escapeCsv(const QString &text) const;
};

#endif // MESSAGEPROCESSOR_H
