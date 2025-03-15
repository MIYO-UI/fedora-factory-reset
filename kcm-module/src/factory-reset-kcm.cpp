#include "factory-reset-kcm.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QFrame>
#include <QFont>
#include <QTimer>

K_PLUGIN_FACTORY_WITH_JSON(FactoryResetKCMFactory, "factory-reset.json", registerPlugin<FactoryResetKCM>();)

FactoryResetKCM::FactoryResetKCM(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_resetHelper(new FactoryResetHelper(this))
{
    setButtons(Help | Apply);
    setupUI();
    loadSettings();

    // Połączenia sygnałów
    connect(m_removePackagesCheckbox, &QCheckBox::toggled, this, &FactoryResetKCM::updateResetButtonState);
    connect(m_removeUserDataCheckbox, &QCheckBox::toggled, this, &FactoryResetKCM::updateResetButtonState);
    connect(m_confirmationCheckbox, &QCheckBox::toggled, this, &FactoryResetKCM::updateResetButtonState);
    connect(m_resetButton, &QPushButton::clicked, this, &FactoryResetKCM::confirmReset);
    
    // Połączenia z helperem
    connect(m_resetHelper, &FactoryResetHelper::progressUpdated, this, &FactoryResetKCM::updateProgress);
    connect(m_resetHelper, &FactoryResetHelper::resetFinished, this, &FactoryResetKCM::resetCompleted);
    
    updateResetButtonState();
}

FactoryResetKCM::~FactoryResetKCM()
{
}

void FactoryResetKCM::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // Etykieta wprowadzająca
    m_introLabel = new QLabel(i18n("Ten moduł pozwala przywrócić system Fedora do stanu fabrycznego. "
                                  "Możesz wybrać, czy chcesz usunąć dodatkowe pakiety i/lub dane użytkowników."));
    m_introLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_introLabel);
    
    addSeparator();
    
    // Ostrzeżenie
    m_warningLabel = new QLabel(i18n("<b>UWAGA:</b> Przywrócenie systemu do stanu fabrycznego jest nieodwracalne. "
                                    "Zaleca się utworzenie kopii zapasowej ważnych danych przed kontynuowaniem."));
    m_warningLabel->setWordWrap(true);
    QFont warningFont = m_warningLabel->font();
    warningFont.setBold(true);
    m_warningLabel->setFont(warningFont);
    m_warningLabel->setStyleSheet("color: red;");
    m_mainLayout->addWidget(m_warningLabel);
    
    m_mainLayout->addSpacing(20);
    
    // Opcje przywracania
    m_removePackagesCheckbox = new QCheckBox(i18n("Usuń dodatkowe pakiety i przywróć oryginalne"));
    m_mainLayout->addWidget(m_removePackagesCheckbox);
    
    m_removeUserDataCheckbox = new QCheckBox(i18n("Usuń wszystkie dane użytkowników"));
    m_mainLayout->addWidget(m_removeUserDataCheckbox);
    
    m_mainLayout->addSpacing(20);
    
    // Potwierdzenie
    m_confirmationCheckbox = new QCheckBox(i18n("Rozumiem, że ta operacja jest nieodwracalna i chcę kontynuować"));
    m_mainLayout->addWidget(m_confirmationCheckbox);
    
    m_mainLayout->addSpacing(10);
    
    // Przycisk resetowania
    m_resetButton = new QPushButton(i18n("Przywróć system do stanu fabrycznego"));
    m_resetButton->setStyleSheet("background-color: #cc0000; color: white; font-weight: bold;");
    m_mainLayout->addWidget(m_resetButton);
    
    // Pasek postępu
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_mainLayout->addWidget(m_progressBar);
    
    // Etykieta statusu
    m_statusLabel = new QLabel();
    m_statusLabel->setVisible(false);
    m_mainLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addStretch();
    
    setLayout(m_mainLayout);
}

void FactoryResetKCM::loadSettings()
{
    // Sprawdź, czy system kwalifikuje się do resetu (czy mamy listę oryginalnych pakietów)
    if (!m_resetHelper->isSystemEligibleForReset()) {
        m_introLabel->setText(i18n("System nie może zostać przywrócony do stanu fabrycznego, ponieważ "
                                 "brakuje informacji o oryginalnej instalacji. Ten moduł działa tylko "
                                 "na systemach, które miały zainstalowany pakiet fedora-package-recorder "
                                 "podczas pierwszego uruchomienia."));
        
        m_removePackagesCheckbox->setEnabled(false);
        m_removeUserDataCheckbox->setEnabled(false);
        m_confirmationCheckbox->setEnabled(false);
        m_resetButton->setEnabled(false);
    }
}

void FactoryResetKCM::addSeparator()
{
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_mainLayout->addWidget(line);
}

void FactoryResetKCM::updateResetButtonState()
{
    bool canReset = m_confirmationCheckbox->isChecked() && 
                   (m_removePackagesCheckbox->isChecked() || m_removeUserDataCheckbox->isChecked());
    
    m_resetButton->setEnabled(canReset);
}

void FactoryResetKCM::confirmReset()
{
    int result = KMessageBox::warningContinueCancel(
        this,
        i18n("Zamierzasz przywrócić system do stanu fabrycznego. Ta operacja jest nieodwracalna.\n\n"
             "Czy na pewno chcesz kontynuować?"),
        i18n("Potwierdzenie przywracania systemu"),
        KStandardGuiItem::cont(),
        KStandardGuiItem::cancel(),
        QString(),
        KMessageBox::Dangerous
    );
    
    if (result == KMessageBox::Continue) {
        // Przygotuj interfejs
        m_removePackagesCheckbox->setEnabled(false);
        m_removeUserDataCheckbox->setEnabled(false);
        m_confirmationCheckbox->setEnabled(false);
        m_resetButton->setEnabled(false);
        m_progressBar->setVisible(true);
        m_statusLabel->setVisible(true);
        
        // Rozpocznij reset
        QTimer::singleShot(500, this, &FactoryResetKCM::performReset);
    }
}

void FactoryResetKCM::performReset()
{
    m_resetHelper->startReset(
        m_removePackagesCheckbox->isChecked(),
        m_removeUserDataCheckbox->isChecked()
    );
}

void FactoryResetKCM::updateProgress(int progress, const QString &statusMessage)
{
    m_progressBar->setValue(progress);
    m_statusLabel->setText(statusMessage);
}

void FactoryResetKCM::resetCompleted(bool success, const QString &message)
{
    if (success) {
        KMessageBox::information(
            this,
            i18n("Operacja zakończona pomyślnie: %1\n\nSystem zostanie teraz uruchomiony ponownie.", message),
            i18n("Operacja zakończona")
        );
        
        // Uruchom ponownie system
        QProcess::startDetached("systemctl", QStringList() << "reboot");
    } else {
        KMessageBox::error(
            this,
            i18n("Wystąpił błąd podczas przywracania systemu: %1", message),
            i18n("Błąd operacji")
        );
        
        // Przywróć kontrolki
        m_removePackagesCheckbox->setEnabled(true);
        m_removeUserDataCheckbox->setEnabled(true);
        m_confirmationCheckbox->setEnabled(true);
        updateResetButtonState();
    }
}

#include "factory-reset-kcm.moc"