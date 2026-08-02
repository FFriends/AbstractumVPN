#ifndef SERVERCONFIGUTILS_H
#define SERVERCONFIGUTILS_H

#include <QJsonObject>

namespace serverConfigUtils
{

enum ConfigType {
    // A server added from a subscription service this application does not work
    // with. Such entries used to come in five flavours; the application no longer
    // tells them apart, because it cannot use any of them.
    //
    // They are deliberately NOT treated as invalid: an invalid entry is dropped on
    // load and then erased from Servers/serversList on the next write, which would
    // destroy someone's stored servers without asking. Instead the server stays in
    // the list under its own name and refuses to connect with an explanation.
    UnsupportedSubscription = 0,

    SelfHostedAdmin = 8,
    SelfHostedUser,
    Native,
    Invalid
};

enum ConfigSource {
    Telegram = 1,
    AmneziaGateway
};

// True for configs that came from a subscription service rather than from a
// server of one's own. Recognised by config_version alone.
bool isServerFromApi(const QJsonObject &serverConfigObject);

ConfigSource getConfigSource(const QJsonObject &serverConfigObject);

ConfigType configTypeFromJson(const QJsonObject &serverConfigObject);

bool isUnsupportedSubscription(ConfigType configType);

} // namespace serverConfigUtils

#endif // SERVERCONFIGUTILS_H
