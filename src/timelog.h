#ifndef TIMELOG_H
#define TIMELOG_H

#include <QAbstractListModel>
#include <QFile>
#include <QTimer>
#include <QDateTime>

const QString DefaultDateTimeFormat("yyyy-MM-dd HH:mm");
const QString DefaultTimelogFilename("timelog.txt");
const QString DefaultTasksFilename("tasks.txt");

class TimeLog : public QAbstractListModel {
    Q_OBJECT

public:
    explicit TimeLog(QObject *parent = nullptr);
    virtual ~TimeLog();

    enum LogRole {
        StartTimeRole = Qt::UserRole,
        EndTimeRole = Qt::UserRole + 1,
        DurationRole = Qt::UserRole + 2,
        ProjectRole = Qt::UserRole + 3,
        TaskRole = Qt::UserRole + 4,
        SlackingRole = Qt::UserRole + 5
    };
    Q_ENUM(LogRole)
    QHash<int, QByteArray> roleNames() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE bool addData(const QString &input);

    Q_PROPERTY(QString timelogFile READ timelogFile WRITE setTimelogFile NOTIFY timelogFileChanged)
    const QString timelogFile() const { return m_timelogFile; }
    void setTimelogFile(const QString &fileName);

    Q_PROPERTY(QString tasksFile READ tasksFile WRITE setTasksFile NOTIFY tasksFileChanged)
    const QString tasksFile() const { return m_tasksFile; }
    void setTasksFile(const QString &fileName);

    Q_PROPERTY(QDateTime lastTime READ lastTime NOTIFY lastTimeChanged)
    const QDateTime lastTime() const;

    Q_INVOKABLE const static QDateTime getTime(const QString &line);
    Q_INVOKABLE const static QString getProjectAndTask(const QString &line);
    Q_INVOKABLE const static QString getProject(const QString &line);
    Q_INVOKABLE const static QString getTask(const QString &line);
    Q_INVOKABLE static bool isSlacking(const QString &line);
    Q_INVOKABLE static bool isValid(const QString &line);

public slots:
    Q_INVOKABLE bool reload();

signals:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void timelogFileChanged(const QString &fileName);
    void tasksFileChanged(const QString &fileName);
    void lastTimeChanged(const QDateTime &time);

private:
    QString m_timelogFile;
    QDateTime m_timelogFileModifiedTime;
    QString m_tasksFile;
    QTimer m_reloadTimer;
    QStringList m_timelog;
};

#endif // TIMELOG_H
