#ifndef PEERVERIFIER_H
#define PEERVERIFIER_H

#include <QString>

class QLocalSocket;

namespace amnezia {

// Decides who is allowed to talk to the privileged service.
//
// The local sockets stay world-accessible on purpose. The service runs as
// root/SYSTEM while the client runs as the desktop user, so tightening the
// socket permissions would cut the two apart instead of protecting anything.
// The decision is taken here instead, once per connection: ask the operating
// system which executable sits on the other end and compare it with the client
// binary installed next to this service.
//
// Anything else is refused - an arbitrary local process, a copy of the client
// living somewhere else, or a platform where the peer cannot be identified at
// all.
namespace PeerVerifier {

// Absolute path of the executable behind the socket. Empty when the platform
// has no implementation here or the lookup failed.
QString peerExecutablePath(QLocalSocket *socket);

// Where the client is expected to live: next to the running service binary.
// Both installers put them in the same directory.
QString expectedClientPath();

// True only when the peer could be identified and is that exact binary.
bool isTrustedPeer(QLocalSocket *socket);

} // namespace PeerVerifier

} // namespace amnezia

#endif // PEERVERIFIER_H
