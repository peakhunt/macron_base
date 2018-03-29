#!/bin/sh
### BEGIN INIT INFO
# Provides:          macron_bog
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start the Macron BOG daemon
# Description:       LNG BOG Handling system daemon
### END INIT INFO

set -e

MACRON_BOG="/home/hkim/macron_base/build/macron_bog"

# Check for daemon presence
[ -x "$MACRON_BOG" ] || exit 0

OPTIONS=""
MODULES=""

# Get lsb functions
. /lib/lsb/init-functions

case "$1" in
  start)
    log_begin_msg "Starting macron_bog..."
    start-stop-daemon --start --quiet --oknodo --exec "$MACRON_BOG" -- $OPTIONS
    log_end_msg $?
    ;;
  stop)
    log_begin_msg "Stopping macron_bog..."
    start-stop-daemon --stop --quiet --oknodo --retry 2 --exec "$MACRON_BOG"
    log_end_msg $?
    ;;
  restart)
    $0 stop
    sleep 1
    $0 start
    ;;
  reload|force-reload)
    log_begin_msg "Reloading macron_bog..."
    start-stop-daemon --stop --signal 1 --exec "$MACRON_BOG"
    log_end_msg $?
    ;;
  *)
    log_success_msg "Usage: /etc/init.d/macron_bog {start|stop|restart|reload|force-reload}"
    exit 1
esac
