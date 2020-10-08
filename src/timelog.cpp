#include "timelog.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>

TimeLog::TimeLog(QObject *parent) : QAbstractListModel(parent)
{
    m_reloadTimer.setInterval(1000);
    connect(&m_reloadTimer, SIGNAL(timeout()), this, SLOT(reload()));
    connect(this, SIGNAL(timelogFileChanged(QString)), this, SLOT(reload()));
    connect(this, SIGNAL(timelogFileChanged(QString)), &m_reloadTimer, SLOT(start()));
    QDir appDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    appDir.mkdir(".");
    setTimelogFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTimelogFilename);
    setTasksFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTasksFilename);
}

TimeLog::~TimeLog() {
    m_reloadTimer.stop();
}

QHash<int, QByteArray> TimeLog::roleNames() const {
    return QHash<int, QByteArray> ( {
                                        {TimeLog::StartTimeRole, "startTime"},
                                        {TimeLog::EndTimeRole, "endTime"},
                                        {TimeLog::DurationRole, "duration"},
                                        {TimeLog::ProjectRole, "project"},
                                        {TimeLog::TaskRole, "task"},
                                        {TimeLog::SlackingRole, "slacking"},
                                        {TimeLog::CommentRole, "comment"}
                                    } );
}

Qt::ItemFlags TimeLog::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable; // | Qt::ItemIsSelectable
    }
    else {
        return Qt::NoItemFlags;
    }
}

int TimeLog::rowCount(const QModelIndex &parent) const {
    return m_timelog.size();
}

QVariant TimeLog::data(const QModelIndex &index, int role) const {
    return QVariant();
}

bool TimeLog::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

bool TimeLog::addData(const QString &input) {
    if (m_timelogFile.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)) {
        QString line = QString(QDateTime::currentDateTime().toString(DateTimeFormat) + ": " + input);
        beginInsertRows(QModelIndex(), m_timelog.size(), m_timelog.size());
        m_timelog.append(line);
        m_timelogFile.write(line.toUtf8());
        endInsertRows();
        m_timelogFile.close();
        return true;
    }
    qDebug() << "Could not open" << m_timelogFile.fileName();
    return false;
}

void TimeLog::setTimelogFile(const QString &fileName) {
    QFileInfo newFile(fileName);
    if (newFile.isFile())
        newFile.setFile(fileName);
    else if (newFile.isDir())
        newFile.setFile(QDir(fileName), DefaultTimelogFilename);

    if (newFile.absoluteFilePath() != QFileInfo(m_timelogFile).absoluteFilePath()) {
        m_reloadTimer.stop();
        if (m_timelogFile.isOpen())
            m_timelogFile.close();
        m_timelogFile.setFileName(newFile.absoluteFilePath());
        qDebug() << QFileInfo(m_timelogFile).absoluteFilePath();
        emit timelogFileChanged(QFileInfo(m_timelogFile).absoluteFilePath());
    }
}

void TimeLog::setTasksFile(const QString &fileName) {
    QFileInfo newFile(fileName);
    if (newFile.isFile())
        newFile.setFile(fileName);
    else if (newFile.isDir())
        newFile.setFile(QDir(fileName), DefaultTasksFilename);

    if (newFile.absoluteFilePath() != QFileInfo(m_tasksFile).absoluteFilePath()) {
        if (m_tasksFile.isOpen())
            m_tasksFile.close();
        m_tasksFile.setFileName(newFile.absoluteFilePath());
        qDebug() << QFileInfo(m_tasksFile).absoluteFilePath();
        emit tasksFileChanged(QFileInfo(m_tasksFile).absoluteFilePath());
    }
}

bool TimeLog::reload() {
    if (m_timelogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QStringList tmpTimeLog = QString(m_timelogFile.readAll()).split('\n', QString::SkipEmptyParts);
        m_timelogFile.close();
        if (m_timelog != tmpTimeLog) {
            for (int i=0; i < m_timelog.size() && i < tmpTimeLog.size(); ++i) {
                if (m_timelog.at(i) != tmpTimeLog.at(i)) {
                    m_timelog[i] = tmpTimeLog.at(i);
                    emit dataChanged(index(i), index(i));
                }
            }
            if (m_timelog.size() < tmpTimeLog.size()) {
                beginInsertRows(QModelIndex(), m_timelog.size(), tmpTimeLog.size()-1);
                for (int i = m_timelog.size(); i < tmpTimeLog.size(); ++i) {
                    m_timelog.append(tmpTimeLog.at(i));
                }
                endInsertRows();
            }
            else if (m_timelog.size() > tmpTimeLog.size()) {
                beginRemoveRows(QModelIndex(), tmpTimeLog.size(), m_timelog.size()-1);
                for (int i = m_timelog.size()-1; i >= tmpTimeLog.size(); --i) {
                    m_timelog.removeLast();
                }
                endRemoveRows();
            }
        }
        qDebug() << m_timelog;
        return true;
    }
    qDebug() << "Could not open" << m_timelogFile.fileName();
    return false;
}
