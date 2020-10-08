#include "timelog.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>

TimeLog::TimeLog(QObject *parent) : QAbstractListModel(parent)
{
    m_refreshTimer.setInterval(1000);
    connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    connect(this, SIGNAL(timelogFileChanged(QString)), this, SLOT(refresh()));
    connect(this, SIGNAL(timelogFileChanged(QString)), &m_refreshTimer, SLOT(start()));
    setTimelogFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTimelogFilename);
    setTasksFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTasksFilename);
}

TimeLog::~TimeLog() {
    m_refreshTimer.stop();
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
    return 0;
}

QVariant TimeLog::data(const QModelIndex &index, int role) const {
    return QVariant();
}

bool TimeLog::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

bool TimeLog::addData(const QString &input) {
    return false;
}

void TimeLog::setTimelogFile(const QString &fileName) {
    QFileInfo newFile(fileName);
    if (newFile.isFile())
        newFile.setFile(fileName);
    else if (newFile.isDir())
        newFile.setFile(QDir(fileName), DefaultTimelogFilename);

    if (newFile.absoluteFilePath() != QFileInfo(m_timelogFile).absoluteFilePath()) {
        m_refreshTimer.stop();
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

bool TimeLog::refresh() {
    if (m_timelogFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        m_timelog = m_timelogFile.readAll();
        /*QTextStream stream(&m_timelogFile);
        stream.seek(0);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            QDateTime time = QDateTime::fromString(line.split(": ")[0], DateTimeFormat);
            line = line.split(": ")[1];
            qDebug() << time.toString() << line;
        }*/
        m_timelogFile.close();
    }
    return false;
}
