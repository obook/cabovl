# CAB.OVL

**HTTP module for the CAB web browser on Atari ST/TT/Falcon, over the STiK, STinG, IConnect and MiNT (GlueStik) TCP/IP stacks.**

This repository holds the original C sources of CAB.OVL V5.68 (January 19, 2000) and the release files.

Documentation: [manual](docs/MANUAL.md) and [version history](docs/HISTORY.md). In German: [Anleitung](docs/de/MANUAL.md) and [History](docs/de/HISTORY.md).

CAB.OVL was written by Olivier Booklage. The sources are covered by the GNU General Public License version 2 or later (see [License](#license)).

## What it does

CAB (Cristal Atari Browser) hands the network protocols to modules, called overlays, loaded from its `MODULES` folder. CAB.OVL is the module for HTTP. It opens the TCP connection, sends the request, reads the answer and gives the document back to CAB.

Supported:

- **HTTP requests**: GET, POST and HEAD, with the `Host`, `User-Agent`, `Accept` and `Referer` header fields.
- **Redirections**: absolute and relative `Location` values, a redirected POST becomes a GET, HEAD is kept for redirections (autosurf).
- **Cache control**: `If-Modified-Since` is sent unless CAB asks for a reload, or passes a date earlier than 1971, which means "load the full file".
- **Cookies**: stored in a file, with a dialog to accept or refuse each one (an attempt at RFC 2109). The setup page can refuse all cookies, or leave cookie handling to CAB.
- **Server authentication**: HTTP Basic, with a password dialog.
- **Proxies**: HTTP and FTP proxies, on every supported TCP/IP stack including IConnect.
- **mailto: links**: the module has its own SMTP client. The subject can follow the address after a "?". A report page is created for each message.
- **Gopher icons**: the `/internal-gopher` XBM images for CAB 2.51 and later.
- **Error pages**: when a server answers with an error, the module builds an HTML page that shows the server's own text.
- **Servers without a legal HTTP header**: the raw data is loaded and an alert tells you to contact the server administrator.
- **Offline mode**: the setup page can force CAB to read its cache instead of asking for a permanent network connection.
- **Hotlist check**: the module works with CAB's hotlist scan for changed documents.
- **Turbo mode**: a faster data path for medium documents.
- **Log file**: one log per session, shown in a CAB window.
- **Languages**: English and German resource files and documentation.

The setup page is `http://cab.ovl/setup.html`. Type that address in CAB to open it.

## TCP/IP stacks

| Stack | Notes |
|---|---|
| STiK | `STIK.ACC` must stay installed (it holds the TCP/IP routines even when you use another dialer) |
| STinG | same interface as STiK; client-server stack, first written by Peter Rottengatter, later maintained by Ronald Andersson |
| IConnect | socket calls, non-blocking resolver, name cache, stop button and Esc key supported |
| MiNT | MiNTNet through GlueStik |

At startup the module tries STiK (or STinG) first, then IConnect.

A 2005 comparison of these stacks, with MagiCNet, is on [atari-source.org](https://www.atari-source.org/articles/sting-versus-stik2-versus-iconnect-versus-mintnet-versus-magicnet/). It also mentions a separate cab.ovl made for MiNTNet that processes images and downloads in the background. That module is not this one.

## Installation (binary release)

1. Keep your old `CAB.OVL`.
2. Copy `CABOVL/CAB.OVL` and `CABOVL/CAB_OVL.RSC` to the `CAB/MODULES/` folder.
3. To get another language, use the `CAB_OVL.RSC` of the `ENGLISH` or `GERMAN` folder.
4. Restart CAB and open `http://cab.ovl` to set the module up.

CAB 1.5 is the browser the module is meant to be tested with.

## Release files (`CABOVL/`)

| Path | Content |
|---|---|
| `CAB.OVL` | the binary, V5.68, built with Pure C |
| `CAB_OVL.RSC`, `CAB_OVL.HRD` | GEM resource file and its header |
| `ENGLISH/` | resource file, manual (`CAB_OVL.*`) and history (`HISTORY.*`) in TXT, HTM and UDO source |
| `GERMAN/` | the same in German (`CAB_OVLG.*`, `HISTORYG.*`) |

The release was marked as a beta version for beta testers.

The manual and the history are also available as Markdown: [docs/MANUAL.md](docs/MANUAL.md) and [docs/HISTORY.md](docs/HISTORY.md), in German [docs/de/MANUAL.md](docs/de/MANUAL.md) and [docs/de/HISTORY.md](docs/de/HISTORY.md).

## Known problems (from the original manual)

- CAB 2.5x cannot send a request for a URL longer than 81 characters, so some forms cannot be sent correctly.
- Some servers that use cookies answer "bad request". Refusing cookies in the setup page avoids it.
- Many errors come from the network, the server or the CAB version. Before reporting a fault, test with other software and other versions of CAB.OVL. A report should include the log file and the versions of CAB.APP and CAB.OVL.

## Version history

Full history: `CABOVL/ENGLISH/HISTORY.TXT`. Main steps:

| Version | Change |
|---|---|
| 5.68 | fixes a crash when an HTTP proxy was set without ":" and a port |
| 5.67 | FTP fix when no HTTP proxy is set, faster |
| 5.63 | mailto subject, mailto under IConnect, German resource file and manual |
| 5.62 | authentication rewritten, internal non-blocking resolver, proxies and stop button under IConnect |
| 5.59 to 5.61 | mailto: support, `Referer` field, user-agent fix |
| 5.55 | turbo mode built in, option to leave cookies to CAB |
| 5.52 | IConnect support |
| 5.4 | offline mode |
| 5.3 | gopher icons |
| 4.4 | option to refuse cookies |
| 3.8 | cookie acceptance dialog |
| 3.6 | customizable resource file |
| 3.1 | dialogs in windows (windform library by Jacques Delavoix), cookies close to RFC 2109 |

## Sources (`src/`)

The files come from `STIKCOVL.ZIP` (August 2001), the Pure C project folder `I:\STIKCOVL`. `VERSION.H` gives V5.68 of January 19, 2000. Several `.C` files are dated January 22 to 25, 2000. Whether they match the released binary exactly is not verified.

Each file starts with a header: role, modules used, creation date, author, license. The original comment block is kept when it holds notes. The code is untouched.

Files are in the original Atari ST character set (accents included) with CRLF line endings. Convert them before reading them on a PC, for example `iconv -f cp437 -t utf-8`.

| File | Content |
|---|---|
| MAIN.C | module entry points for CAB (init_module, get_version, mailto) |
| HTTP_GET.C | HTTP request, reception, redirections |
| GET_URL.C, POST_URL.C, IF_MODIF.C, GET_INFO.C | the entry points get_url, post, get_url_if_modified, get_url_info |
| MY_HEAD.C, HEADER.C | request header, response header parsing |
| COOKIE.C | cookies (parsing, file, sending) |
| AUTHEN.C | HTTP authentication |
| CONFIG.C, SETUP.C | configuration, setup page http://cab.ovl |
| REPORT.C, RSC.C | error pages, GEM resource and dialogs |
| SENDICON.C | gopher XBM icons |
| MAILTO.C | mailto: links (SMTP) |
| TCP.C, CACHE_IP.C | TCP/IP stacks, name cache |
| TOOLS.C, GLOBALS.C | helpers, shared definitions |
| TCP/ | one file per TCP/IP stack: `STIK.C` (STiK, STinG), `ICONNECT.C` (IConnect), `MINTNET.C`, `DRACONIS.C` and `MODELE.C` (template) |
| ICONS/ | the gopher icons as C data, built into `ICONS.LIB` |
| STRISTR.S, FXDATIME.S | assembler routines |
| CABOVL.PRJ | Pure C project file |
| B64ENCOD.C, EXEC.C, ENV.C, LANGUAGE.C | not in the project file |

The headers the sources include are in [include/](include/README.md).

### Building

Not tested. `CABOVL.PRJ` lists the modules and the libraries. Several libraries are not in this repository: Pure C libraries, `sting.lib`, `logfile.lib`, NKCC, windform 3.00, IConnect and Draconis socket libraries. Some headers are missing too (see [include/README.md](include/README.md)).

## Origin and license

CAB.OVL descends from the STiK module published by Tim in 1996 (STIKCOVL, one single CABOVL.C file, with its README and LICENSE.TXT). That module reuses the example skeleton by Stephane Boisson (1995, MODULE.H and INIT.C). Both are licensed under the GNU GPL version 2 or later. CAB.OVL is a derivative work and is released under the same terms: see [LICENSE](LICENSE).

The software is provided without warranty and used at your own risk.

### Related

[Atarimania](https://www.atarimania.com/games/atari-st-utilities-cab-ovl-44809) lists an earlier CAB.OVL for STiK: version 1.2618, July 1997, credited to Dan Ackerman, freeware. It ships as two builds (68000 and 68030) and is documented there with its configuration variables and error codes. Its relation to the STiK module by Tim above is not verified.

## Credits

- Author: Olivier Booklage.
- Dialog windows: windform library by Jacques Delavoix.
- German resource file and manual: Harald Placke.
- Beta testers: Ronald Andersson, Jan Daldrup, Bernd Maedicke, the beazt, Pat Morales, Derryck Croker, Joel Houplon, Harald Placke, Pascal Ricard, Matthias Jaap, Ari Laine, Juergen Koneczny and John Banbury.
- Manual built with UDO (http://www.dirk-hagedorn.de/et/).
