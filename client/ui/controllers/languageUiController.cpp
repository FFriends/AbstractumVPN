#include "languageUiController.h"

LanguageUiController::LanguageUiController(SettingsController* settingsController,
                                           LanguageModel* languageModel,
                                           QObject *parent)
    : QObject(parent),
      m_settingsController(settingsController),
      m_languageModel(languageModel)
{
}

void LanguageUiController::onAppLanguageChanged(const QLocale &locale)
{
    emit updateTranslations(locale);
}

void LanguageUiController::changeLanguage(const LanguageSettings::AvailableLanguageEnum language)
{
    QLocale locale = languageEnumToLocale(language);
    m_settingsController->setAppLanguage(locale);
}

int LanguageUiController::getCurrentLanguageIndex() const
{
    auto locale = m_settingsController->getAppLanguage();
    switch (locale.language()) {
    case QLocale::English: return static_cast<int>(LanguageSettings::AvailableLanguageEnum::English); break;
    case QLocale::Russian: return static_cast<int>(LanguageSettings::AvailableLanguageEnum::Russian); break;
    default: return static_cast<int>(LanguageSettings::AvailableLanguageEnum::English); break;
    }
}

// Nine text types in Controls2/TextTypes add this to their line height, so the
// method stays even though every shipped language now answers 0. It existed for
// Burmese, whose glyphs need the extra room; should a language like that come
// back, this is where it is handled.
int LanguageUiController::getLineHeightAppend() const
{
    return 0;
}

QString LanguageUiController::getCurrentLanguageName() const
{
    int index = getCurrentLanguageIndex();
    return getLocalLanguageName(static_cast<LanguageSettings::AvailableLanguageEnum>(index));
}

LanguageSettings::AvailableLanguageEnum LanguageUiController::getSystemLanguageEnum() const
{
    QLocale locale = QLocale::system();
    switch (locale.language()) {
    case QLocale::Russian: return LanguageSettings::AvailableLanguageEnum::Russian;
    case QLocale::English: return LanguageSettings::AvailableLanguageEnum::English;
    default: return LanguageSettings::AvailableLanguageEnum::English;
    }
}

// The project has no website of its own, but it does have a README with a
// walkthrough for people setting up a VPN for the first time. The setup wizard
// asks for exactly one path - "starter-guide" - and that is what it gets;
// anything else lands on the repository front page, which renders the same
// README.
//
// The Russian README is a translation rather than a shorter version, so a
// Russian interface is sent to it. "blob/HEAD" follows the default branch, so
// this survives a branch rename.
//
// The fragments are the anchors GitHub derives from the walkthrough headings.
// Rename a heading and this link quietly lands at the top of the page instead -
// nothing reports it, so the headings and these two strings change together.
QString LanguageUiController::getCurrentSiteUrl(const QString &path) const
{
    const QString repository = QStringLiteral("https://github.com/FFriends/AbstractumVPN");

    if (path == QLatin1String("starter-guide")) {
        const bool isRussian = m_settingsController->getAppLanguage().language() == QLocale::Russian;
        return isRussian ? repository + QStringLiteral("/blob/HEAD/README_RU.md#по-шагам")
                         : repository + QStringLiteral("#step-by-step");
    }

    return repository;
}

// No documentation site exists, so these keep pointing at the Telegram channel,
// where the linked chat answers questions in Russian and English. One address
// for every language - there is nothing to select a locale by.
QString LanguageUiController::getCurrentDocsUrl(const QString &path) const
{
    Q_UNUSED(path)
    return QStringLiteral("https://t.me/AbstractumMind");
}

QString LanguageUiController::getLocalLanguageName(const LanguageSettings::AvailableLanguageEnum language) const
{
    QString strLanguage("");
    switch (language) {
    case LanguageSettings::AvailableLanguageEnum::English: strLanguage = "English"; break;
    case LanguageSettings::AvailableLanguageEnum::Russian: strLanguage = "Русский"; break;
    default: break;
    }

    return strLanguage;
}

QLocale LanguageUiController::languageEnumToLocale(const LanguageSettings::AvailableLanguageEnum language) const
{
    switch (language) {
    case LanguageSettings::AvailableLanguageEnum::English: return QLocale::English;
    case LanguageSettings::AvailableLanguageEnum::Russian: return QLocale::Russian;
    default: return QLocale::English;
    }
}

