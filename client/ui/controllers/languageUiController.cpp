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

// The project has no website and no documentation site of its own, so both of
// these point at the Telegram channel, where the linked chat answers questions
// in Russian and English. There is nothing to select a locale by: it is one
// address for every language.
//
// "path" is kept only so the QML call sites need not change, and is ignored -
// there are no sections to address. Should a documentation site ever appear,
// this is the single place that has to learn about it.
QString LanguageUiController::getCurrentSiteUrl(const QString &path) const
{
    Q_UNUSED(path)
    return QStringLiteral("https://t.me/AbstractumMind");
}

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

