#include "TransferHistory.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

QJsonObject TransferRecord::toJson() const
{
    QJsonObject json;
    json["taskId"] = taskId;
    json["fileName"] = fileName;
    json["sourcePath"] = sourcePath;
    json["targetPath"] = targetPath;
    json["fileSize"] = QString::number(fileSize);
    json["isSuccess"] = isSuccess;
    json["errorMessage"] = errorMessage;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["startTime"] = startTime.toString(Qt::ISODate);
    json["endTime"] = endTime.toString(Qt::ISODate);
    return json;
}

TransferRecord TransferRecord::fromJson(const QJsonObject& json)
{
    TransferRecord record;
    record.taskId = json["taskId"].toString();
    record.fileName = json["fileName"].toString();
    record.sourcePath = json["sourcePath"].toString();
    record.targetPath = json["targetPath"].toString();
    record.fileSize = json["fileSize"].toString().toLongLong();
    record.isSuccess = json["isSuccess"].toBool();
    record.errorMessage = json["errorMessage"].toString();
    record.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    record.startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);
    record.endTime = QDateTime::fromString(json["endTime"].toString(), Qt::ISODate);
    return record;
}

TransferHistory& TransferHistory::instance()
{
    static TransferHistory history;
    return history;
}

TransferHistory::TransferHistory(QObject* parent)
    : QObject(parent)
{
    // 设置历史记录文件路径
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_historyFile = dataPath + "/transfer_history.json";
    
    // 加载历史记录
    load();
}

TransferHistory::~TransferHistory()
{
    save();
}

void TransferHistory::addRecord(const TransferRecord& record)
{
    m_records.prepend(record);
    while (m_records.size() > MAX_RECORDS) {
        m_records.removeLast();
    }
    save();
}

QList<TransferRecord> TransferHistory::getRecords(const QDateTime& from, const QDateTime& to) const
{
    if (!from.isValid() && !to.isValid()) {
        return m_records;
    }
    
    QList<TransferRecord> filtered;
    for (const auto& record : m_records) {
        if ((!from.isValid() || record.timestamp >= from) &&
            (!to.isValid() || record.timestamp <= to)) {
            filtered.append(record);
        }
    }
    return filtered;
}

void TransferHistory::clear()
{
    clearRecords();
}

void TransferHistory::clearRecords()
{
    m_records.clear();
    save();
}

bool TransferHistory::save() const
{
    QFile file(m_historyFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonArray array;
    for (const auto& record : m_records) {
        array.append(record.toJson());
    }
    
    QJsonDocument doc(array);
    file.write(doc.toJson());
    return true;
}

bool TransferHistory::load()
{
    QFile file(m_historyFile);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return false;
    }
    
    m_records.clear();
    QJsonArray array = doc.array();
    for (const auto& value : array) {
        if (value.isObject()) {
            m_records.append(TransferRecord::fromJson(value.toObject()));
        }
    }
    
    return true;
} 