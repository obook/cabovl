# CAB.OVL

**HTTP-Modul für CAB unter STinG/STiK/Iconnect/MintNet**

Von Olivier Booklage. Release V5.68. Versionsgeschichte: [HISTORY.md](HISTORY.md). English version: [../MANUAL.md](../MANUAL.md).

Dies ist eine Markdown-Umsetzung der deutschen UDO-Anleitung `CABOVL/GERMAN/CAB_OVLG.U`. Der Wortlaut ist der des Übersetzers. E-Mail-Adressen und die Postadresse wurden weggelassen.

## Inhalt

1. [Über CAB.OVL](#über-cabovl)
2. [Installation](#installation)
3. [Konfiguration](#konfiguration)
4. [Haftung](#haftung)
5. [Fehlerbericht](#fehlerbericht)
6. [History](#history)
7. [Autor](#autor)
8. [Beta-Tester](#beta-tester)

## Über CAB.OVL

CAB.OVL ist ein Modul für Cristal Atari Browser 'CAB' für HTTP-Protokoll unter TCP/IP-Stack [STinG](http://www.rottengatter.de), STiK, IConnect oder MintNet mit GlueStik.

Unterstützung von Server-Autorisierung und Cookies auch bei CAB < 2.7.

Für STiK-Benutzer: STIK.ACC enthält essentielle TCP/IP-Routinen außerhalb des eingebauten Dialers, darum muß es resident gehalten werden!

**Dieses ist eine BETA-VERSION nur für Beta-Tester.**

## Installation

Sichern Sie Ihr altes CAB.OVL und legen Sie das Neue dann zusammen mit dem CAB_OVL.RSC in Ihren Ordner CAB/MODULES/

Wenn Sie das deutschsprachige CAB_OVL.RSC aus dem Ordner GERMAN benutzen wollen, so legen Sie dieses anstatt des englischsprachigen CAB_OVL.RSC in den Ordner CAB/MODULES/.

## Konfiguration

Rufen Sie http://cab.ovl in CAB auf, um CABOVL zu konfigurieren.

## Haftung

Vom Autor wurde alles Mögliche für einen problemlosen Betrieb unternommen. Eine Haftung für mittelbare oder unmittelbare Schäden kann nicht übernommen werden. Die Benutzung dieser Software geschieht auf eigenes Risiko.

Viel Vergnügen damit.

## Fehlerbericht

- CAB 2.5X kann keine URL > 81 Zeichen anfordern, einige Formulare können mit dieser Version nicht korrekt versandt werden.
- Bei manchen Servern mit Cookies wird eine Fehlermeldung 'bad request' zurückgeschickt. Als Notlösung können Sie in der Konfiguration 'Cookie ablehnen' einstellen, wenn das Problem hartnäckig ist.
- Viele Fehler kommen vom Netzwerk, Servern oder verschiedenen CAB-Versionen, bevor Sie mir also schreiben (siehe Autor), **VERGEWISSERN SIE SICH, DASS CAB.OVL SCHULD AN DER FEHLFUNKTION IST** durch Tests auf anderen Plattformen und mit anderen CAB.OVLs. Senden Sie mir dann bitte das CAB.OVL-Logfile und geben Sie die verwendeten Versionen von CAB und von CAB.OVL an.

## History

Siehe [HISTORY.md](HISTORY.md).

## Autor

Olivier Booklage
http://obooklage.free.fr/

## Beta-Tester

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
- John Banbury

---

Anleitung geschrieben mit UDO, http://www.dirk-hagedorn.de/et/
