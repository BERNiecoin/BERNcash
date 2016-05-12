
Copyright (c) 2016 BERN Developers


BERN 1.0.0 BETA

Copyright (c) 2016 BERN Developers
Copyright (c) 2013-2014 SoleCoin Developers
Copyright (c) 2013 NovaCoin Developers
Copyright (c) 2011-2012 Bitcoin Developers
Distributed under the MIT/X11 software license, see the accompanying
file license.txt or http://www.opensource.org/licenses/mit-license.php.
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/).  This product includes
cryptographic software written by Eric Young (eay@cryptsoft.com).


Intro
-----
BERN is a free open source project derived from NovaCoin and subsequelty Bitcoin, with
the goal of providing a long-term energy-efficient x14-based crypto-currency to support
progressive politics. Built on the foundation of Bitcoin and NovaCoin, innovations such as proof-of-stake
help further advance the field of crypto-currency.

Setup
-----
After completing windows setup then run windows command line (cmd)
  cd daemon
  BERNd
You would need to create a configuration file BERN.conf in the default
wallet directory. Grant access to BERNd.exe in anti-virus and firewall
applications if necessary.

The software automatically finds other nodes to connect to.  You can
enable Universal Plug and Play (UPnP) with your router/firewall
or forward port 32020 (TCP) to your computer so you can receive
incoming connections.  BERN works without incoming connections,
but allowing incoming connections helps the BERN network.

Upgrade
-------
All you existing coins/transactions should be intact with the upgrade.
To upgrade first backup wallet
BERNd backupwallet <destination_backup_file>
Then shutdown BERNd by
BERNd stop
Start up the new BERNd.


See the BERN site:
  http://BERN.org/
for help and more information.

