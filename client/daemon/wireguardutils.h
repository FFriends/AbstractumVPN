/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILS_H
#define WIREGUARDUTILS_H

#define _WINSOCKAPI_

#include <QCoreApplication>
#include <QHostAddress>
#include <QObject>
#include <QStringList>

#include "interfaceconfig.h"

// Name of the local tunnel interface on Linux. Must differ from AmneziaVPN's
// "amn0", or two clients installed on one machine create and tear down the same
// interface behind each other's back.
//
// Careful: this constant is duplicated in
// platforms/linux/daemon/linuxroutemonitor.cpp and spelled out as a literal in
// linuxfirewall.cpp and wireguardutilslinux.cpp. Both have internal linkage, so
// changing one and missing the others compiles cleanly and breaks the tunnel at
// runtime. Grep for the value, not for the name.
//
// Unrelated to the "amn0" in server_scripts/prepare_host.sh - that is the
// Docker bridge on the server and part of the contract with Amnezia Client.
constexpr const char* WG_INTERFACE = "abs0";

class WireguardUtils : public QObject {
  Q_OBJECT

 public:
  class PeerStatus {
   public:
    PeerStatus(const QString& pubkey = QString()) { m_pubkey = pubkey; }
    QString m_pubkey;
    qint64 m_handshake = 0;
    qint64 m_rxBytes = 0;
    qint64 m_txBytes = 0;
  };

  explicit WireguardUtils(QObject* parent) : QObject(parent){};
  virtual ~WireguardUtils() = default;

  virtual bool interfaceExists() = 0;
  virtual QString interfaceName() { return WG_INTERFACE; }
  virtual bool addInterface(const InterfaceConfig& config) = 0;
  virtual bool deleteInterface() = 0;

  virtual bool updatePeer(const InterfaceConfig& config) = 0;
  virtual bool deletePeer(const InterfaceConfig& config) = 0;
  virtual QList<PeerStatus> getPeerStatus() = 0;

  virtual bool updateRoutePrefix(const IPAddress& prefix) = 0;
  virtual bool deleteRoutePrefix(const IPAddress& prefix) = 0;
  
  virtual bool addExclusionRoute(const IPAddress& prefix) = 0;
  virtual bool deleteExclusionRoute(const IPAddress& prefix) = 0;

  virtual bool excludeLocalNetworks(const QList<IPAddress>& addresses) = 0;
};

#endif  // WIREGUARDUTILS_H
