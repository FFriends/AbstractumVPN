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

[Download](https://github.com/FFriends/AbstractumVPN/releases) ·
[How it works](#how-it-works) ·
[Step by step](#step-by-step) ·
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

## Step by step

### First, the thing everyone trips over: you install nothing on the server

**You download nothing onto the server, run nothing there, and never log into it yourself.**
There is nothing to install by hand. The downloads on this page are a program for **your own
computer**, and not one byte of them ends up on the server.

Whatever the server needs, the program puts there itself: it connects over the network, runs
the commands for you, and disconnects. Your part is typing the server's address and password
into four fields.

There are two machines in this story, and they are easy to confuse:

| | What it is | What you do with it |
|---|---|---|
| **Your computer** | The Windows or Linux machine in front of you | Download and install AbstractumVPN from the releases page |
| **The server** | Someone else's computer in a data centre that you rent | **Nothing.** You only find out its address and password and type them into the program |

### Step 1. Rent a server

What you need is a **VPS** — a virtual private server, meaning you pay to rent a slice of a
machine in a data centre and get full control of it. Usually 3–5 dollars a month. There are
many providers; we recommend none of them, because we have no arrangement with anyone.

What to choose when ordering:

- **Operating system: Linux.** Ubuntu, Debian, Fedora, CentOS, openSUSE or Arch all work. If
  you have no opinion, take the latest **Ubuntu** — it is the ordinary choice.
- **Location: the country you want to appear to be in.** Closer to you means faster.
- **Size: the smallest one.** A VPN barely loads a server.
- **It must be an empty server**, not one already running something of yours.

After you pay, the provider sends three things. They are everything you need:

| What arrives | What it looks like | Where it goes later |
|---|---|---|
| The server's IP address | `203.0.113.17` | the "Server IP address" field |
| A user name | usually `root` | the login field |
| A password | a string of characters | the password field |

Some providers hand you an SSH key file instead of a password. The program takes that too.

### Step 2. Install the program on your own computer

Download a file from the [releases page](https://github.com/FFriends/AbstractumVPN/releases)
and run it, like any other program.

| Your computer | Which file |
|---|---|
| Windows | `AbstractumVPN_*_windows_x64.exe` |
| Linux | `AbstractumVPN_*_linux_x64.run` |

Windows will warn about an unknown publisher. It will keep doing that until the project pays
for a signing certificate.

### Step 3. Type in the server details

Open the program and press **"Let's get started"**. It asks where the connection comes from —
choose the option about **your own server**, not the one about a ready-made configuration.

Then four fields, all of it straight from the provider's email:

1. **Server IP address** — the address from the email, for example `203.0.113.17`. You do not
   need a port: unless the provider told you otherwise, the program uses the standard one. If
   they did, add it after a colon: `203.0.113.17:2222`.
2. **Login** — usually `root`.
3. **Password** — from the email. Or paste the whole key, including the `BEGIN` and `END` lines.
4. Press **"Continue"**.

The program connects and checks that the access works and that the server is not busy with
another installation. If something does not add up, it says what.

### Step 4. Choose a protocol

The **"VPN protocol"** screen appears. If you do not know which to take, take **AmneziaWG** —
it is first in the list and suits most people. Others can be added later; the server can hold
several.

### Step 5. Wait

From here the program does everything itself: logs in, installs Docker, builds the VPN, opens
the port and connects. Usually **three to ten minutes**, longest on a slow server. Progress is
on screen, with a cancel button next to it.

When the bar finishes, the VPN works. You can stop reading here.

### Step 6. Later: add a service, for example AmneziaDNS

Optional. If you also want your lookups of website names to stop going to your provider:

**Settings** → your server → the **"Services"** tab → **AmneziaDNS** → install.

A few minutes of waiting again, and again nothing to configure by hand. Everything else in the
table below is added the same way, one at a time.

The **"Protocols"** tab next to it adds a second protocol, and **"Management"** removes things —
one container, or everything the program ever put on that server.

### If something goes wrong

Ask in the chat attached to [@AbstractumMind](https://t.me/AbstractumMind), in English or
Russian. The link is in the program too, under "About".

## What the server needs

The precise version of [step 1](#step-1-rent-a-server), for readers who want the requirements
rather than the shopping advice. Any Linux machine you control, reachable over SSH. Nothing is
pre-configured by hand.

| | |
|---|---|
| **Distribution** | Anything with `apt-get`, `dnf`, `yum`, `zypper` or `pacman`, and systemd. Debian, Ubuntu, Fedora, CentOS, openSUSE and Arch all qualify |
| **Access** | SSH, as `root` or as a user in the `sudo` / `wheel` group |
| **Docker** | Installed for you if it is missing. Nothing to prepare |
| **Ports** | Opened for you through the host firewall. Defaults: OpenVPN `1194`, WireGuard `51820`, AmneziaWG `55424`, XRay and Cloak `443`, IKEv2 `500` and `4500` UDP |

Before installing anything the client looks at the package manager's lock file. If the machine
is already busy, the client waits for that install to finish rather than starting a second one
on top. It says what it is waiting for and offers a cancel button; if the lock is still held
after about five minutes, it stops with an error instead of waiting forever.

## Protocols

| | Good for |
|---|---|
| **AmneziaWG** | The default. WireGuard speed, with obfuscation so the traffic does not look like WireGuard |
| **WireGuard** | Fast and simple, when nothing is inspecting your traffic |
| **OpenVPN** | Mature and widely understood. Slower |
| **XRay** | Deep packet inspection evasion, using REALITY |
| **IKEv2 / IPsec** | Built into Windows, no extra driver |

**Cloak and Shadowsocks are gone.** Upstream dropped both before our fork point, so neither can
be deployed any more. A server that still runs one is recognised and listed, and the client
tells you the protocol is no longer supported — it does not pretend to connect.

`AmneziaWG` and `AmneziaDNS` keep their names on purpose: they are components you install on
your own server, not our branding. Renaming them would leave you unable to recognise what you
actually deployed.

## Other things it can install

**None of these are installed by default.** Pick a protocol and you get that protocol and
nothing else. The list below is what you *may* add afterwards, one at a time, if you happen to
want it — the path through the interface is step 6 above. Removing it is the same button.

| | |
|---|---|
| **AmneziaDNS** | An `unbound` resolver reachable only inside the tunnel, forwarding over DNS-over-TLS. Your DNS stops going to your provider |
| **SFTP storage** | A private file share on the server |
| **SOCKS5 proxy** | For applications that speak SOCKS but not VPN |
| **MTProxy** | A Telegram proxy running on your server, for handing out access to other people |
| **Telemt** | The same idea, a newer Telegram proxy written in Rust |
| **Website in Tor** | Runs a WordPress site on your server and publishes it as an onion address. Opening that address needs Tor Browser — an ordinary browser cannot resolve `.onion` |

There is no manual setup guide for any of them, because there is nothing to set up by hand.
You never log into the server yourself and you never copy anything onto it: the client opens an
SSH session, builds the container there and closes it. The downloads on this page are the
desktop client — nothing from them is installed on the server.

## Install

Which file to take is in [step 2](#step-2-install-the-program-on-your-own-computer) above. What
is worth knowing beyond that:

Every release is published as a pre-release while the project is young. GitHub's "latest"
shortcut only ever points at a stable release, so it resolves to nothing here — the releases
page itself is the place to look.

macOS, Android and iOS are not built at the moment: the code supports them, the signing
certificates and store accounts do not exist.

Once installed, the client checks this repository's releases for a newer version and offers to
download it. The check sends no request body and carries nothing that identifies your
installation.

## Building

**There is no local build, by choice.** Everything compiles in GitHub Actions: Qt 6.10 plus
nineteen Conan dependencies built from source is not a toolchain worth reproducing on every
machine that touches this code.

Push, then watch [Actions](https://github.com/FFriends/AbstractumVPN/actions). A daily job at
03:00 UTC builds anything new; a weekly job on Sunday at 09:00 UTC publishes a release, and
only when there are commits since the last one.

A local toolchain is still possible if you want a debugger: CMake 3.25+, Conan 2.x, Qt 6.10+
**with Qt Remote Objects** — that module is not in the default Qt install, and without it the
privileged service will not build at all. Then `deploy/build.sh` or `deploy\build.bat`. Nothing
in the project depends on you doing this.

## Security

Report vulnerabilities through
[GitHub Security Advisories](https://github.com/FFriends/AbstractumVPN/security/advisories/new).
It keeps the discussion private until a fix ships, and it stays open even while the issue
tracker is closed. Scope, timelines and what counts as ours are in [SECURITY.md](SECURITY.md).

The client is split in two: an ordinary application, and a service running with system
privileges that owns the routing, the tunnel interface and the kill switch. That boundary is
the part worth scrutinising, and it is where the work has gone:

- the privileged channels serve the client installed next to the service and refuse everyone
  else, which is checked per connection rather than assumed;
- arguments handed to processes started with system privileges go through a whitelist per
  process, and an unrecognised option refuses the whole launch rather than being quietly
  dropped.

One thing to know before you rely on this, because it is not fixed:

- **On Linux the stored server list is not encrypted.** Upstream disabled it because the
  keychain backend is unreliable there, and we have not replaced it. Your server addresses and
  keys sit in plain `QSettings`.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Short version: commits follow `type: summary`, work
happens on `dev`, and the build only runs in CI.

The issue tracker is closed for now. Security reports still go through the advisories link
above; a change is best sent as a pull request, and a question is best asked in the Telegram
chat below.

## Where this gets written about

[**@AbstractumMind**](https://t.me/AbstractumMind) on Telegram: what is being built, what broke
on the way, and whatever else seems worth writing down. Posts are in Russian.

The channel has a chat attached, and you are welcome to write there about anything — help with
the client included — in Russian or English. Other languages work as well, though a translator
sits in between and some of the meaning does not survive the trip.

## Licence and origin

GPL-3.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).

AbstractumVPN is derived from
[amnezia-vpn/amnezia-client](https://github.com/amnezia-vpn/amnezia-client), merged up to
`0d116bfc`. Most of this codebase is their work, and the licence is inherited rather than
chosen: GPLv3 requires it.

This is an independent project. It is not affiliated with or endorsed by Amnezia. **Report
problems with AbstractumVPN here, not to them.** What differs: no subscriptions, no paid
hosting, no advertising, no account, and no request to anyone else's servers — the update
check goes to this repository, everything else goes to the machine you provisioned yourself.

Third-party components and their licences are listed in
[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
