# Contributing

Thanks for looking. A few things about this project are unusual, so read the first section
before you set anything up.

## There is no local build

Everything compiles in GitHub Actions. This is a decision, not a gap: Qt 6.10 plus nineteen
Conan dependencies built from source is a long first build, and pinning it to one machine's
environment defeats the point.

The loop is: edit, push, watch [Actions](https://github.com/FFriends/AbstractumVPN/actions).

- A scheduled job builds Linux and Windows daily at 03:00 UTC, and only when something
  changed since the last successful run.
- You can trigger it yourself instead of waiting:
  `gh workflow run abstractum-ci.yml -R FFriends/AbstractumVPN`
- Pull requests build automatically.

If you genuinely need a local toolchain, the requirements are CMake 3.25+, Conan 2.x and
Qt 6.10+ **with the Qt Remote Objects module**. That module is not in the default Qt install,
and without it the client-to-service IPC does not compile. A missing Conan shows up as an
OpenSSL error, which is misleading; check `conan --version` first.

## Getting the source

```bash
git clone https://github.com/FFriends/AbstractumVPN.git
```

That is all. There are no submodules: third-party sources under `client/3rd/` are vendored
into the tree, so a fresh clone does not depend on other people's repositories staying up.

## Commits

Prefix in English, message in Russian, written in the first person as you would describe your
own work:

```
fix: поправил переподключение после выхода из сна

Watchdog не срабатывал, потому что состояние туннеля считалось живым по
статусу интерфейса, а не по времени последнего handshake.
```

Prefixes: `feat` `fix` `chore` `ci` `build` `docs` `refactor`.

Keep the body to a few lines and spend them on **why**, not on retelling the diff. If a change
needs fifteen lines to justify, it probably wants to be two changes.

Do not add `Co-Authored-By` or any AI-attribution trailer.

## What not to put in a commit message

This repository is public, and a commit message is as public as the code.

- No local paths (`D:\...`, `/home/you/...`)
- No credentials, tokens or keys, in any form
- No unfixed security findings. Describe what became correct, not what was broken and how to
  reach it. `Validate arguments passed to privileged processes` is fine; spelling out the
  exploit is not.

## Names we do not rename

`AmneziaWG`, `AmneziaDNS`, `OpenVPN`, `WireGuard`, `XRay`, `Cloak`, `Shadowsocks`, `IKEv2`,
`unbound`.

These are components, not our branding. `AmneziaDNS` in particular is an unbound container
that **the user installs on their own server**; rename it in the interface and they can no
longer recognise the thing they deployed. The `amnezia::` namespace and upstream file names
stay for the same practical reason: renaming them turns every future merge into a conflict.

## Branches

Work lands on `dev`. Larger changes go through `feature/**` and a pull request.

## Reporting problems

Bugs and features: [issues](https://github.com/FFriends/AbstractumVPN/issues).

Security vulnerabilities:
[GitHub Security Advisories](https://github.com/FFriends/AbstractumVPN/security/advisories/new),
which keeps things private until a fix ships. Please do not open a public issue for something
exploitable.

## Upstream

This project is derived from
[amnezia-vpn/amnezia-client](https://github.com/amnezia-vpn/amnezia-client) and merges from it
regularly. Two consequences worth knowing:

- **A new file never conflicts. An edited upstream file conflicts forever.** When there is a
  choice, add rather than modify.
- If you are fixing a bug that exists upstream too, say so in the commit. Those fixes are
  tracked separately, because they are the ones most likely to collide on the next merge.
