#include "updateUiController.h"

UpdateUiController::UpdateUiController(UpdateController* updateController, QObject *parent)
    : QObject(parent), m_updateController(updateController)
{
    if (m_updateController) {
        connect(m_updateController, &UpdateController::updateFound, this, [this]() {
            m_manualCheck = false;
            emit updateFound();
        });
        connect(m_updateController, &UpdateController::updateNotFound, this, [this]() {
            if (!m_manualCheck) {
                return;
            }
            m_manualCheck = false;
            emit updateNotFound();
        });
    }
}

QString UpdateUiController::getHeaderText() const
{
    if (!m_updateController) {
        return QString();
    }

    const QString version = m_updateController->getVersion();
    const QString releaseDate = m_updateController->getReleaseDate();
    if (releaseDate.trimmed().isEmpty()) {
        return tr("New version released: %1").arg(version);
    }

    return tr("New version released: %1 (%2)").arg(version, releaseDate);
}

QString UpdateUiController::getChangelogText() const
{
    if (!m_updateController) {
        return QString();
    }

    const QString rawChangelog = m_updateController->getRawChangelogText();
    if (rawChangelog.isEmpty()) {
        return tr("Failed to load changelog text");
    }

    // Shown as published. There used to be a filter here that kept only the
    // lines under "### General" and the section for the current OS - a shape
    // the upstream project maintained by hand. Our release notes are generated
    // from commit subjects and have no such headings, so the filter matched
    // nothing and the drawer came up empty.
    return rawChangelog;
}

QString UpdateUiController::getVersion() const
{
    return m_updateController ? m_updateController->getVersion() : QString();
}

void UpdateUiController::checkForUpdates()
{
    if (m_updateController) {
        m_manualCheck = true;
        m_updateController->checkForUpdates();
    }
}

void UpdateUiController::runInstaller()
{
    if (m_updateController) {
        m_updateController->runInstaller();
    }
}
