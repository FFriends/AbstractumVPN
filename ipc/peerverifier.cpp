// Both defines have to be set before anything else is included, so they come
// ahead of the Qt headers rather than next to the code that needs them.
//
// SO_PEERCRED and struct ucred are glibc extensions.
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

// GetNamedPipeClientProcessId, QueryFullProcessImageName and
// PROCESS_QUERY_LIMITED_INFORMATION all appeared in Windows Vista and are
// hidden by <windows.h> unless the target version says so.
#if defined(_WIN32) && !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0601
#endif

#include "peerverifier.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>

#include "version.h"

#ifdef Q_OS_WIN
    #include <windows.h>
#elif defined(Q_OS_LINUX)
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

namespace {

QString clientFileName()
{
#ifdef Q_OS_WIN
    return QStringLiteral(APPLICATION_NAME ".exe");
#else
    return QStringLiteral(APPLICATION_NAME);
#endif
}

// Both paths go through the same normalisation before they are compared:
// symlinks resolved, case folded where the filesystem ignores case. Without it
// /usr/local/bin/AbstractumVPN and /opt/AbstractumVPN/bin/AbstractumVPN look
// like different programs, and on Windows so do C:\Program Files\... and
// c:\program files\...
QString normalized(const QString &path)
{
    if (path.isEmpty()) {
        return {};
    }

    const QString canonical = QFileInfo(path).canonicalFilePath();
    const QString result = canonical.isEmpty() ? QDir::cleanPath(path) : canonical;

#ifdef Q_OS_WIN
    return result.toLower();
#else
    return result;
#endif
}

} // namespace

namespace amnezia {

QString PeerVerifier::peerExecutablePath(QLocalSocket *socket)
{
    if (!socket) {
        return {};
    }

    const qintptr descriptor = socket->socketDescriptor();
    if (descriptor == -1) {
        return {};
    }

#ifdef Q_OS_WIN
    // We hold the server end of the pipe, so the OS can name the client
    // process for us; the process then names its own image.
    ULONG peerPid = 0;
    if (!GetNamedPipeClientProcessId(reinterpret_cast<HANDLE>(descriptor), &peerPid)) {
        qWarning() << "PeerVerifier: GetNamedPipeClientProcessId failed, error" << GetLastError();
        return {};
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, peerPid);
    if (!process) {
        qWarning() << "PeerVerifier: OpenProcess failed for pid" << peerPid << "error" << GetLastError();
        return {};
    }

    const DWORD bufferLength = 4096;
    wchar_t buffer[bufferLength] = {};
    DWORD size = bufferLength;
    const bool queried = QueryFullProcessImageNameW(process, 0, buffer, &size);
    CloseHandle(process);

    if (!queried) {
        qWarning() << "PeerVerifier: QueryFullProcessImageName failed for pid" << peerPid;
        return {};
    }

    return QString::fromWCharArray(buffer, static_cast<int>(size));
#elif defined(Q_OS_LINUX)
    // The kernel fills SO_PEERCRED at connect() time, so these credentials
    // belong to the process that actually opened the connection and cannot be
    // set by it.
    struct ucred credentials = {};
    socklen_t length = sizeof(credentials);
    if (getsockopt(static_cast<int>(descriptor), SOL_SOCKET, SO_PEERCRED, &credentials, &length) != 0) {
        qWarning() << "PeerVerifier: SO_PEERCRED failed";
        return {};
    }

    // Resolving /proc happens after the fact, so a recycled pid is a
    // theoretical hole here. Closing it properly means holding a pidfd, which
    // is a Linux 5.3+ interface - noted, not done.
    return QFileInfo(QStringLiteral("/proc/%1/exe").arg(credentials.pid)).canonicalFilePath();
#else
    // No implementation for this platform yet. Refusing is both the safe
    // answer and a loud one: whoever ports the service here trips over it
    // immediately instead of shipping an open channel.
    return {};
#endif
}

QString PeerVerifier::expectedClientPath()
{
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(clientFileName());
}

bool PeerVerifier::isTrustedPeer(QLocalSocket *socket)
{
    const QString peer = normalized(peerExecutablePath(socket));

    if (peer.isEmpty()) {
        qWarning() << "PeerVerifier: refused a connection, the peer process could not be identified";
        return false;
    }

    const QString expected = normalized(expectedClientPath());
    if (peer != expected) {
        qWarning() << "PeerVerifier: refused" << peer << "- this service only serves" << expected;
        return false;
    }

    return true;
}

} // namespace amnezia
