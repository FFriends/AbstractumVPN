# Pinned by digest — see awg/Dockerfile. Was :latest, resolved 2026-08-01.
FROM amneziavpn/mtproxy@sha256:c5e00d700bc1f81cbc2a60ad05bd65a0db34d50a7fb531125ab7ecc3708db7c2

RUN mkdir -p /opt/amnezia /data
RUN printf '#!/bin/sh\ntail -f /dev/null\n' > /opt/amnezia/start.sh && \
    chmod a+x /opt/amnezia/start.sh

VOLUME /data
ENTRYPOINT ["/bin/sh", "/opt/amnezia/start.sh"]
CMD [""]
