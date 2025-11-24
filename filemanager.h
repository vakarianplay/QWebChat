#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QMimeDatabase>

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);

    QString saveFile(const QByteArray &fileData, const QString &fileName, const QString &senderIP);
    QByteArray loadFile(const QString &fileName);
    bool fileExists(const QString &fileName) const;
    QStringList getFileList() const;
    QString getMimeType(const QString &fileName) const;

private:
    QDir filesDir;
    QMimeDatabase mimeDb;

    QString generateUniqueFileName(const QString &originalName);
};

#endif // FILEMANAGER_H