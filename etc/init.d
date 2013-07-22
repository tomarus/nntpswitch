#!/bin/bash

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/sbin/nntpswitchd
ARGS=
NAME=nntpswitchd
DESC=NNTPSwitch
PIDFILE=/var/run/nntpswitchd.pid

test -x $DAEMON || exit 0
set -e

ulimit -cunlimited

case "$1" in
start)
	if [ ! $EUID -eq 0 ]; then
		echo $0 start needs to be run by root.
		exit 1
	fi

	echo -n "Starting $DESC: "
	if [ -f $PIDFILE ]; then
		echo -n "already running with pid "
		echo `cat $PIDFILE`
		exit 1
	fi
	$DAEMON $ARGS # >/dev/null 2>&1 && echo OK || echo failed : $?
	;;
force-start)
	if [ ! $EUID -eq 0 ]; then
		echo $0 force-start needs to be run by root.
		exit 1
	fi

        logger -p news.err "nntpswitch force-restart called"

	set +e
	echo -n "Forcing start of $DESC: "
	if [ -f $PIDFILE ]; then
		kill -9 `cat $PIDFILE` >/dev/null 2>&1
		# grep op time[r] ipv grep -v grep
		ps ax  | grep 'nntpswitchd: time[r]' |awk '{print $1}' | xargs kill
		rm -f $PIDFILE >/dev/null 2>&1
		sleep 1
	fi
	$DAEMON $ARGS
	echo $NAME
	;;
stop)
	if [ ! $EUID -eq 0 ]; then
		echo $0 stop needs to be run by root.
		exit 1
	fi

	echo -n "Stopping $DESC: "
	if [ ! -f $PIDFILE ]; then
		echo "not running"
	else
		kill `cat $PIDFILE`
		rm -f $PIDFILE >/dev/null 2>&1
		echo $NAME
	fi
	;;
reload)
	echo -n "Reloading $DESC: "
	if [ ! -f $PIDFILE ]; then
		echo "not running"
		exit 1
	fi
	kill -1 `cat $PIDFILE`
	echo $NAME
	;;
restart)
	if [ ! $EUID -eq 0 ]; then
		echo $0 restart needs to be run by root.
		exit 1
	fi

	echo -n "Restarting $DESC: "
	if [ ! -f $PIDFILE ]; then
		echo "not running"
	else
		kill `cat $PIDFILE` && sleep 1
	fi
	$DAEMON $ARGS # >/dev/null 2>&1
	echo $NAME
	;;
config|test|configtest|check)
	$DAEMON -t $ARGS
	;;
*)
	echo "Usage: $NAME {start|stop|reload|restart|force-start|test}" >&2
	exit 1
	;;
esac

exit 0
