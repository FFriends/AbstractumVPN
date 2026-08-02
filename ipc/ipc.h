#ifndef IPC_H
#define IPC_H

#include <QDebug>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

#include "../client/core/utils/utilities.h"

// Name of the channel between the unprivileged client and the root service.
// Both sides must agree on it, so a client and a service from different brands
// cannot talk to each other - which is the point: this build does not drive an
// installed AmneziaVPN service, and that service does not answer this client.
#define IPC_SERVICE_URL "local:AbstractumVpnIpcInterface"

namespace amnezia {

enum PermittedProcess {
    Invalid,
    OpenVPN,
    Wireguard,
    Tun2Socks,
    CertUtil
};

inline QString permittedProcessPath(PermittedProcess pid)
{
    switch (pid) {
        case PermittedProcess::OpenVPN:
            return Utils::openVpnExecPath();
        case PermittedProcess::Wireguard:
            return Utils::wireguardExecPath();
        case PermittedProcess::CertUtil:
            return Utils::certUtilPath();
        case PermittedProcess::Tun2Socks:
            return Utils::tun2socksPath();
        default:
            return "";
    }
}


inline QString getIpcServiceUrl() {
#ifdef Q_OS_WIN
    return IPC_SERVICE_URL;
#else
    return QString("/tmp/%1").arg(IPC_SERVICE_URL);
#endif
}

inline QString getIpcProcessUrl(int pid) {
#ifdef Q_OS_WIN
    return QString("%1_%2").arg(IPC_SERVICE_URL).arg(pid);
#else
    return QString("/tmp/%1_%2").arg(IPC_SERVICE_URL).arg(pid);
#endif
}

// Whitelist of what the client is allowed to hand to a process the service
// runs as root/SYSTEM. Everything is described per option: how many values
// follow the key, and what those values are allowed to look like.
//
// Anything not described here is refused, and refusal drops the whole command
// line rather than the offending token. A root process started with a command
// line we only half understand is worse than one that fails to start: the
// second is visible immediately, the first is not.
//
// Values are never written to the log - one of them is the certificate
// password for IKEv2.
inline QStringList sanitizeArguments(PermittedProcess proc, const QStringList &args) {
    using Validator = std::function<bool(const QStringList&)>;

    struct Rule {
        int values = 0;
        Validator validator;
    };

    const Validator existingFile = [](const QStringList &v) {
        return v.size() == 1 && !v.first().isEmpty() && QFileInfo(v.first()).isFile();
    };
    const Validator nonEmpty = [](const QStringList &v) {
        return v.size() == 1 && !v.first().isEmpty();
    };
    // "--management <host> <port>": the client always points OpenVPN at its own
    // management server on loopback, so nothing else has any business here.
    const Validator loopbackAndPort = [](const QStringList &v) {
        if (v.size() != 2) {
            return false;
        }
        if (v.first() != QLatin1String("127.0.0.1") && v.first() != QLatin1String("::1")) {
            return false;
        }
        bool ok = false;
        const uint port = v.at(1).toUInt(&ok);
        return ok && port > 0 && port <= 65535;
    };

    QMap<QString, Rule> namedArgs;
    QList<Validator> positionalArgs;

    switch (proc) {
    case OpenVPN:
        namedArgs["--config"] = { 1, existingFile };
        namedArgs["--management"] = { 2, loopbackAndPort };
        namedArgs["--management-client"] = { 0, nullptr };
        break;
    case Tun2Socks:
        namedArgs["-device"] = { 1, [](const QStringList &v) { return v.first().startsWith("tun://"); } };
        namedArgs["-proxy"] = { 1, [](const QStringList &v) { return v.first().startsWith("socks5://"); } };
        break;
    case CertUtil:
        namedArgs["-f"] = { 0, nullptr };
        namedArgs["-importpfx"] = { 0, nullptr };
        namedArgs["-p"] = { 1, nonEmpty };
        positionalArgs << existingFile
                       << [](const QStringList &v) { return v.first() == QLatin1String("NoExport"); };
        break;
    case Wireguard:
        // Nothing is allowed through: no code path in this application starts
        // wireguard as a privileged process. The value stays in the enum so
        // the numbering keeps matching an already installed service.
        break;
    default:
        break;
    }

    QStringList sanitized;
    int positionalIndex = 0;

    for (int i = 0; i < args.size(); i++) {
        const QString &token = args.at(i);

        if (const auto found = namedArgs.constFind(token); found != namedArgs.constEnd()) {
            const QStringList values = args.mid(i + 1, found->values);
            if (static_cast<int>(values.size()) != found->values) {
                qWarning() << "sanitizeArguments: process" << static_cast<int>(proc) << "- option" << token << "is missing its values";
                return {};
            }
            if (found->validator && !found->validator(values)) {
                qWarning() << "sanitizeArguments: process" << static_cast<int>(proc) << "- rejected the value given for" << token;
                return {};
            }

            sanitized << token << values;
            i += found->values;
            continue;
        }

        if (positionalIndex < positionalArgs.size()) {
            const Validator &validator = positionalArgs.at(positionalIndex);
            if (validator && !validator({ token })) {
                qWarning() << "sanitizeArguments: process" << static_cast<int>(proc) << "- rejected positional argument"
                           << positionalIndex;
                return {};
            }

            sanitized << token;
            positionalIndex++;
            continue;
        }

        qWarning() << "sanitizeArguments: process" << static_cast<int>(proc) << "- unexpected argument, refusing the whole command line";
        return {};
    }

    return sanitized;
}

} // namespace amnezia

#endif // IPC_H
