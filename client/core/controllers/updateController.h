#ifndef UPDATECONTROLLER_H
#define UPDATECONTROLLER_H

#include <QJsonArray>
#include <QObject>
#include <QNetworkReply>
#include <QUrl>

#include "core/repositories/secureAppSettingsRepository.h"

class UpdateController : public QObject
{
    Q_OBJECT
public:
    explicit UpdateController(SecureAppSettingsRepository* appSettingsRepository, QObject *parent = nullptr);

    QString getRawChangelogText() const;
    QString getReleaseDate() const;
    QString getVersion() const;

public slots:
    void checkForUpdates();
    void runInstaller();

signals:
    void updateFound();

    // Emitted when the check completed and the published release is not newer
    // than what is running. Deliberately not emitted on a network failure or on
    // a rate limit: "you are up to date" and "we could not find out" are
    // different answers, and only the first one is this signal.
    void updateNotFound();

private:
    void finishUpdateCheck();

    // Asks the GitHub Releases API of UPDATE_REPO_SLUG for the newest published
    // release. The request carries no payload and no identifier of this
    // installation - only the product name and version in the User-Agent, which
    // GitHub requires.
    void fetchLatestRelease();
    bool parseRelease(const QByteArray &json);

    // "v5.0.0.2631" -> "5.0.0.2631". Also copes with the older tag shape
    // "v5.0.0.5-2026w31", where everything from the dash on is not a version,
    // and with "v5.0.0.2632.3", where the trailing ".3" only disambiguates a
    // second release published in the same week.
    static QString normalizedVersion(const QString &tagName);

    // Picks the installer for this platform out of the release assets by file
    // extension. Never by file name: the product name has already changed once,
    // and a hardcoded name broke artifact upload back then.
    bool pickInstallerAsset(const QJsonArray &assets);

    // Strict kill switch blocks everything outside the tunnel, including this
    // check and the download that follows it.
    void allowHostThroughKillSwitch(const QUrl &url);

    bool isNewVersionAvailable() const;
    void setupNetworkErrorHandling(QNetworkReply* reply, const QString& operation);
    void handleNetworkError(QNetworkReply* reply, const QString& operation);

    SecureAppSettingsRepository* m_appSettingsRepository;

    QString m_changelogText;
    QString m_version;
    QString m_releaseDate;
    QString m_downloadUrl;
    qint64 m_downloadSize = 0;
    QString m_downloadDigest;
    bool m_updateCheckRunning = false;

#if defined(Q_OS_WINDOWS)
    int runWindowsInstaller(const QString &installerPath);
#elif defined(Q_OS_MACOS)
    int runMacInstaller(const QString &installerPath);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    int runLinuxInstaller(const QString &installerPath);
#endif
};

#endif // UPDATECONTROLLER_H
