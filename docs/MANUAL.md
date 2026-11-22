# CAB.OVL

**HTTP module for CAB under STinG/STiK/Iconnect/MintNet**

By Olivier Booklage. Release V5.68. Version history: [HISTORY.md](HISTORY.md).

This is a Markdown conversion of the UDO manual `CABOVL/ENGLISH/CAB_OVL.U`. The wording is the author's. E-mail addresses and the postal address were left out.

## Contents

1. [About](#about)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Avertissement](#avertissement)
5. [Bugs](#bugs)
6. [History](#history)
7. [Author](#author)
8. [Beta-Testers](#beta-testers)

## About

CAB.OVL is a module for Cristal Atari Brownser 'CAB' for HTTP protocole under TCP/IP stack [STinG](http://www.rottengatter.de), STiK, IConnect or MintNet with GlueStik.

Support server Authentification and cookie under CAB < 2.7 to.

For STiK users, STIK.ACC contains essential TCP/IP routines apart from the in-built dialer, so it must stay present.

**This is a BETA VERSION for beta testers only.**

## Installation

Save your old cab.ovl and put this one in your CAB/MODULES/ folder with is CAB_OVL.RSC file.

## Configuration

Please ask for http://cab.ovl in CAB for set CABOVL configuration.

## Avertissement

Although every effort has been taken by the author to ensure trouble free operation, i can take no responsibility for any damage caused by use of this program, either directly or indirectly. This software is used at your own risk.

Enjoy it

## Bugs

- CAB 2.5X can not send request for URL > 81 characters, some formulars can not be sended propely with this version.
- In some servers with cookies, an error return 'bad request', so you can refuse cookie by configuring until this problem is persistant.
- Many errors come from NetWork, servers or CAB versions so before email me (see Author), **BE SURE THAT MALFUNCTION IS CAB.OVL FAULT** by testing with other plateforms and other CAB.OVL. Send my Logfile, CAB.APP and CAB.OVL versions.

## History

See [HISTORY.md](HISTORY.md).

## Author

Olivier Booklage
http://obooklage.free.fr/

## Beta-Testers

- Ronald Andersson
- Jan Daldrup
- Bernd Maedicke
- the beazt
- Pat Morales
- Derryck Croker
- Joel Houplon
- Harald Placke
- Pascal Ricard
- Matthias Jaap
- Ari Laine
- Juergen Koneczny
- John Banbury

---

Manual written with UDO, http://www.dirk-hagedorn.de/et/
