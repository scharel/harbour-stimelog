#include "timelog.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>

TimeLog::TimeLog(QObject *parent) : QAbstractListModel(parent)
{
    QDir::setCurrent(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QDir appDir;
    appDir.mkpath(QDir::currentPath());

    setTasksFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTasksFilename);
    setTimelogFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + DefaultTimelogFilename);

    m_reloadTimer.setInterval(1000);
    connect(&m_reloadTimer, SIGNAL(timeout()), this, SLOT(reload()));
    m_reloadTimer.start();
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
                                        {TimeLog::SlackingRole, "slacking"}
                                    } );
}

Qt::ItemFlags TimeLog::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled; // | Qt::ItemIsEditable; // | Qt::ItemIsSelectable
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
    if (index.isValid() && index.row() >= 0 && index.row() < m_timelog.size()) {
        QString line = m_timelog.at(index.row());
        QString prevLine;
        if (index.row() > 0)
            prevLine = m_timelog.at(index.row()-1);
        QDateTime time = getTime(line);
        QDateTime prevTime = getTime(prevLine);
        switch (role) {
        case StartTimeRole:
            if (prevTime.isValid() && prevTime.date() != time.date())
                prevTime = time;
            data = prevTime;
            break;
        case EndTimeRole:
            data = getTime(line);
            break;
        case DurationRole:
            if (prevTime.isValid() && prevTime.date() == time.date())
                data = prevTime.secsTo(getTime(line)) / 60;
            else {
                data = 0;
            }
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
        }
    }

    return data;
}

bool TimeLog::setData(const QModelIndex &index, const QVariant &value, int role) {
    return false;
}

bool TimeLog::removeRows(int row, int count, const QModelIndex &parent) {
    return false;
}

bool TimeLog::addData(const QString &input) {
    QString line;
    QFile timelogFile(m_timelogFile);
    if (!timelogFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (isValid(input)) {
            beginInsertRows(QModelIndex(), m_timelog.size(), m_timelog.size());
            line.prepend(QString(QDateTime::currentDateTime().toString(DefaultDateTimeFormat) + ": "));
            m_timelog.append(line);
            endInsertRows();
        }
        timelogFile.write(line.append('\n').toUtf8());
        timelogFile.close();
        return true;
    }
    qDebug() << "Could not open" << m_timelogFile;
    return false;
}

void TimeLog::setTimelogFile(const QString &fileName) {
    QFileInfo newFile(fileName);
    if (newFile.isFile())
        newFile.setFile(fileName);
    else if (newFile.isDir())
        newFile.setFile(QDir(fileName), DefaultTimelogFilename);

    if (newFile.canonicalFilePath() != QFileInfo(m_timelogFile).canonicalFilePath()) {
        m_reloadTimer.stop();
        m_timelogFile = newFile.filePath();
        qDebug() << QFileInfo(m_timelogFile).canonicalFilePath();
        m_reloadTimer.start();
    }
}

void TimeLog::setTasksFile(const QString &fileName) {
    QFileInfo newFile(fileName);
    if (newFile.isFile())
        newFile.setFile(fileName);
    else if (newFile.isDir())
        newFile.setFile(QDir(fileName), DefaultTasksFilename);

    if (newFile.canonicalFilePath() != QFileInfo(m_tasksFile).canonicalFilePath()) {
        m_tasksFile = newFile.filePath();
        qDebug() << QFileInfo(m_tasksFile).canonicalFilePath();
    }
}

const QDateTime TimeLog::lastTime() const {
    QDateTime time;
    if (!m_timelog.isEmpty())
        time = getTime(m_timelog.last());
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
    if (isValid(line)) {
        projectAndTask = line.section(':', DefaultDateTimeFormat.count(':') + 1);
        if (isSlacking(line)) {
            projectAndTask = projectAndTask.section(" **", 0);
        }
    }
    return projectAndTask;
}

const QString TimeLog::getProject(const QString &line) {
    QString project;
    if (isValid(line)) {
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
    if (isValid(line)) {
        task = getProjectAndTask(line);
        if (task.contains(": ")) {
            task = task.section(": ", 1);
        }
    }
    return task;
}

bool TimeLog::isSlacking(const QString &line) {
    return isValid(line) && line.endsWith(" **");
}

bool TimeLog::isValid(const QString &line) {
    return !line.isEmpty() && !getTime(line).isValid();
}

bool TimeLog::reload() {
    int row = 0;
    QFileInfo fileInfo(m_timelogFile);
    if (fileInfo.lastModified() > m_timelogFileModifiedTime) {
        QFile timelogFile(m_timelogFile);
        if (!timelogFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString line;
            while (!timelogFile.atEnd()) {
                line = timelogFile.readLine();
                if (row < m_timelog.size()) {
                    if (line != m_timelog.at(row)) {
                        m_timelog[row] = line;
                        emit dataChanged(index(row), index(row));
                    }
                }
                else {
                    beginInsertRows(QModelIndex(), row, row);
                    m_timelog.append(line);
                    endInsertRows();
                }
                row ++;
            }
            timelogFile.close();
            beginRemoveRows(QModelIndex(), row+1, m_timelog.size()-1);
            while (m_timelog.size() > row) {
                m_timelog.removeLast();
            }
            endRemoveRows();
        }
        return true;
    }
    qDebug() << "Could not open" << m_timelogFile;
    return false;
}
