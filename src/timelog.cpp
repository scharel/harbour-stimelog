#include "timelog.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

TimeLog::TimeLog(QObject *parent) : QAbstractListModel(parent)
{
    QDir appDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (appDir.mkpath(".")) {
        qDebug() << "Directory created:" << appDir.path();
    }

    m_reloadTimer.setInterval(1000);
    connect(&m_reloadTimer, SIGNAL(timeout()), this, SLOT(reload()));

    connect(this, SIGNAL(timelogFileChanged(QString)), this, SLOT(reload()));
    connect(this, SIGNAL(timelogFileChanged(QString)), &m_reloadTimer, SLOT(start()));
    setTasksFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTasksFilename);
    setTimelogFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTimelogFilename);
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
    QVariant data;
    if (index.isValid() && index.row() < m_timelog.size()) {
        QString line = m_timelog.at(index.row());
        int prevIndex = index.row() - 1;
        QDateTime prevTime;
        switch (role) {
        case StartTimeRole:
            while (prevIndex >= 0) {
                prevTime = getTime(m_timelog.at(prevIndex));
                if (prevTime.isValid()) {
                    if (prevTime.daysTo(getTime(line)) != 0) {
                        prevTime = QDateTime();
                    }
                    prevIndex = 0;
                }
                prevIndex--;
            }
            data = prevTime;
            break;
        case EndTimeRole:
            data = getTime(line);
            break;
        case DurationRole:
            while (prevIndex >= 0) {
                prevTime = getTime(m_timelog.at(prevIndex));
                if (prevTime.isValid()) {
                    if (prevTime.daysTo(getTime(line)) != 0) {
                        prevTime = QDateTime();
                    }
                    prevIndex = 0;
                }
                prevIndex--;
            }
            data = prevTime.secsTo(getTime(line)) / 60;
            break;
        case ProjectRole:
            data = getProject(line);
            break;
        case TaskRole:
            data = getTask(line);
            break;
        case SlackingRole:
            data = isSlacking(line);
            break;
        case CommentRole:
            if (isComment(line)) {
                data = line;
            }
            break;
        }
    }
    return data;
}

bool TimeLog::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

bool TimeLog::removeRows(int row, int count, const QModelIndex &parent) {
    if (row >= 0 && row + count <= m_timelog.size()) {
        for (int i = row; i < row + count; ++i) {
            m_timelog.removeAt(row);
        }
        return true;
    }
    return false;
}

bool TimeLog::addData(const QString &input) {
    if (m_timelogFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QString line = input;
        if (!input.startsWith('#')) {
            line.prepend(QString(QDateTime::currentDateTime().toString(DefaultDateTimeFormat) + ": "));
        }
        beginInsertRows(QModelIndex(), m_timelog.size(), m_timelog.size());
        m_timelog.append(line);
        m_timelogFile.write(line.append('\n').toUtf8());
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

const QDateTime TimeLog::lastTime() const {
    QDateTime time;
    for (int i = m_timelog.size()-1; i >= 0 && !time.isValid(); --i) {
        time = getTime(m_timelog.at(i));
    }
    return time;
}

const QDateTime TimeLog::getTime(const QString &line) {
    QDateTime time;
    if (line.contains(": ")) {
        time = QDateTime::fromString(line.section(':', 0, DefaultDateTimeFormat.count(':')), DefaultDateTimeFormat);
    }
    return time;
}

const QString TimeLog::getProjectAndTask(const QString &line) {
    QString projectAndTask;
    if (!line.isEmpty() && !isComment(line)) {
        projectAndTask = line.section(':', DefaultDateTimeFormat.count(':') + 1);
        if (isSlacking(line)) {
            projectAndTask = projectAndTask.section(" **", 0);
        }
    }
    return projectAndTask;
}

const QString TimeLog::getProject(const QString &line) {
    QString project;
    if (!line.isEmpty() && !isComment(line)) {
        project = getProjectAndTask(line);
        if (project.contains(": ")) {
            project = project.section(": ", 0, 0);
        }
        else {
            project.clear();
        }
    }
    return project;
}

const QString TimeLog::getTask(const QString &line) {
    QString task;
    if (!line.isEmpty() && !isComment(line)) {
        task = getProjectAndTask(line);
        if (task.contains(": ")) {
            task = task.section(": ", 1);
        }
    }
    return task;
}

bool TimeLog::isSlacking(const QString &line) {
    return !isComment(line) && line.endsWith(" **");
}

bool TimeLog::isComment(const QString &line) {
    //return line.startsWith('#');
    return !line.isEmpty() && !getTime(line).isValid();
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
        //qDebug() << m_timelog;
        return true;
    }
    qDebug() << "Could not open" << m_timelogFile.fileName();
    return false;
}
