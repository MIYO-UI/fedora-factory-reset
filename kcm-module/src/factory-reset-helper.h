#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QThread>

class FactoryResetHelper : public QObject
{
    Q_OBJECT

public:
    explicit FactoryResetHelper(QObject *parent = nullptr);
    ~FactoryResetHelper() override;

    bool isSystemEligibleForReset() const;
    QStringList getOriginalPackages() const;
    QStringList getCurrentPackages() const;
    QStringList getUserAccounts() const;

public slots:
    void startReset(bool removePackages, bool removeUserData);
    void cancelReset();

signals:
    void progressUpdated(int progress, const QString &statusMessage);
    void resetFinished(bool success, const QString &message);

private slots:
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void readProcessOutput();

private:
    void performReset();
    bool removeExtraPackages();
    bool reinstallMissingPackages();
    bool removeUserData();
    void reportProgress(int progress, const QString &statusMessage);

    bool m_shouldRemovePackages;
    bool m_shouldRemoveUserData;
    bool m_isRunning;
    int m_currentProgress;

    QProcess *m_process;
    QStringList m_originalPackages;
    QStringList m_currentPackages;
    QStringList m_packageDifference;
};