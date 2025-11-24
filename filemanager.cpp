#include "filemanager.h"
#include <QUrl>

FileManager::FileManager(QObject *parent) 
    : QObject(parent), filesDir("files")
{
    if (!filesDir.exists()) {
        filesDir.mkpath(".");
    }
}

QString FileManager::saveFile(const QByteArray &fileData, const QString &fileName, const QString &senderIP)
{
    QString uniqueFileName = generateUniqueFileName(fileName);
    QString filePath = filesDir.filePath(uniqueFileName);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(fileData);
        file.close();
        qDebug() << "File saved:" << filePath;
        return uniqueFileName;
    }

    return QString();
}

QByteArray FileManager::loadFile(const QString &fileName)
{
    QString filePath = filesDir.filePath(fileName);
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        return data;
    }
    return QByteArray();
}

bool FileManager::fileExists(const QString &fileName) const
{
    return QFile::exists(filesDir.filePath(fileName));
}

QStringList FileManager::getFileList() const
{
    return filesDir.entryList(QDir::Files, QDir::Time);
}

QString FileManager::getMimeType(const QString &fileName) const
{
    QString filePath = filesDir.filePath(fileName);
    return mimeDb.mimeTypeForFile(filePath).name();
}

QString FileManager::generateUniqueFileName(const QString &originalName)
{
    QString baseName = QFileInfo(originalName).baseName();
    QString extension = QFileInfo(originalName).suffix();
    QString safeBaseName = QUrl::toPercentEncoding(baseName, "", "_");

    return QString("%1_%2.%3")
        .arg(safeBaseName)
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz"))
        .arg(extension);
}