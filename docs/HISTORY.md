# CAB.OVL history

**HTTP module for CAB under STinG/STiK/Iconnect/MintNet**

By Olivier Booklage. Release V5.68, January 19, 2000. Manual: [MANUAL.md](MANUAL.md).

This is a Markdown conversion of the UDO file `CABOVL/ENGLISH/HISTORY.U`. The wording is the author's. The HTML test forms of the original page (mailto, authentication, gopher images) and e-mail addresses were left out.

## V5.68

- Fixed: Very old bug, cab.ovl crash if a HTTP proxy was defined without semi-colon ":" followed by the port value.

## V5.67

- Fixed: FTP bug if no HTTP proxy.
- New: Speed increased.

## V5.66

- Fixed: HEAD method preserved for redirections (autosurf).

## V5.65

- Fixed: POST method redirected is now fixed in GET, some redirections and formulars run again.

## V5.64

- Fixed: V5.63 crash with CABDLOAD :( . Thank you to Joel Houplon.
- Fixed: For cache problem, errors are often "Can not write the cache file" for any reason. Thank you to Joel Houplon.

## V5.63

- New: Subject in mailto URL can be transmitted by "?" behind address. EG: "mailto:name@example.com?Test from cab.ovl documentation".
- New: mailto can be used under IConnect. Thank to Juergen Koneczny.
- Neu: deutschesprachige RSC-Datei und Anleitung, Dank an Harald Placke.
- Fixed: Bug in case of root redirection as http://www.mysite/pub to http://www.mysite/pub/ because CAB refuse (right) to handle a new filename. Maybe solve M&N Freeman's bug ?. Thank you to John Banbury.

## V5.62

- New: Authentification re-writted.
- New: No more alert for http header forgetted. Thank to Bernd Maedicke.
- New: Internal "tried be" non-blocking resolver.
- New: Data speed in turbo mode under IConnect.
- New: http and ftp proxies can be used under IConnect.
- New: Stop button and Esc key can be used under IConnect. Thank to Ari Laine.
- Not Fixed: mailto can not be used under IConnect, but do not crash CAB. Thank to Harald Placke.

## V5.61

- Fixed: mailto was running only if ftp_proxy was set !!. Thank to Joel Houplon.
- Fixed: Bad user-agent. Thank to Matthias Jaap.

## V5.60

- Fixed: mailto: was running just one time :(
- New: mailto: a report document is created.

## V5.59

- New: support mailto: as link with application/x-www-form-urlencoded coding. Variables "EMAIL=" and "SMTP_SERVER=" are used under STiK.

## V5.58

- New: HTTP header use "Referer:" field for **ALL** methods, thank to Pascal Ricard.

## V5.57

- New: HTTP header use "Referer:" field for POST method, so some forms run (tested with my homepage).
- Neu: deutschesprachige RSC-Datei und Anleitung, Dank an Harald Placke.

## V5.56

- Fixed: Memory crash in TCP buffer, thank to Bernd Maedicke.
- Fixed: Bug about cookie path, thank to Bernd Maedicke.

## V5.55

- New: Turbo mode is directly implemented, so you do not need to set it in config.
- New: In config SETUP a "use CAB" choice is added for let CAB to manage cookies.
- Fixed: Bad bug about cookie transmission between CAB and OVL.
- Fixed: No data comming indication for header in CAB info bar. So "receiving data xxxx/xxxx" is only for real datas. This way increase net speed is case of redirection.

## V5.54

- Near Fixed: No more 0 bytes/sec during download.
- New: The module check hotlist can be used for scanning changed documents.
- New: In config SETUP a turbo mode is added. Increase data flow for medium documents.

## V5.53

- Fixed: CAB.OVL crash if STinG is inactive. Thank to Bernd Maedicke.

## V5.52

- Fixed: Bad bug about cache, was erased in case of connect fail.
- New: Iconnect compatible.

## V5.50-51

- New: Speed about read datas optimized, you must (?) have a download speed fastest as old versions.
- New: Some very bad servers do not send HTTP "legal" header. In this case an alert is displayed, raw data loaded but you must contact the administrator for this protocol default.
- New: Limited alerts, so 'use form alert' is no more supported.
- New: In case of SERVER ERROR (not cab.ovl), a HTML document with original server's text is created and show, so 'debug button' is no more supported.
- Fixed: Bad bug about authentification: server with WWW-Authenticate header line, and without proxy. Thank you to Joel Houplon.

## V5.4

- New: Setup allow to be OFFLINE if you request a permanent network. Is this case a alert is show, and read cab cache is forced. Thank to Derryck Croker.

## V5.3

- New: Including '/internal-gopher' images for CAB version 2.51 and above (back, menu, binary, image, index, movie, sound, text and unknown).
- New: Setup allow to be OFFLINE if you request a permanent network. Is this case a alert is show, and read cab cache is forced. Thank to Derryck Croker.

## V5.2

- Fixed: Very bad bug with redirection after a POST method, so many formulars works now. Thank you to Bernd Maedicke.

## V5.1

- Fixed: Bug for logfile (not closed).
- Fixed: Bug for logfile, now one per cession.

## V5.0

- Fixed: Bug about if-modified, fixed now ? Ronald ??
- Fixed: In case of error or UserBreak, the incomplete file is removed for futurs reloads.

## V4.9

- Fixed: Bug about URL with space character (prohibited) is now coded ("%20") so no more file not found or server error.

## V4.8

- New: CAB.OVL do not send "if-modified" for date under 1971 ! passed by CAB, so request complete file is forced.
- Fixed: Some problems fixed with cookies and servers as [multimania](http://www.multimania.com/).

## V4.7

- New: CAB.OVL check CAB internal variable "reload", so "if-modified" is not send to the server if this flag is set.

## V4.6

- Fixed: Problem about file-not-modified (cache). Thank to Ronald Andersson.

## V4.5

- New: "Host : " is set in header some some servers. Thank to Bernd Maedicke.

## V4.4

- New: Refuse cookie in configuration page do not received OR SEND any cookie.

## V4.3

- Maybe Fixed: Bug in some servers return 'bad request', thank you to Bernd Maedicke.

## V4.2

- Fixed: Stupid bug for protected access (Authentification).
- Fixed: E_RRESET is no more an error, used for STinG Server<->Client due TCP_close() bug.

## V3.9-4.1

- Fixed: bad error form if no data is comming.
- Fixed: default button in alerts is no more supported.
- Fixed: no alerts if configuration is set as.

## V3.8

- New: decent dialog for cookie acceptation.

## V3.7

- Fixed: very bad bug on some formulares, thank you to Joel Houplon.

## V3.6

- New: ressource customization.

## V3.5

- New: ressource modules/cab_ovl.rsc for futurs translations.
- Fixed: some bugs about cookies.

## V3.4

- New: Debug informations are on new CAB window.

## V3.3

- Fixed: Urgent release, V3.2b destroy memory :(

## V3.2

- New: Try to call ST-GUIDE after write the debug.txt file.
- New: Window-dialog topped for CAB window topped.
- Fixed: Debug option re-writed (bombs).
- Fixed: Relative location allowed (eg: "Location: path/dummy/index.html").

## V3.1

- New: Dialbox in windows by Jacques Delavoix.
- New: Try to be RFC2109 compliant about cookie, but some servers are not :(
- Fixed: Crash in debug mode for long cookie value.

## V3.0

- New: Please, for version old 2.8 of fast cab.ovl, remove your module/cookie.set file.
- Fixed: in case of HTTP error, you can really retry.

## V2.9

- New: bell when dialbox.
- Fixed: cab crash for HTTP unkown error number.

---

Manual written with UDO, http://www.dirk-hagedorn.de/et/
