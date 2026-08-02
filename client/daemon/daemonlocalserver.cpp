/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "daemonlocalserver.h"

#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>

#include "daemonlocalserverconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "peerverifier.h"

#if defined(MZ_MACOS) || defined(MZ_LINUX)
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>

// Renamed away from "amneziavpn": this application installs alongside
// AmneziaVPN instead of over it, and both daemons run as root. Sharing the
// path meant whichever started first owned it, and the other product's client
// would then be talking to this daemon.
constexpr const char* SOCKET_DIR_NAME = "abstractumvpn";
constexpr const char* TMP_PATH = "/tmp/abstractumvpn.socket";
constexpr const char* VAR_PATH = "/var/run/abstractumvpn/daemon.socket";
constexpr const char* VAR_DIR = "/var/run/abstractumvpn";
#endif

namespace {
Logger logger("DaemonLocalServer");
}  // namespace

DaemonLocalServer::DaemonLocalServer(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(DaemonLocalServer);
}

DaemonLocalServer::~DaemonLocalServer() { MZ_COUNT_DTOR(DaemonLocalServer); }

bool DaemonLocalServer::initialize() {
  m_server.setSocketOptions(QLocalServer::WorldAccessOption);

  QString path = daemonPath();
  logger.debug() << "Server path:" << path;

  if (QFileInfo::exists(path)) {
    QFile::remove(path);
  }

  if (!m_server.listen(path)) {
    logger.error() << "Failed to listen the daemon path";
    return false;
  }

  connect(&m_server, &QLocalServer::newConnection, [&] {
    logger.debug() << "New connection received";

    if (!m_server.hasPendingConnections()) {
      return;
    }

    QLocalSocket* socket = m_server.nextPendingConnection();
    Q_ASSERT(socket);

    // Second privileged channel, same gate as the Qt Remote Objects one -
    // see ipc/peerverifier.h.
    if (!amnezia::PeerVerifier::isTrustedPeer(socket)) {
      socket->abort();
      socket->deleteLater();
      return;
    }

    DaemonLocalServerConnection* connection =
        new DaemonLocalServerConnection(&m_server, socket);
    connect(socket, &QLocalSocket::disconnected, connection,
            &DaemonLocalServerConnection::deleteLater);
  });

  return true;
}

QString DaemonLocalServer::daemonPath() const {
#if defined(MZ_WINDOWS)
  return "\\\\.\\pipe\\abstractumvpn";
#endif
#if defined(MZ_MACOS) || defined(MZ_LINUX)
  QDir dir("/var/run");
  if (!dir.exists()) {
    logger.warning() << "/var/run doesn't exist. Fallback /tmp.";
    return TMP_PATH;
  }

  if (dir.exists(SOCKET_DIR_NAME)) {
    logger.debug() << VAR_DIR << "seems to be usable";
    return VAR_PATH;
  }

  if (!dir.mkdir(SOCKET_DIR_NAME)) {
    logger.warning() << "Failed to create" << VAR_DIR;
    return TMP_PATH;
  }

  // Traversable by everyone, writable only by root. The socket inside carries
  // its own world access; the directory does not need to hand out the right
  // to create or replace entries in it.
  if (chmod(VAR_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0) {
    logger.warning() << "Failed to set the right permissions to" << VAR_DIR;
    return TMP_PATH;
  }

  return VAR_PATH;
#endif
}
