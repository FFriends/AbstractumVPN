#include "updateController.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>
#include <QVersionNumber>

#include "amneziaApplication.h"
#include "logger.h"
#include "version.h"
#include "core/utils/networkUtilities.h"

#ifdef AMNEZIA_DESKTOP
    #include "core/utils/ipcClient.h"
#endif

namespace
{
    Logger logger("UpdateController");

    constexpr int kRequestTimeoutMsecs = 7000;

    // Installers are told apart by extension, not by name. CPack builds the file
    // name out of the target name, so it already changed once with the rebrand
    // and a hardcoded mask stopped matching without anyone noticing.
#if defined(Q_OS_WINDOWS)
    const QLatin1String kInstallerSuffix(".exe");
    const QString kInstallerLocalPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" APPLICATION_NAME "_installer.exe";
#elif defined(Q_OS_MACOS) && !defined(MACOS_NE)
    const QLatin1String kInstallerSuffix(".pkg");
    const QString kInstallerLocalPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" APPLICATION_NAME ".pkg";
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    const QLatin1String kInstallerSuffix(".run");
    const QString kInstallerLocalPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" APPLICATION_NAME ".run";
#endif
}

UpdateController::UpdateController(SecureAppSettingsRepository* appSettingsRepository, QObject *parent)
    : QObject(parent), m_appSettingsRepository(appSettingsRepository)
{
}

QString UpdateController::getRawChangelogText() const
{
    return m_changelogText;
}

QString UpdateController::getReleaseDate() const
{
    return m_releaseDate;
}

QString UpdateController::getVersion() const
{
    return m_version;
}

void UpdateController::checkForUpdates()
{
    if (m_updateCheckRunning || !m_appSettingsRepository) {
        return;
    }
    m_updateCheckRunning = true;

    fetchLatestRelease();
}

void UpdateController::finishUpdateCheck()
{
    m_updateCheckRunning = false;
}

void UpdateController::allowHostThroughKillSwitch(const QUrl &url)
{
#ifdef AMNEZIA_DESKTOP
    if (!m_appSettingsRepository || !m_appSettingsRepository->isStrictKillSwitchEnabled()) {
        return;
    }

    const QString ip = NetworkUtilities::getIPAddress(url.host());
    if (ip.isEmpty()) {
        return;
    }

    IpcClient::withInterface([&ip](QSharedPointer<IpcInterfaceReplica> iface) {
        QRemoteObjectPendingReply<bool> reply = iface->addKillSwitchAllowedRange(QStringList { ip });
        if (!reply.waitForFinished(1000) || !reply.returnValue()) {
            logger.warning() << "Failed to allow" << ip << "through the kill switch";
        }
    });
#else
    Q_UNUSED(url)
#endif
}

void UpdateController::fetchLatestRelease()
{
    // The list, not /releases/latest: our releases are published as
    // pre-releases, and /latest skips those and answers 404.
    const QUrl url(QStringLiteral("https://api.github.com/repos/%1/releases?per_page=10")
                           .arg(QLatin1String(UPDATE_REPO_SLUG)));

    allowHostThroughKillSwitch(url);

    QNetworkRequest request;
    request.setTransferTimeout(kRequestTimeoutMsecs);
    request.setUrl(url);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    // GitHub refuses requests without a User-Agent. Product name and version
    // only - this request must not say anything about who is asking.
    request.setRawHeader("User-Agent", QByteArray(APPLICATION_NAME "/" APP_VERSION));

    QNetworkReply *reply = amnApp->networkManager()->get(request);
    setupNetworkErrorHandling(reply, QStringLiteral("releases"));

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool rateLimited = (status == 403 || status == 429) && reply->rawHeader("X-RateLimit-Remaining") == "0";
        const QNetworkReply::NetworkError error = reply->error();
        const QByteArray body = reply->readAll();

        if (error != QNetworkReply::NoError && !rateLimited) {
            handleNetworkError(reply, QStringLiteral("releases"));
        }
        reply->deleteLater();

        if (rateLimited) {
            // Anonymous requests share an hourly budget per address, so several
            // clients behind one address can exhaust it. Not a failure worth
            // showing anyone - the next check will go through.
            logger.info() << "GitHub rate limit reached, skipping this update check";
            finishUpdateCheck();
            return;
        }

        if (error != QNetworkReply::NoError || !parseRelease(body)) {
            finishUpdateCheck();
            return;
        }

        if (!isNewVersionAvailable()) {
            // Split out of the failure branch above on purpose: this is the one
            // case where we actually know the answer, and the manual check in
            // the settings has to be able to say so.
            emit updateNotFound();
            finishUpdateCheck();
            return;
        }

        emit updateFound();
        finishUpdateCheck();
    });
}

QString UpdateController::normalizedVersion(const QString &tagName)
{
    QString version = tagName;
    if (version.startsWith(QLatin1Char('v'))) {
        version.remove(0, 1);
    }

    // Tags published before the version scheme changed look like
    // "5.0.0.5-2026w31"; everything from the dash on is not part of a version.
    const qsizetype dash = version.indexOf(QLatin1Char('-'));
    if (dash != -1) {
        version.truncate(dash);
    }

    // A second release within the same ISO week keeps the version it was built
    // with and gets a ".2", ".3" ... suffix on the tag alone - four components
    // is all the Windows FILEVERSION resource accepts, so the version itself
    // cannot grow. That suffix is not a version component and must not be read
    // as one: "5.0.0.2632.3" parses as five components and compares greater
    // than the installed "5.0.0.2632", so the client offers an update to the
    // build it is already running, and keeps offering it forever.
    QStringList components = version.split(QLatin1Char('.'));
    if (components.size() > 4) {
        components = components.mid(0, 4);
        version = components.join(QLatin1Char('.'));
    }

    return QVersionNumber::fromString(version).isNull() ? QString() : version;
}

bool UpdateController::pickInstallerAsset(const QJsonArray &assets)
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(MACOS_NE)
    Q_UNUSED(assets)
    return false;
#else
    for (const QJsonValue &value : assets) {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        if (!name.endsWith(kInstallerSuffix, Qt::CaseInsensitive)) {
            continue;
        }

        m_downloadUrl = asset.value(QStringLiteral("browser_download_url")).toString();
        m_downloadSize = static_cast<qint64>(asset.value(QStringLiteral("size")).toDouble());
        // GitHub started returning this as "sha256:<hex>"; absent on older releases.
        m_downloadDigest = asset.value(QStringLiteral("digest")).toString();

        return !m_downloadUrl.isEmpty();
    }

    return false;
#endif
}

bool UpdateController::parseRelease(const QByteArray &json)
{
    const QJsonArray releases = QJsonDocument::fromJson(json).array();

    for (const QJsonValue &value : releases) {
        const QJsonObject release = value.toObject();
        if (release.value(QStringLiteral("draft")).toBool()) {
            continue;
        }

        const QString version = normalizedVersion(release.value(QStringLiteral("tag_name")).toString());
        if (version.isEmpty()) {
            continue;
        }

        // A release without an installer for this platform is not an update for
        // us - keep looking at older ones.
        if (!pickInstallerAsset(release.value(QStringLiteral("assets")).toArray())) {
            continue;
        }

        m_version = version;
        m_changelogText = release.value(QStringLiteral("body")).toString();

        const QDateTime published =
                QDateTime::fromString(release.value(QStringLiteral("published_at")).toString(), Qt::ISODate);
        m_releaseDate = published.isValid() ? published.toLocalTime().date().toString(Qt::ISODate) : QString();

        return true;
    }

    logger.info() << "No published release with an installer for this platform";
    return false;
}

bool UpdateController::isNewVersionAvailable() const
{
    auto currentVersion = QVersionNumber::fromString(QString(APP_VERSION));
    auto newVersion = QVersionNumber::fromString(m_version);
    return newVersion > currentVersion;
}

void UpdateController::setupNetworkErrorHandling(QNetworkReply* reply, const QString& operation)
{
    QObject::connect(reply, &QNetworkReply::errorOccurred, [reply, operation](QNetworkReply::NetworkError error) {
        logger.error() << QString("Network error occurred while fetching %1: %2 %3")
                          .arg(operation, reply->errorString(), QString::number(error));
    });

    QObject::connect(reply, &QNetworkReply::sslErrors, [operation](const QList<QSslError> &errors) {
        QStringList errorStrings;
        for (const QSslError &err : errors) {
            errorStrings << err.errorString();
        }
        logger.error() << QString("SSL errors while fetching %1: %2").arg(operation, errorStrings.join("; "));
    });
}

void UpdateController::handleNetworkError(QNetworkReply* reply, const QString& operation)
{
    logger.error() << "Network error code:" << QString::number(static_cast<int>(reply->error()));
    logger.error() << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

void UpdateController::runInstaller()
{
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS) && !defined(MACOS_NE)
    if (m_downloadUrl.isEmpty()) {
        logger.error() << "Download URL is empty";
        return;
    }

    const QUrl downloadUrl(m_downloadUrl);

    // The download lands on a different host than the API call: GitHub answers
    // browser_download_url with a redirect to its object storage.
    allowHostThroughKillSwitch(downloadUrl);

    QNetworkRequest request;
    request.setTransferTimeout(30000);
    request.setUrl(downloadUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = amnApp->networkManager()->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray payload = reply->readAll();

            // Artifacts are unsigned so far, so this is all the integrity we
            // have. Refuse rather than run something that does not match what
            // the release says it published.
            if (m_downloadSize > 0 && payload.size() != m_downloadSize) {
                logger.error() << "Installer size mismatch: expected" << m_downloadSize << "got" << payload.size();
                reply->deleteLater();
                return;
            }

            if (m_downloadDigest.startsWith(QLatin1String("sha256:"))) {
                const QString actual = QString::fromLatin1(
                        QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex());
                if (actual != QStringView(m_downloadDigest).mid(7)) {
                    logger.error() << "Installer digest mismatch, refusing to run it";
                    reply->deleteLater();
                    return;
                }
            }

            QFile file(kInstallerLocalPath);
            if (!file.open(QIODevice::WriteOnly)) {
                logger.error() << "Failed to open installer file for writing:" << kInstallerLocalPath << "Error:" << file.errorString();
                reply->deleteLater();
                return;
            }

            if (file.write(payload) == -1) {
                logger.error() << "Failed to write installer data to file:" << kInstallerLocalPath << "Error:" << file.errorString();
                file.close();
                reply->deleteLater();
                return;
            }

            file.close();

    #if defined(Q_OS_WINDOWS)
            runWindowsInstaller(kInstallerLocalPath);
    #elif defined(Q_OS_MACOS) && !defined(MACOS_NE)
            runMacInstaller(kInstallerLocalPath);
    #elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
            runLinuxInstaller(kInstallerLocalPath);
    #endif
        } else {
            logger.error() << "Installer download failed, network error:" << static_cast<int>(reply->error())
                           << reply->errorString();
            logger.error() << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        reply->deleteLater();
    });
#endif
}

#if defined(Q_OS_WINDOWS)
int UpdateController::runWindowsInstaller(const QString &installerPath)
{
    qint64 pid;
    bool success = QProcess::startDetached(installerPath, QStringList(), QString(), &pid);

    if (success) {
        logger.info() << "Installation process started with PID:" << pid;
    } else {
        logger.error() << "Failed to start installation process";
        return -1;
    }

    return 0;
}
#endif

#if defined(Q_OS_MACOS) && !defined(MACOS_NE)
int UpdateController::runMacInstaller(const QString &installerPath)
{
    // Create temporary directory for extraction
    QTemporaryDir extractDir;
    extractDir.setAutoRemove(false);
    if (!extractDir.isValid()) {
        logger.error() << "Failed to create temporary directory";
        return -1;
    }
    logger.info() << "Temporary directory created:" << extractDir.path();

    // Create script file in the temporary directory
    QString scriptPath = extractDir.path() + "/mac_installer.sh";
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::WriteOnly)) {
        logger.error() << "Failed to create script file";
        return -1;
    }

    // Get script content from registry
    QString scriptContent = amnezia::scriptData(amnezia::ClientScriptType::mac_installer);
    if (scriptContent.isEmpty()) {
        logger.error() << "macOS installer script content is empty";
        scriptFile.close();
        return -1;
    }

    scriptFile.write(scriptContent.toUtf8());
    scriptFile.close();
    logger.info() << "Script file created:" << scriptPath;

    // Make script executable
    QFile::setPermissions(scriptPath, QFile::permissions(scriptPath) | QFile::ExeUser);

    // Start detached process
    qint64 pid;
    bool success =
            QProcess::startDetached("/bin/bash", QStringList() << scriptPath << extractDir.path() << installerPath, extractDir.path(), &pid);

    if (success) {
        logger.info() << "Installation process started with PID:" << pid;
    } else {
        logger.error() << "Failed to start installation process";
        return -1;
    }

    return 0;
}
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
int UpdateController::runLinuxInstaller(const QString &installerPath)
{
    QFile::setPermissions(installerPath, QFile::permissions(installerPath) | QFile::ExeUser);

    qint64 pid;
    bool success = QProcess::startDetached(installerPath, QStringList(), QString(), &pid);

    if (success) {
        logger.info() << "Installation process started with PID:" << pid;
    } else {
        logger.error() << "Failed to start installation process";
        return -1;
    }

    return 0;
}
#endif
