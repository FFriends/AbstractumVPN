#include "serverConfigUtils.h"

#include <QJsonArray>
#include <QJsonValue>

#include "core/models/selfhosted/selfHostedAdminServerConfig.h"
#include "core/utils/constants/configKeys.h"

namespace
{

bool hasThirdPartyConfig(const QJsonObject &json)
{
    const QJsonArray containersArray = json.value(amnezia::configKey::containers).toArray();
    for (const QJsonValue &val : containersArray) {
        const QJsonObject containerObj = val.toObject();
        for (auto it = containerObj.begin(); it != containerObj.end(); ++it) {
            if (it.key() == amnezia::configKey::container) {
                continue;
            }
            const QJsonObject protocolObj = it.value().toObject();
            if (protocolObj.contains(amnezia::configKey::isThirdPartyConfig)
                && protocolObj.value(amnezia::configKey::isThirdPartyConfig).toBool()) {
                return true;
            }
        }
    }
    return false;
}

} // namespace

namespace serverConfigUtils
{

bool isServerFromApi(const QJsonObject &serverConfigObject)
{
    const int configVersion = serverConfigObject.value(amnezia::configKey::configVersion).toInt();
    switch (configVersion) {
    case ConfigSource::Telegram:
    case ConfigSource::AmneziaGateway:
        return true;
    default:
        return false;
    }
}

ConfigSource getConfigSource(const QJsonObject &serverConfigObject)
{
    return static_cast<ConfigSource>(serverConfigObject.value(amnezia::configKey::configVersion).toInt());
}

ConfigType configTypeFromJson(const QJsonObject &serverConfigObject)
{
    // Anything carrying a subscription config_version is unusable here, and there
    // is no reason to look any deeper: the endpoint and the service type inside
    // only mattered while the application could talk to those services.
    if (isServerFromApi(serverConfigObject)) {
        return ConfigType::UnsupportedSubscription;
    }

    if (hasThirdPartyConfig(serverConfigObject)) {
        return ConfigType::Native;
    }

    const amnezia::SelfHostedAdminServerConfig adminProbe =
            amnezia::SelfHostedAdminServerConfig::fromJson(serverConfigObject);
    return adminProbe.hasCredentials() ? ConfigType::SelfHostedAdmin : ConfigType::SelfHostedUser;
}

bool isUnsupportedSubscription(ConfigType configType)
{
    return configType == ConfigType::UnsupportedSubscription;
}

} // namespace serverConfigUtils
