#!/bin/sh
#
# Example script for NNTPSwitch statistics generation.
#
# This script rotates the news.notice by one. You have to take
# care of other news logfiles yourself.
# If you need anything else, put it in nsstats.sh.local
#
 
LOGDIR=/var/log/news
WWWDIR=/var/www
BINDIR=/usr/local/sbin

# First rotate logging and reload syslog
rm -f $LOGDIR/news.notice.*
mv $LOGDIR/news.notice $LOGDIR/news.notice.1
#touch $LOGDIR/news.notice
#chown news.news $LOGDIR/news.notice
/etc/init.d/syslog-ng reload

# Update all groups
$BINDIR/updategroups
/etc/init.d/nntpswitch reload

# Make stats
$BINDIR/nsstats.pl -d $WWWDIR < $LOGDIR/news.notice.1
gzip $LOGDIR/news.notice.1

# Any local additions should be in nsstats.sh.local
LOCAL=`basename $0`
if [ -f $BINDIR/$LOCAL.local ]; then
  $BINDIR/$LOCAL.local
fi

