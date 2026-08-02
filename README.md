<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="client/images/AbstractumVPN_Full_logo.svg">
  <img src="client/images/AbstractumVPN_Full_logo_light.svg" alt="AbstractumVPN" width="320">
</picture>

<br><br>

**A VPN client that builds the server for you.**

You give it a machine you own. It installs the VPN there over SSH, then connects to it.
No accounts, no subscriptions, no operator in the middle.

[![Build](https://github.com/FFriends/AbstractumVPN/actions/workflows/abstractum-ci.yml/badge.svg)](https://github.com/FFriends/AbstractumVPN/actions/workflows/abstractum-ci.yml)
[![Release](https://img.shields.io/github/v/release/FFriends/AbstractumVPN?include_prereleases&label=release)](https://github.com/FFriends/AbstractumVPN/releases)
[![Licence](https://img.shields.io/badge/licence-GPL--3.0-blue)](LICENSE)

[Download](https://github.com/FFriends/AbstractumVPN/releases/latest) ·
[How it works](#how-it-works) ·
[Build from source](#building)

</div>

---

## Why this exists

A commercial VPN asks you to trust a company with the traffic you are trying to protect.
Self-hosting removes that company, but usually replaces it with an afternoon of SSH,
Docker and config files.

AbstractumVPN does the afternoon for you and then gets out of the way. The server is yours.
The keys are yours. Nothing phones home.

## How it works

The part that makes this different from every other client: **it provisions the server.**

```
  you                    your server                    the internet
   │                          │                              │
   │  1. SSH in, install      │                              │
   ├─────────────────────────►│                              │
   │     Docker + container   │                              │
   │                          │                              │
   │  2. connect to it        │                              │
   ├═════════════════════════►│─────────────────────────────►│
   │     encrypted tunnel     │                              │
```

Enter an address, a login and a password or key. The client SSHes in, installs Docker if
it is missing, builds the container for the protocol you picked, and connects. Removing it
later is one button.

Any Linux box with root over SSH works: a €4 VPS, a home server, a Raspberry Pi.

## Protocols

| | Good for |
|---|---|
| **AmneziaWG** | The default. WireGuard speed, with obfuscation so the traffic does not look like WireGuard |
| **WireGuard** | Fast and simple, when nothing is inspecting your traffic |
| **OpenVPN** | Mature and widely understood. Slower |
| **OpenVPN over Cloak** | Makes the connection resemble ordinary HTTPS |
| **OpenVPN over Shadowsocks** | Another masking layer, useful where Cloak is blocked |
| **XRay** | Deep packet inspection evasion |
| **IKEv2** | Built into Windows, no extra driver |

`AmneziaWG` and `AmneziaDNS` keep their names on purpose: they are components you install on
your own server, not our branding. Renaming them would leave you unable to recognise what you
actually deployed.

## Install

Grab a build from [Releases](https://github.com/FFriends/AbstractumVPN/releases/latest).

| | |
|---|---|
| **Windows** | `AbstractumVPN_*_windows_x64.exe` |
| **Linux** | `AbstractumVPN_*_linux_x64.run` |

Builds are not code-signed yet, so Windows SmartScreen will warn about an unknown publisher.
macOS, Android and iOS are not built at the moment: the code supports them, the signing
certificates do not exist.

## Building

**There is no local build.** Everything compiles in GitHub Actions, and that is deliberate:
Qt 6.10 plus nineteen Conan dependencies built from source is not a toolchain worth
reproducing on every machine.

Push, then watch [Actions](https://github.com/FFriends/AbstractumVPN/actions). A daily job at
03:00 UTC builds anything new; the weekly job publishes a release when there is something to
release.

If you do want a local toolchain anyway: CMake 3.25+, Conan 2.x, Qt 6.10+ **with Qt Remote
Objects** (not in the default Qt install; without it the privileged service will not build).
Then `deploy/build.sh` or `deploy\build.bat`.

## Security

Report vulnerabilities through
[GitHub Security Advisories](https://github.com/FFriends/AbstractumVPN/security/advisories/new),
which keeps the discussion private until a fix ships. Please do not open a public issue for
anything exploitable.

Two things worth knowing before you rely on this:

- **On Linux the stored server list is not encrypted.** Upstream disabled it because the
  keychain backend is unreliable there. Your server addresses and keys sit in plain
  `QSettings`.
- Arguments passed to the privileged helper are validated for one of the four processes it
  can launch. The other three inherit an upstream `FIXME`.

Neither is a secret and neither is fixed yet.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Short version: commits follow `type: summary`, work
happens on `dev`, and the build only runs in CI.

## Licence and origin

GPL-3.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).

AbstractumVPN is derived from
[amnezia-vpn/amnezia-client](https://github.com/amnezia-vpn/amnezia-client), forked at
`e38a2339`. Most of this codebase is their work, and the licence is inherited rather than
chosen: GPLv3 requires it.

This is an independent project. It is not affiliated with or endorsed by Amnezia. **Report
problems with AbstractumVPN here, not to them.** What differs so far: no subscriptions, no
paid hosting, no advertising, and no calls to anyone else's infrastructure.

Third-party components and their licences are listed in
[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
