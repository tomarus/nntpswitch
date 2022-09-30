NNTPSwitch 0.12+
----------------

This is NNTPSwitch 0.12+

This is the last available production version when www.news-service.com was shut down.

See documentation at: http://redmine.chiparus.net/projects/nntpswitch/wiki

or better at: https://web.archive.org/web/20161023152950/http://redmine.chiparus.net/projects/nntpswitch/wiki

----------------

# wheezy
```
deb http://archive.debian.org/debian wheezy main contrib non-free
deb http://archive.debian.org/debian-security wheezy/updates main contrib non-free
deb http://archive.debian.org/debian wheezy-backports main contrib non-free
```

```
apt-get install build-essential unzip bison libmysqlclient-dev libpq-dev libssl-dev libcurl3-openssl-dev libjson-c-dev libmcrypt-dev libldap2-dev
```


# stretch
```
apt install unzip bison default-libmysqlclient-dev libpq-dev libssl1.0-dev libcurl4-openssl-dev libjson-c-dev libmcrypt-dev libldap2-dev
```

```
# on a 64bit system add -fPIC to the CFLAGS variable in the makefile of both packages.
```


```
cd /usr/src
# tacacs/libmd.tar.gz 10880 bytes
# MD5 3b114cb6d5b491bbf78dbf89d4a60b0e  libmd.tar.gz
# SHA256 b01190f41c988e77e58f81e0e531817f527db51ae2747ae618771ede991dbe13  libmd.tar.gz
# wget http://portal-to-web.de/tacacs/libmd.tar.gz
# wget https://up.ovpn.to/file/3k452BGrLjSinZlhpH4yxnIXA9HmKj5OpFhhe7oz/libmd.tar.gz
tar xfvz libmd.tar.gz
cd libmd
mkdir /usr/local/man/man3
make  && make install

cd /usr/src
# tacacs/libradius-linux-20040827.tar.gz 15791 bytes
# MD5 c7cc2f49acdd9955a052029326833fe1  libradius-linux-20040827.tar.gz
# SHA256 e8beead35a82836171f1e5a199b185555d35f12acaaa86c6533af47b4a4f109b  libradius-linux-20040827.tar.gz
# wget http://portal-to-web.de/tacacs/libradius-linux-20040827.tar.gz
# wget https://up.ovpn.to/file/Spd7lU1oTwcpmcEwtgEt6Q780fePIZ6WM4TzSg1O/libradius-linux-20040827.tar.gz
tar xfvz libradius-linux-20040827.tar.gz
cd libradius-linux
make && make install

useradd -m -s /bin/bash nntpswitch
cd /home/nntpswitch
su nntpswitch

cd #
wget https://github.com/tomarus/nntpswitch/archive/master.zip
unzip master.zip
cd nntpswitch-master
```

libmd
```
6480eedccaab7e014ccc4bd2229b3e7d  Makefile
3fc668845d2980b8bb36446786feabe7  md2c.c
8382580ecd575810b50148d28a6e9bc0  md2.copyright
3dea7466f4a71fbc6832666a8de798da  md2.h
5dc2525dbd65e2d7a582595eac5c0f79  md4c.c
08810d568d0e710e411fa6183d15f66f  md4.copyright
a50233a9c763895aa1eb3fc500507e09  md4.h
ad3f7fd831f3227a7963d03d9377ed6e  md5c.c
4fd5a5f031473d08f902f754b0d9d74f  md5.copyright
b9fa9bc32c149c7d3ffe96a7c2dcde6c  md5.h
faaf018f540fc59e668fb35d0f324e2c  mddriver.c
dc1246470757f8b927dd98a32ca69d7b  mdX.3
1c0e63499802048cfe5a8c0ffedb2eea  mdXhl.c
8f8a3e36457856c6c5906c76dc7c02b5  README
```

libradius-linux-20040827
```
3529d884538b1007345ed8b80a220f77  libradius.3
ff94c2c30331cc004698d904f7c143b1  Makefile
b9fa9bc32c149c7d3ffe96a7c2dcde6c  md5.h
cd2f13f4ceddd295b090194f562cf311  radius.conf.5
44816c51b83d4ae516065ff4e2f7c7c6  radlib.c
8d590ceac33a370d5d9809cb674bec29  radlib.h
f98c5f450ad6dfb8cd9d86d8bbd6aef2  radlib_private.h
f8df6c1e97cc4471ddc72ec88fb85298  README
```
