#include "factory-reset-helper.h"

#include <QDir>
#include <QDebug>
#include <QThread>
#include <QFileInfo>
#include <QStandardPaths>
#include <QRegularExpression>

const QString PACKAGES_FILE = "/var/lib/factory-reset/installed-packages.txt";

FactoryResetHelper::FactoryResetHelper(QObject *parent)
    : QObject(parent)
    , m_shouldRemovePackages(false)
    , m_shouldRemoveUserData(false)
    , m_isRunning(false)
    , m_currentProgress(0)
    , m_process(new QProcess(this))
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &FactoryResetHelper::readProcessOutput);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &FactoryResetHelper::handleProcessFinished);
}

FactoryResetHelper::~FactoryResetHelper()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished();
    }
}

bool FactoryResetHelper::isSystemEligibleForReset() const
{
    return QFileInfo::exists(PACKAGES_FILE);
}

QStringList FactoryResetHelper::getOriginalPackages() const
{
    QStringList packages;
    QFile file(PACKAGES_FILE);
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line;
        
        // Pomiń nagłówek
        in.readLine();
        
        // Format pliku DNF: nazwa.arch [wersja] repo
        QRegularExpression packagePattern("^([^\\s]+)\\s+");
        
        while (!in.atEnd()) {
            line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                auto match = packagePattern.match(line);
                if (match.hasMatch()) {
                    packages.append(match.captured(1));
                }
            }
        }
        
        file.close();
    }
    
    return packages;
}

QStringList FactoryResetHelper::getCurrentPackages() const
{
    QStringList packages;
    
    QProcess process;
    process.start("dnf", QStringList() << "list" << "installed" << "--quiet");
    process.waitForFinished();
    
    if (process.exitCode() == 0) {
        QString output = process.readAllStandardOutput();
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        // Pomiń nagłówek
        bool headerSkipped = false;
        
        // Format pliku DNF: nazwa.arch [wersja] repo
        QRegularExpression packagePattern("^([^\\s]+)\\s+");
        
        for (const QString &line : lines) {
            if (!headerSkipped) {
                headerSkipped = true;
                continue;
            }
            
            auto match = packagePattern.match(line.trimmed());
            if (match.hasMatch()) {
                packages.append(match.captured(1));
            }
        }
    }
    
    return packages;
}

QStringList FactoryResetHelper::getUserAccounts() const
{
    QStringList users;
    
    QProcess process;
    process.start("getent", QStringList() << "passwd" << "1000-60000");
    process.waitForFinished();
    
    if (process.exitCode() == 0) {
        QString output = process.readAllStandardOutput();
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        for (const QString &line : lines) {
            QStringList parts = line.split(':', Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                users.append(parts.at(0));
            }
        }
    }
    
    return users;
}

void FactoryResetHelper::startReset(bool removePackages, bool removeUserData)
{
    if (m_isRunning) {
        return;
    }
    
    m_shouldRemovePackages = removePackages;
    m_shouldRemoveUserData = removeUserData;
    m_isRunning = true;
    m_currentProgress = 0;
    
    reportProgress(0, "Rozpoczęcie procesu przywracania systemu...");
    
    // Załaduj oryginalne pakiety
    m_originalPackages = getOriginalPackages();
    
    // Załaduj aktualnie zainstalowane pakiety
    m_currentPackages = getCurrentPackages();
    
    // Wykonaj operacje resetowania w oddzielnym wątku
    QThread *thread = QThread::create([this]() { performReset(); });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void FactoryResetHelper::cancelReset()
{
    if (m_isRunning) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
        }
        m_isRunning = false;
        emit resetFinished(false, "Operacja została anulowana przez użytkownika.");
    }
}

void FactoryResetHelper::performReset()
{
    bool success = true;
    QString errorMessage;
    
    try {
        if (m_shouldRemovePackages) {
            reportProgress(10, "Przywracanie oryginalnych pakietów...");
            
            if (!removeExtraPackages()) {
                throw QString("Nie udało się usunąć dodatkowych pakietów.");
            }
            
            reportProgress(50, "Reinstalacja brakujących pakietów...");
            
            if (!reinstallMissingPackages()) {
                throw QString("Nie udało się zainstalować brakujących pakietów.");
            }
        }
        
        if (m_shouldRemoveUserData) {
            reportProgress(75, "Usuwanie danych użytkowników...");
            
            if (!removeUserData()) {
                throw QString("Nie udało się usunąć danych użytkowników.");
            }
        }
        
        reportProgress(100, "Operacja zakończona pomyślnie.");
    } catch (const QString &error) {
        success = false;
        errorMessage = error;
        reportProgress(0, "Błąd: " + error);
    }
    
    m_isRunning = false;
    emit resetFinished(success, success ? "System został przywrócony do stanu fabrycznego." : errorMessage);
}

bool FactoryResetHelper::removeExtraPackages()
{
    // Znajdź pakiety, które zostały zainstalowane po pierwszej instalacji
    QStringList packagesToRemove;
    
    for (const QString &package : m_currentPackages) {
        if (!m_originalPackages.contains(package)) {
            packagesToRemove.append(package);
        }
    }
    
    if (packagesToRemove.isEmpty()) {
        // Nie ma pakietów do usunięcia
        return true;
    }
    
    // Użyj dnf do usunięcia pakietów
    QStringList args;
    args << "remove" << "-y" << packagesToRemove;
    
    m_process->start("dnf", args);
    return m_process->waitForFinished(3600000); // Maksymalnie 1 godzina na usunięcie pakietów
}

bool FactoryResetHelper::reinstallMissingPackages()
{
    // Znajdź brakujące pakiety, które były w oryginalnym systemie
    QStringList packagesToInstall;
    
    for (const QString &package : m_originalPackages) {
        if (!m_currentPackages.contains(package)) {
            packagesToInstall.append(package);
        }
    }
    
    if (packagesToInstall.isEmpty()) {
        // Nie ma pakietów do zainstalowania
        return true;
    }
    
    // Użyj dnf do instalacji pakietów
    QStringList args;
    args << "install" << "-y" << packagesToInstall;
    
    m_process->start("dnf", args);
    return m_process->waitForFinished(3600000); // Maksymalnie 1 godzina na instalację pakietów
}

bool FactoryResetHelper::removeUserData()
{
    QStringList users = getUserAccounts();
    
    if (users.isEmpty()) {
        return true;
    }
    
    // Usunięcie katalogów domowych użytkowników
    for (const QString &user : users) {
        // Znajdź katalog domowy użytkownika
        QProcess process;
        process.start("getent", QStringList() << "passwd" << user);
        process.waitForFinished();
        
        if (process.exitCode() == 0) {
            QString output = process.readAllStandardOutput();
            QStringList parts = output.split(':', Qt::SkipEmptyParts);
            
            if (parts.size() >= 6) {
                QString homeDir = parts.at(5);
                
                // Usuń katalog domowy
                QProcess removeHome;
                removeHome.start("rm", QStringList() << "-rf" << homeDir);
                removeHome.waitForFinished();
            }
        }
        
        // Usuń użytkownika
        QProcess removeUser;
        removeUser.start("userdel", QStringList() << "-r" << user);
        removeUser.waitForFinished();
    }
    
    return true;
}

void FactoryResetHelper::reportProgress(int progress, const QString &statusMessage)
{
    m_currentProgress = progress;
    emit progressUpdated(progress, statusMessage);
}

void FactoryResetHelper::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        qDebug() << "Proces zakończył się z błędem:" << exitCode;
        qDebug() << "Wyjście standardowe:" << m_process->readAllStandardOutput();
        qDebug() << "Wyjście błędów:" << m_process->readAllStandardError();
    }
}

void FactoryResetHelper::readProcessOutput()
{
    QString output = m_process->readAllStandardOutput();
    // Można przetwarzać wyjście, np. aktualizować status
    qDebug() << output;
}