#pragma once

#include <KCModule>
#include <KPluginFactory>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>

#include "factory-reset-helper.h"

class FactoryResetKCM : public KCModule
{
    Q_OBJECT

public:
    explicit FactoryResetKCM(QWidget *parent, const QVariantList &args);
    ~FactoryResetKCM() override;

private slots:
    void updateResetButtonState();
    void confirmReset();
    void performReset();
    void updateProgress(int progress, const QString &statusMessage);
    void resetCompleted(bool success, const QString &message);

private:
    void setupUI();
    void loadSettings();
    void addSeparator();

    QVBoxLayout *m_mainLayout;
    QLabel *m_introLabel;
    QLabel *m_warningLabel;

    QCheckBox *m_removePackagesCheckbox;
    QCheckBox *m_removeUserDataCheckbox;
    QCheckBox *m_confirmationCheckbox;

    QPushButton *m_resetButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    FactoryResetHelper *m_resetHelper;
};