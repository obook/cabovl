# CAB.OVL History

**HTTP-Modul für CAB unter STinG/STiK/Iconnect/MintNet**

Von Olivier Booklage. Release V5.68. Anleitung: [MANUAL.md](MANUAL.md). English version, complete: [../HISTORY.md](../HISTORY.md).

Dies ist eine Markdown-Umsetzung der UDO-Datei `CABOVL/GERMAN/HISTORYG.U`. Der Wortlaut ist der des Übersetzers. Die Einträge bis V5.63 sind im Original englisch, ab V5.62 deutsch. Angaben zu älteren Versionen (vor V5.55) stehen im englischen Text. Die HTML-Testformulare und E-Mail-Adressen wurden weggelassen.

## V5.68

- Fixed: Very old bug, cab.ovl crash if a HTTP proxy was defined without semi-colon ":" followed by the port value.

## V5.67

- Fixed: FTP bug if no HTTP proxy.
- New: Speed increased.

Die Quelle nennt diesen Abschnitt "V5.57", ein Tippfehler.

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

- Neu: Autorisierung neu programmiert.
- Neu: Kein Alert mehr für vergessenen HTTP-Header. Dank an Bernd Maedicke.
- Neu: Interner "Versuch" eines nicht blockierenden Resolvers.
- Neu: Datengeschwindigkeit im Turbo-Modus unter IConnect.
- Neu: HTTP- und FTP-Proxies können unter IConnect benutzt werden.
- Neu: Stop-Icon und Esc-Taste können unter IConnect benutzt werden. Dank an Ari Laine.
- Bug: mailto kann unter IConnect nicht benutzt werden, aber läßt CAB nicht abstürzen. Dank an Harald Placke.

## V5.61

- Fix: mailto funktionierte nur, wenn ftp_proxy konfiguriert war!! Dank an Joel Houplon.
- Fix: Bad user-agent. Dank an Matthias Jaap.

## V5.60

- Fix: mailto: funktionierte immer nur gerade ein mal :(
- New: mailto: ein Report-Dokument wird erstellt.

## V5.59

- Neu: Unterstützung von mailto: als Link mit application/x-www-form-urlencoded Kodierung. Dafür müssen unter STinG in der DEFAULT.CFG die Variablen EMAIL und SMTP_SERVER definiert sein.

## V5.58

- Neu: HTTP-Header benutzt "Referer:" Feld für **ALLE** Methoden, Dank an Pascal Ricard.

## V5.57

- Neu: HTTP-Header benutzt "Referer:" Feld für POST Methode, dadurch funktionieren einige Formulare (Getestet mit meiner Homepage).
- Neu: deutschesprachige RSC-Datei und Anleitung, Dank an Harald Placke.

## V5.56

- FIX: Speicherfehler im TCP-Puffer, Dank an Bernd Maedicke.
- FIX: Fehler beim Cookie-Pfad, Dank an Bernd Maedicke.

## V5.55

- Neu: Turbo-Modus ist direkt eingebaut, eine Konfiguration entfällt.
- Neu: In der Konfiguration (SETUP) ist ein Schalter "Verwaltung durch CAB" hinzugekommen, der CAB veranlaßt die Cookies selbst zu verwalten.
- Fix: Schwerer Fehler in der Cookiebehandlung zwischen CAB und OVL.
- Fix: Keine Anzeige über eintreffende Daten in der CAB Infozeile. "receiving data xxxx/xxxx" ist nur für reale Daten. Dies erhöht die Net-Geschwindigkeit im Fall einer Umlenkung.

Angaben zu älteren Versionen sind dem englischen Text zu entnehmen.

---

Anleitung geschrieben mit UDO, http://www.dirk-hagedorn.de/et/
