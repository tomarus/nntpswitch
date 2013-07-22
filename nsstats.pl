#!/usr/bin/perl -w
#
# nntpswitch statistics generator
# $Id: nsstats.pl,v 1.51 2010-11-12 11:42:22 tommy Exp $
#

use strict;
use Time::ParseDate;
use POSIX qw(strftime mktime);
use Getopt::Std;

my $PrintUnknowns = 1;
my $TopStats	  = 30;
my $GroupTopStats = 50;
my $HTMLHeader    = "";
my $HTMLFooter    = "";
my $DIR;	  # set by option -d
my $FILE	  = undef;

my %opts;
my @unparsable  = ();
my @unknown     = ();
my %connects;	# connects by host
my $totconnects = 0;
my %commands;	# total commands by command
my $totcommands = 0;
my %articles;	# articles by host
my $totarticles = 0;
my %groups;	# groups by host
my $totgroups   = 0;
my %bytes;	# bytes by host
my $totbytes    = 0;
my %servers;	# server connects 
my %group;	# individual group requests 
my %timeouts;	# timeout strings
my %timeoutsl;  # timeout strings last time
my %sdowns;	# server downs
my %sdownsl;	# server downs last time
my %auths;	# authinfo results
my %grouparts;
my %groupbytes;
my %serverarts;
my %serverbytes;
my %hostuser;	# last authinfo user on host
my $totserverbytes = 0;
my $stime = 0;	# first log entry start time
my $etime = 0;  # and the end 
my %xtrace;	# shoud x-trace have a separate log?
my %postfilter;
my $totposts = 0;
my $totpostbytes = 0;
my %statindex;	# for index.html
my %posts;	# posts by host
my %postbytes;	# by host

# 
# fb() print number of bytes in human readable format
#
sub fb {
    my ($b) = @_;
    if ($b>=1000000000000) { return sprintf("%.2f Tb",$b/1000000000000); }
    if ($b>=1000000000) { return sprintf("%.2f Gb",$b/1000000000); }
    if ($b>=1000000) { return sprintf("%.2f Mb",$b/1000000); }
    if ($b>=1000) { return sprintf("%.2f Kb",$b/1000); }
    return $b;
}

sub hhead {
    if ( $HTMLHeader ne "" ) {
        open(IF,$HTMLHeader) || die $!;
	print $FILE <IF>;
	close(IF);
    }else{
        print $FILE <<STDHEADER;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en"><head><title>NNTPSwitch Statistics</title>
<style type="text/css" media="screen">\@import "style.css";</style>
</head>
<body>
STDHEADER
    }
}

sub hfoot {
    if ( $HTMLFooter ne "" ) {
        open(IF,$HTMLFooter) || die $!;
	print $FILE <IF>;
	close(IF);
    }else{
        print $FILE "</body></html>\n";
    }
}

############################################################
#
# pretty output routines, check for plain or html output
#
############################################################

sub htitle {
    my ($cs,$msg,$nam) = @_;
    print $FILE "<p><a id=\"$nam\"/></p><table border=\"1\"><tr><th colspan=\"$cs\">$msg</th></tr>\n";
}

sub hend {
    print $FILE "</table>\n";
}

sub rowtab {
    print $FILE "<tr><td>";
    print $FILE join("</td><td>",@_);
    print $FILE "</td></tr>\n";
}

sub rowtab_right {
    print $FILE "<tr><td align=\"right\">";
    print $FILE join("</td><td align=\"right\">",@_);
    print $FILE "</td></tr>\n";
}

sub headtab {
    print $FILE "<tr><th>";
    print $FILE join("</th><th>",@_);
    print $FILE "</th></tr>\n";
}

sub ltgt {
    $_=~s/\</\&lt\;/g;
    $_=~s/\>/\&gt\;/g;
    $_;
}

############################################################
#
# read and parse the logfile from stdin
#
############################################################

our $usage = "Usage: $0 [-h] [-n] -d /output/dir\n\t-h\tthis help\n\t-n\tdont generate trace stats\n";
getopts("d:nh", \%opts);
die $usage if ( $opts{h} || ! $opts{d} );
$DIR = $opts{d};

while(<STDIN>){
    chop;
    next if ! m/nntpswitch/;
    next if m/\: command /o;
    next if m/last message repeated/io;
    next if m/(read|write)(client|server)/io;
    next if m/too many concurrent connections/io;
    next if m/accounting_entry/io;
    next if m/module.*loaded/io;
    next if m/active.*groups/io;
    next if m/active file freed/io;
    next if m/loaded.*acls/io;
    next if m/Freed.*(acls|Servers|Wildmats|Aliases|Authenticators)/io;
    next if m/loaded access.conf/io;
    next if m/Startup Test Successfull/io;
#    next if m/disconnected from server (\S+)$/o;
    next if m/client error 500/io;
    next if m/post server responded (235|335)/io;
    next if m/received.*realmsgid/io;
    next if m/Session Time Limit Reached/o;
    next if m/Server running new pid/o;
    next if m/NNTP Server Starting/o;
    next if m/Listening on/o;
    next if m/updategroups\[/o;
    next if m/Loaded (\d+) (servers|newsgroups|overview.fmt|profiles|wildmats|aliases|authenticators)/i;
    next if m/Using configuration/o;
    next if m/auth_radius. (Denied|Successful) Login/;
    next if m/missing_article/;
    next if m/DEBUG/;
    next if m/assigning.*userkey|assigning-change|dropping connection profile/;
    next if m/auth_resellermap: login/;
    next if m/radius acct server not responding/;
    next if m/auth_usetracks/;
    next if m/quota limit reached/;
    next if m/SSL call SSL_(accept|write|read) cause error/;
    next if m/SSL_accept timeout/;
    next if m/Cant set up SSL connection/;
    next if m/connecting to server/o;
    next if m/auth_.*no.such.user/;
    next if m/password mismatch/;
    # tmp stuff
    next if m/posting body \d+ bytes/;
    next if m/post body done checking for result/;
    next if m/post received..telling client msg_post_received/;
    next if m/Finishing old post connection/;
    next if m/auth.*login.*profile.*kbit.*gb/;

    s/\^M//g;
    my ($month, $day, $time, $server, $daemon, $pid, undef) = 
      m/^(\S+)\s+(\d+)\s+([0-9\:]+)\s+(\S+)\s+(\S+)\[(\d+)\]\:(.*)/o;

    if ( ! $month || ! $day || ! $time || ! $server || ! $daemon || ! $pid ) {
        push @unparsable, $_;
	next;
    }

    $stime = parsedate("$month $day $time") if $stime == 0;
    $etime = parsedate("$month $day $time");

    if ( m/connect from (\S+)\:/o ) {
        next if ! $1;
	$connects{$1}++;
	$totconnects++;
	next;

    }elsif ( m/(\S+)\: commands usage (.*)/o ) {
        my $cmds = $2;
    
        while ( $cmds =~ m/(\w+) (\d+) ?(.*)/ ) {
            $commands{uc($1)} += $2;
            $totcommands += $2;
            $cmds = $3;
        }
        next;

    }elsif ( m/\: group (\S+) closed articles (\d+) bytes (\d+)/io ) {
        next if ! $1;
	$grouparts{lc($1)} += $2;
	$groupbytes{lc($1)} += $3;
	next;

    }elsif ( m/\: group (\S+)/o ) {
        next if ! $1;
	$group{lc($1)}++;
	next;

    # remove 2nd m//, and defined $5 and $6 after migration
    }elsif ( m/(\S+)\: connection closed articles (\d+) bytes (\d+) groups (\d+) posts (\d+) postbytes (\d+)/o ||
    	     m/(\S+)\: connection closed articles (\d+) bytes (\d+) groups (\d+)/o ) {
	$articles{$1} += $2;
	$totarticles  += $2;
	$bytes{$1}    += $3;
	$totbytes     += $3;
	$groups{$1}   += $4;
	$totgroups    += $4;
	if ( defined $5 && defined $6 ) {
	    $posts{$1}    += $5;
	    $postbytes{$1}+= $6;
    	    $totposts     += $5;
	    $totpostbytes += $6;
        }
	next;

    }elsif ( m/disconnected from server (\S+) articles (\d+) bytes (\d+)/io ) {
        $serverarts{$1} += $2;
	$serverbytes{$1} += $3;
	$totserverbytes += $3;
	$servers{$1}++;
	next;

#    }elsif ( m/connecting to server (\S+)/o ) {
#	$servers{$1}++;
#	next;

    }elsif ( m/(backup server .*|remote server .*|cant s?write to server .*|no such article, found in (\d+) retries|connect to server .* failed\: .*)/io ) {
    	$sdowns{$1}++;
	$sdownsl{$1} = $etime;
	next;

    }elsif ( m/(timeout after \d+ seconds|timeout or error .*|client read timeout|cant slwrite to client .*|cant write to server (.*)|eof from client|Can.t get default ACL|session (time|byte) limit reached|too many users|read_socket: select timeout|denied connection)/io ) {
	$timeouts{$1}++;
	$timeoutsl{$1} = $etime;
	next;

    }elsif ( m/(auth_radius: unknown attribute: \d+)/) {
	$auths{$1}++;
	next;

    # no username logged with this line
    }elsif ( m/authinfo login.*result.*(482 Too many connections for your user)/ )
    {
    	$auths{$1}++;
	next;

    }elsif ( m/(\S+): authinfo login: (.+) result: (.*)/o ) {
        $hostuser{$1} = $2;
	my $res = $3;
	if ( $res =~ m/(Authentication Accepted)/ )
	{
    		$auths{$1}++;
	} elsif ( $res =~ m/(.OK..Download limit)/ )
	{
		$auths{$1}++;
	} else {
    		$auths{$res}++;
	}
	next;

#    }elsif ( m/Authentication Accepted|.OK. Download limit:/) {
#	$auths{"Authentication Accepted"}++;
#	next;

    }elsif ( m/X-Trace: (.*)/o ) {
        $xtrace{$1} = $_;
	next;

    }elsif ( m/post server responded (.*)/o ) {
    	$postfilter{$1}++;
	next;

    }elsif ( m/(Post Aborted or No body received|Post to nonexistent group)/o ) {
    	$postfilter{$1}++;
	next;

    }else{
	push @unknown, $_;
    }
}

############################################################
#
# print the report
#
############################################################

# This is a workaround for rollover years if you generate
# statistics from 31-dec on 01-jan.
my $nwy = strftime("%m%d",localtime($etime)); # newyear
my $cry = strftime("%m%d",localtime(time())); # curryear
if ( $nwy eq "0101" && $cry eq "0101" ) {
    my $yr = strftime("%Y", localtime($stime));
    $yr--;
    $stime = parsedate( "$yr-".strftime("%m-%d %H:%M:%S", localtime($stime)) );
}

my $s = strftime("%Y%m%d_%H%M",localtime($stime));
my $e = strftime("%Y%m%d_%H%M",localtime($etime));
my $ss= strftime("%d-%m-%Y %H:%M:%S",localtime($stime));
my $ee= strftime("%d-%m-%Y %H:%M:%S",localtime($etime));

if ( ! open($FILE, ">$DIR/stats.$s-$e.html") ) {
    die("Can't open $DIR/stats.$s-$e.html for output, $!");
}

hhead();
print $FILE "<h1>NNTPSwitch Statistics $ss - $ee</h1>\n";

print $FILE "<ul>\n";
print $FILE "<li><a href=\"#totals\">Totals</a></li>\n";
print $FILE "<li><a href=\"#cmdusage\">Command usage</a></li>\n";
print $FILE "<li><a href=\"#serverconns\">Server Connections</a></li>\n" if ( keys %servers );
print $FILE "<li><a href=\"#discreasons\">Client Disconnect Reasons</a></li>\n";
print $FILE "<li><a href=\"#serverdowns\">Server Downs</a></li>\n" if ( keys %sdowns );
print $FILE "<li><a href=\"#authresults\">Authentication Results</a></li>\n" if ( keys %auths );
print $FILE "<li><a href=\"#postfilter\">Posting Filter Results</a></li>\n" if ( keys %postfilter );
print $FILE "<li><a href=\"#topposts\">Top $TopStats Posts by Host</a></li>\n" if ( $totposts > 0 );
print $FILE "<li><a href=\"#toppostbytes\">Top $TopStats Post Bytes by Host</a></li>\n" if ( $totposts > 0 );
print $FILE "<li><a href=\"#topconnects\">Top $TopStats Connects by Host</a></li>\n";
print $FILE "<li><a href=\"#topbytes\">Top $TopStats Bytes by Host</a></li>\n";
print $FILE "<li><a href=\"#topgroups\">Top $TopStats Groups by Host</a></li>\n";
print $FILE "<li><a href=\"#toparticles\">Top $TopStats Articles by Host</a></li>\n";
print $FILE "<li><a href=\"#topgroups\">Top $GroupTopStats Group Statistics by Article</a></li>\n";
print $FILE "<li><a href=\"#topgroupbytes\">Top $GroupTopStats Group Statistics by Bytes</a></li>\n";
print $FILE "<li><a href=\"#unparsable\">Unparsable Log Entries</a></li>\n" if ( $PrintUnknowns && @unparsable );
print $FILE "<li><a href=\"#unknown\">Unknown Log Entries</a></li>\n" if ( $PrintUnknowns && @unknown );
print $FILE "</ul>\n";
print $FILE "<p/>\n";

htitle(2,"Totals","totals");
rowtab("Total Connects:",	    $totconnects);
rowtab("Total Commands:",	    $totcommands);
rowtab("Total Articles:",	    $totarticles);
rowtab("Total Accounted Bytes:",    fb($totbytes));
rowtab("Total Bytes from Server:",  fb($totserverbytes));
rowtab("Accounted Traffic:",	    sprintf("%.1f %%",((($totbytes+1)/($totserverbytes+1))*100)+.05));
rowtab("Total Group Changes:",	    $totgroups);
rowtab("Unique Groups:",	    scalar keys %group);
rowtab("Unique Hosts:",		    scalar keys %connects);
rowtab("Total Posts:",		    $totposts);
rowtab("Total Post Size:",	    fb($totpostbytes));
hend();

htitle(2,"Command usage","cmdusage");
foreach( sort { $commands{$b} <=> $commands{$a} } keys %commands ) {
    rowtab($_, $commands{$_}) if ( $commands{$_} > 0 );
}
hend();

if ( keys %servers ) {
    htitle(4,"Server Connections","serverconns");
    headtab("Server", "Connections", "Articles", "Bytes");
    foreach( sort keys %servers ) {
        $serverarts{$_} = 0 if ! $serverarts{$_};
        $serverbytes{$_} = 0 if ! $serverbytes{$_};
        rowtab($_, $servers{$_}, $serverarts{$_}, fb($serverbytes{$_}));
    }
    hend();
}

htitle(3,"Client Disconnect Reasons","discreasons");
headtab("Last","Count","Message");
foreach( sort { $timeouts{$b} <=> $timeouts{$a} } keys %timeouts ) {
    rowtab(strftime("%H:%M:%S",localtime($timeoutsl{$_})), $timeouts{$_}, $_);
}
hend();

if ( keys %sdowns ) {
    htitle(3,"Server Downs","serverdowns");
    headtab("Last","Count","Message");
    foreach( sort { $sdowns{$b} <=> $sdowns{$a} } keys %sdowns ) {
        rowtab(strftime("%H:%M:%S",localtime($sdownsl{$_})), $sdowns{$_}, $_);
    }
    hend();
}

if ( keys %auths ) {
    htitle(2,"Authentication Results","authresults");
    foreach( sort { $auths{$b} <=> $auths{$a} } keys %auths ) {
        rowtab($_, $auths{$_});
    }
    hend();
}

if ( keys %postfilter ) {
    htitle(2,"Posting Filter Results","postfilter");
    foreach( sort { $postfilter{$b} <=> $postfilter{$a} } keys %postfilter ) {
        rowtab($_, $postfilter{$_});
    }
    hend();
}

my $i = 0;

if ( $totposts > 0 ) {
    $i = 0;
    htitle(4,"Top $TopStats Posts by Host","topposts");
    headtab("Host", "Posts", "Bytes", "User");
    foreach( sort { $posts{$b} <=> $posts{$a} } keys %posts ) {
        last if $posts{$_} == 0;
        $hostuser{$_}  = "&nbsp;" if ! $hostuser{$_};
        $postbytes{$_} = 0   if ! $postbytes{$_};
        rowtab($_, $posts{$_}, fb($postbytes{$_}), $hostuser{$_} );
        $i++;
        last if $i >= $TopStats;
    }
    hend();

    $i=0;
    htitle(4,"Top $TopStats Post Bytes by Host","toppostbytes");
    headtab("Host", "Posts", "Bytes", "User");
    foreach( sort { $postbytes{$b} <=> $postbytes{$a} } keys %postbytes ) {
        last if $postbytes{$_} == 0;
        $hostuser{$_}  = "&nbsp;" if ! $hostuser{$_};
        $postbytes{$_} = 0   if ! $postbytes{$_};
        rowtab($_, $posts{$_}, fb($postbytes{$_}), $hostuser{$_} );
        $i++;
        last if $i >= $TopStats;
    }
    hend();
}

$i = 0;
htitle(6,"Top $TopStats Connects by Host","topconnects");
headtab("Host", "Connects", "Articles", "Groups", "Bytes", "User");
foreach( sort { $connects{$b} <=> $connects{$a} } keys %connects ) {
    last if $connects{$_} == 0;
    $hostuser{$_}="&nbsp;" if ! $hostuser{$_};
    $connects{$_}=0 if ! $connects{$_};
    $articles{$_}=0 if ! $articles{$_};
    $groups{$_}=0   if ! $groups{$_};
    $bytes{$_}=0    if ! $bytes{$_};
    rowtab($_, $connects{$_}, $articles{$_}, $groups{$_}, fb($bytes{$_}), $hostuser{$_} );
    $i++;
    last if $i >= $TopStats;
}
hend();

$i = 0;
htitle(6,"Top $TopStats Bytes by Host","topbytes");
headtab("Host", "Connects", "Articles", "Groups", "Bytes", "User");
foreach( sort { $bytes{$b} <=> $bytes{$a} } keys %bytes ) {
    last if $bytes{$_} == 0;
    $hostuser{$_}="&nbsp;" if ! $hostuser{$_};
    $connects{$_}=0 if ! $connects{$_};
    $articles{$_}=0 if ! $articles{$_};
    $groups{$_}=0   if ! $groups{$_};
    rowtab($_, $connects{$_}, $articles{$_}, $groups{$_}, fb($bytes{$_}), $hostuser{$_} );
    $i++;
    last if $i >= $TopStats;
}
hend();


$i = 0;
htitle(6,"Top $TopStats Groups by Host","topgroups");
headtab("Host", "Connects", "Articles", "Groups", "Bytes", "User");
foreach( sort { $groups{$b} <=> $groups{$a} } keys %groups ) {
    last if $groups{$_} == 0;
    $hostuser{$_}="&nbsp;" if ! $hostuser{$_};
    $connects{$_}=0 if ! $connects{$_};
    rowtab($_, $connects{$_}, $articles{$_}, $groups{$_}, fb($bytes{$_}), $hostuser{$_} );
    $i++;
    last if $i >= $TopStats;
}
hend();


$i = 0;
htitle(6,"Top $TopStats Articles by Host","toparticles");
headtab("Host", "Connects", "Articles", "Groups", "Bytes", "User");
foreach( sort { $articles{$b} <=> $articles{$a} } keys %articles ) {
    last if $articles{$_} == 0;
    $hostuser{$_}="&nbsp;" if ! $hostuser{$_};
    $connects{$_}=0 if ! $connects{$_};
    rowtab($_, $connects{$_}, $articles{$_}, $groups{$_}, fb($bytes{$_}), $hostuser{$_} );
    $i++;
    last if $i >= $TopStats;
}
hend();


$i = 0;
htitle(4,"Top $GroupTopStats Group Statistics by Article","topgrouparts");
headtab("Group", "Connects", "Articles", "Bytes");
foreach( sort { $grouparts{$b} <=> $grouparts{$a} } keys %grouparts ) {
    rowtab($_, $group{$_} ? $group{$_} : "0", $grouparts{$_}, fb($groupbytes{$_}));
    $i++;
    last if $i >= $GroupTopStats;
}
hend();

$i = 0;
htitle(4,"Top $GroupTopStats Group Statistics by Bytes","topgroupbytes");
headtab("Group", "Connects", "Articles", "Bytes");
foreach( sort { $groupbytes{$b} <=> $groupbytes{$a} } keys %groupbytes ) {
    rowtab($_, $group{$_} ? $group{$_} : "0", $grouparts{$_}, fb($groupbytes{$_}));
    $i++;
    last if $i >= $GroupTopStats;
}
hend();

if ( $PrintUnknowns ) {
    if ( @unparsable ) {
        htitle(1,"Unparsable Log Entries","unparsable");
        for(@unparsable){
            rowtab(ltgt($_));
        }
	hend();
    }

    if ( @unknown ) {
	htitle(1,"Unknown Log Entries","unknown");
	for(@unknown){
            rowtab(ltgt($_));
	}
	hend();
    }
}
hfoot();
close($FILE);

############################################################
#
# create the trace log
#
############################################################

my ($trace, $uhost, $msgid, $profile, $uname);

if ( $totposts > 0 && ! $opts{n} ) {

if ( ! open($FILE, ">$DIR/trace.$s-$e.txt") ) {
    die("Can't open $DIR/trace.$s-$e.txt for output, $!");
}

print $FILE "NNTPSwitch X-Trace Log $ss - $ee\n\n";
print $FILE "X-Trace                    Profile         Username           Hostname                                 Message-ID\n";

foreach( keys %xtrace ) {

    m/id=\"(.*)\" msgid=\"(.*)\" host=\"(.*)\" profile=\"(.*)\"(.*)/;
    $trace = $1;
    $msgid = $2;
    $uhost = $3;
    $profile = $4;

    $uname = "";
    if ( $5 )
    {
    	$5 =~ m/user=\"(.*)\"/;
	$uname = $1;
    }

    print $FILE sprintf("%-26s %-18s %-16s %-40.40s %s\n", $trace, $uname, $profile, $uhost, $msgid);
}

close $FILE;
} #totposts>0

############################################################
#
# create the index.html 
#
############################################################

my $d;
if ( ! opendir($d,$DIR) ) { die "Cant open $DIR $!\n"; }
while($_=readdir($d)) {
   next if ! m/^stats\.([0-9_]+)\-([0-9_]+)\.html$/o;
   $statindex{"$1-$2"} = "$1-$2";
}
closedir($d);

if ( ! open($FILE, ">$DIR/index.html") ) {
    die("Can't open $DIR/index.html for output, $!");
}

hhead();
print $FILE "<h1>NNTPSwitch Statistics</h1>\n";
htitle(10,"Daily Statistics","daily");
headtab("Date", "Connections", "Commands", "Articles", "Bytes", "Groups", "Hosts", "Posts", "Post Size", "Log");

foreach( sort { $b cmp $a } keys %statindex ) {
    my $dat = $_;
    my $f   = sprintf("stats.%s.html",$statindex{$dat});
    my $tf  = sprintf("trace.%s.txt", $statindex{$dat});
    if ( -f "$DIR/$tf" ) {
        $tf = "<a href=$tf>trace</a>";
    }else{
        $tf = "n/a";
    }

    my ($tc, $tcm, $ta, $tb, $tug, $tuh, $ps, $pb) = (0,0,0,0,0,0,0,0);

    if ( open(F, "$DIR/$f") ) {
	while(<F>) {
	    if ( m/Total Connects\:\<\/td\>\<td\>(\d+)/ ) {
		$tc = $1;
	    }elsif ( m/Total Commands\:\<\/td\>\<td\>(\d+)/ ) {
		$tcm = $1;
	    }elsif ( m/Total Articles\:\<\/td\>\<td\>(\d+)/ ) {
		$ta = $1;
	    }elsif ( m/Total Accounted Bytes\:\<\/td\>\<td\>(.*)\<\/td>/ ) {
		$tb = $1;
	    }elsif ( m/Unique Groups\:\<\/td\>\<td\>(\d+)/ ) {
		$tug = $1;
	    }elsif ( m/Unique Hosts\:\<\/td\>\<td\>(\d+)/ ) {
		$tuh = $1;
	    }elsif ( m/Total Posts\:\<\/td\>\<td\>(\d+)/ ) {
		$ps = $1;
	    }elsif ( m/Total Post Size\:\<\/td\>\<td\>(.*)\<\/td>/ ) {
		$pb = $1;
	    }
	    last if m/Command Usage/;
	}
	close(F);

	my $url = "<a href=\"$f\">".$statindex{$dat}."</a>";
	rowtab_right($url, $tc, $tcm, $ta, $tb, $tug, $tuh, $ps, $pb, $tf);
    }
}
hend();
hfoot();
close $FILE;

############################################################
#
# create a default stylesheet
#
############################################################
if ( ! -f "$DIR/style.css" )
{
	 open($FILE, ">$DIR/style.css") || die("Can't open $DIR/index.html for output, $!");

	 print $FILE <<EOFSTYLE;
body {
        font-family: arial, helvetica;
        font-size: 10pt;
}
h1 {
        color: #333;
        border-bottom: 2px solid black;
        margin: 0px;
}
table {
        border: 1px solid black;
        border-collapse: collapse;
}
th {
        font-size: 11pt;
        font-weight: bold;
        border-bottom: 1px solid black;
        padding: .5em;
        background: #eee;
}
td {
        font-size: 10pt;
        vertical-align: top;
        border-bottom: 1px dotted #aaa;
        padding: 2px;
        padding-left: 4px;
        padding-right: 4px;
}
EOFSTYLE
	close($FILE);
}

# exit
1;

