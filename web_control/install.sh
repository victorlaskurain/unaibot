#!/bin/bash

set -e

INSTALLDIR=/opt/unaibot
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"
mkdir -p $INSTALLDIR/share $INSTALLDIR/bin $INSTALLDIR/etc
cp -rL unaibot_web.js html lib node_modules $INSTALLDIR/share
cp -rL etc/* $INSTALLDIR/etc/
cat > $INSTALLDIR/bin/unaibot_web <<EOF
#!/bin/bash
test -f /opt/node.login && . /opt/node.login
node /opt/unaibot/share/unaibot_web.js --serial-port="\$SERIAL_PORT" --http-basedir="\$HTTP_BASEDIR" --http-port="\$HTTP_PORT" 2>&1 | logger -t unaibot_web
EOF
chmod a+x $INSTALLDIR/bin/unaibot_web
sudo systemctl link /opt/unaibot/etc/systemd/system/unaibot.service
