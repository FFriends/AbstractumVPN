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

[Русский](README_RU.md)

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

## What the server needs

Any Linux machine you control, reachable over SSH. Nothing is pre-configured by hand.

| | |
|---|---|
| **Distribution** | Anything with `apt-get`, `dnf`, `yum`, `zypper` or `pacman`, and systemd. Debian, Ubuntu, Fedora, CentOS, openSUSE and Arch all qualify |
| **Access** | SSH, as `root` or as a user in the `sudo` / `wheel` group |
| **Docker** | Installed for you if it is missing. Nothing to prepare |
| **Ports** | Opened for you through the host firewall. Defaults: OpenVPN `1194`, WireGuard `51820`, AmneziaWG `55424`, XRay and Cloak `443`, IKEv2 `500` and `4500` UDP |

Before installing anything the client looks at the package manager's lock file. If the machine
is already busy, it says so instead of starting a second install on top.

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

## Other things it can install

The same one-button flow puts these on your server as well:

| | |
|---|---|
| **AmneziaDNS** | An `unbound` resolver reachable only inside the tunnel, forwarding over DNS-over-TLS. Your DNS stops going to your provider |
| **SFTP storage** | A private file share on the server |
| **SOCKS5 proxy** | For applications that speak SOCKS but not VPN |
| **MTProxy** | Telegram proxy |
| **Website in Tor** | Publishes a site as an onion service |

There is no manual setup guide for these, because there is nothing to set up manually. Open
the server in Settings, go to its **Services** tab, pick one, and the client installs it over
the same SSH session. Removing it is the same button.

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
[GitHub Security Advisories](https://github.com/FFriends/AbstractumVPN/security/advisories/new).
It keeps the discussion private until a fix ships, and it stays open even while the issue
tracker is closed. Scope, timelines and what counts as ours are in [SECURITY.md](SECURITY.md).

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

The issue tracker is closed for now. Security reports still go through the advisories link
above; for anything else, open a pull request.

## Where this gets written about

[**@AbstractumMind**](https://t.me/AbstractumMind) on Telegram: what is being built, what broke
on the way, and whatever else seems worth writing down. Not a support desk.

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
