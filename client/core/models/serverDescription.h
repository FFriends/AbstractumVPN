#ifndef SERVERDESCRIPTION_H
#define SERVERDESCRIPTION_H

#include <QString>
#include <QJsonObject>

#include "core/utils/containerEnum.h"
#include "core/utils/selfhosted/sshSession.h"
#include "core/models/selfhosted/selfHostedAdminServerConfig.h"
#include "core/models/selfhosted/selfHostedUserServerConfig.h"
#include "core/models/selfhosted/nativeServerConfig.h"

namespace amnezia
{

struct ServerDescription
{
    QString serverId;

    QString serverName;
    QString baseDescription;
    QString hostName;

    int configVersion = 0;

    ServerCredentials selfHostedSshCredentials;
    bool hasWriteAccess = false;

    bool primaryDnsIsAmnezia = false;
    DockerContainer defaultContainer = DockerContainer::None;
    bool hasInstalledVpnContainers = false;

    // The only survivor of the gateway/subscription era. A few QML pages still
    // ask it in order to hide settings that make no sense for a server the user
    // does not own; nothing sets it any more, so it is always false, and the
    // branches behind it are unreachable. Removing it means rewriting those
    // pages, which is a separate piece of work.
    bool isServerFromGatewayApi = false;

    QString collapsedServerDescription;
    QString expandedServerDescription;
};

ServerDescription buildServerDescription(const SelfHostedAdminServerConfig &server, bool isAmneziaDnsEnabled);
ServerDescription buildServerDescription(const SelfHostedUserServerConfig &server, bool isAmneziaDnsEnabled);
ServerDescription buildServerDescription(const NativeServerConfig &server, bool isAmneziaDnsEnabled);

// A server left over from a subscription service. Nothing can be parsed out of
// it any more, so the name is read straight from the stored JSON - without it
// the entry would show up in the list nameless.
ServerDescription buildUnsupportedSubscriptionDescription(const QJsonObject &storedJson);

} // namespace amnezia

#endif
