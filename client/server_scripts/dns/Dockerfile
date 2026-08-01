# AmneziaDNS. Pinned by digest — see awg/Dockerfile. This one resolves every DNS
# query the user makes inside the tunnel, so a silently substituted image would
# be the worst possible place for one. Was :latest, resolved 2026-08-01.
FROM mvance/unbound@sha256:76906da36d1806f3387338f15dcf8b357c51ce6897fb6450d6ce010460927e90

LABEL maintainer="AmneziaVPN" 

RUN echo " \n\
domain-insecure: \"coin.\"\n\
domain-insecure: \"emc.\"\n\
domain-insecure: \"lib.\"\n\
domain-insecure: \"bazar.\"\n\
domain-insecure: \"enum.\"\n\
\n\
stub-zone:\n\
   name: coin.\n\
   stub-host: seed1.emercoin.com\n\
   stub-host: seed2.emercoin.com\n\
   stub-first: yes\n\
\n\
stub-zone:\n\
   name: emc.\n\
   stub-host: seed1.emercoin.com\n\
   stub-host: seed2.emercoin.com\n\
   stub-first: yes\n\
\n\
stub-zone:\n\
   name: lib.\n\
   stub-host: seed1.emercoin.com\n\
   stub-host: seed2.emercoin.com\n\
   stub-first: yes\n\
\n\
stub-zone:\n\
   name: bazar.\n\
   stub-host: seed1.emercoin.com\n\
   stub-host: seed2.emercoin.com\n\
   stub-first: yes\n\
\n\
stub-zone:\n\
   name: enum.\n\
   stub-host: seed1.emercoin.com\n\
   stub-host: seed2.emercoin.com\n\
   stub-first: yes\n\
\n\
forward-zone:\n\
   name: .\n\
   forward-tls-upstream: yes\n\
   forward-addr: 1.1.1.1@853  #cloudflare-dns.com\n\
   forward-addr: 1.0.0.1@853  #cloudflare-dns.com\n\
" | tee /opt/unbound/etc/unbound/forward-records.conf

