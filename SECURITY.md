# Security policy

This is a VPN client. A bug here can expose the traffic someone was trying to protect, so
please report anything you find, even if you are not sure it matters.

## Reporting a vulnerability

**Use [GitHub Security Advisories](https://github.com/FFriends/AbstractumVPN/security/advisories/new).**
It keeps the report private until a fix ships, and it works even though the issue tracker is
closed.

If you cannot use that form, write to [@AbstractumMind](https://t.me/AbstractumMind) on
Telegram and ask for a private channel. **Do not put the details in a public message.**

Write in English or Russian, whichever you prefer. Both are read.

## What happens next

This is a small project, so here is what can honestly be promised:

| | |
|---|---|
| First reply | Within a few days. If a week passes with no answer, ping again |
| Fix | Depends on severity. Something that leaks traffic or keys jumps ahead of everything else |
| Credit | You will be named in the advisory unless you ask otherwise |
| Bounty | None. There is no money in this project |

Please give roughly 90 days before disclosing publicly, and less if the issue is being
actively exploited. If a fix is taking longer than that, say so and we will agree on a date
rather than let it drift.

## What is in scope

- The client itself, including the privileged service and the IPC channel between them.
- The scripts under `client/server_scripts/`, which run on machines that users own.
- Our build and release pipeline.

## What is not in scope

- **The protocols themselves.** WireGuard, OpenVPN, XRay, Cloak and Shadowsocks have their own
  maintainers. A flaw in the protocol is not ours to fix.
- **Container images we did not build.** They are pinned by digest, so a report that an image
  is outdated is useful, but the fix lives upstream of us.
- **The server you installed it on.** How that machine is otherwise configured is yours.
- Anything requiring an attacker who already has administrator rights on the same machine.

## Inherited issues

AbstractumVPN is derived from
[amnezia-vpn/amnezia-client](https://github.com/amnezia-vpn/amnezia-client). A bug that came
with that code affects their users too. We fix such issues in our own tree, and we do not
forward reports on your behalf, so if you want them notified, report it to them as well.

Two known weaknesses are already written up in plain sight in the
[README](README.md#security). Neither is fixed. Reporting them again is not necessary, but
reporting a way to actually exploit them very much is.

## Supported versions

Only the latest release. There are no maintenance branches, and there will not be until there
is someone to maintain them.
