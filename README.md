# CAB.OVL

**HTTP module for the CAB web browser on Atari ST/TT/Falcon, over the STiK, STinG, IConnect and MiNT (GlueStik) TCP/IP stacks.**

This repository holds the C sources of CAB.OVL V5.68 (January 19, 2000), reconstructed from the original binary, and the original release files.

Documentation: [manual](docs/MANUAL.md) and [version history](docs/HISTORY.md).

CAB.OVL was written by Olivier Booklage. The reconstructed sources are covered by the GNU General Public License version 2 or later (see [License](#license)).

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

CAB 1.5 is the browser the reconstructed module is meant to be tested with.

## Release files (`CABOVL/`)

| Path | Content |
|---|---|
| `CAB.OVL` | the binary, V5.68, built with Pure C |
| `CAB_OVL.RSC`, `CAB_OVL.HRD` | GEM resource file and its header |
| `ENGLISH/` | resource file, manual (`CAB_OVL.*`) and history (`HISTORY.*`) in TXT, HTM and UDO source |
| `GERMAN/` | the same in German (`CAB_OVLG.*`, `HISTORYG.*`) |

The release was marked as a beta version for beta testers.

The manual and the history are also available as Markdown: [docs/MANUAL.md](docs/MANUAL.md) and [docs/HISTORY.md](docs/HISTORY.md).

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

## Reconstructed sources (`src/`)

The original sources (`I:\STIKCOVL\*.C`) are not in the archive. These files are a reconstruction: they are equivalent to the binary, not identical to the original. None has been built with Pure C yet. Each one passes `gcc -x c -std=gnu89 -fsyntax-only`.

| File | Content |
|---|---|
| OVL.C | entry point (ovl_entry), file paths, log, TCP stack |
| HTTP_GET.C | HTTP request, reception, redirections, get_url and post wrappers |
| HEADER.C | response header parsing |
| COOKIE.C | cookies (parsing, file, sending) |
| CONFIG.C | configuration file (22 bytes) |
| SETUP.C | the page http://cab.ovl/setup.html and its settings |
| SENDICON.C, ICONS.C | gopher XBM icons (identical to the binary) |
| REPORT.C | error and report pages |
| GUI.C | GEM resources, cookie, password and alert dialogs |
| MAILTO.C | mailto: links (SMTP client, base64) |
| TCP.C | TCP stack: STiK, STinG, IConnect, name cache |
| URL.C | URL parsing and proxies |
| LOG.C | log and trace function |
| FILEDBG.C | file trace wrappers, version formatters |
| UTIL.C | skip_blanks, trim_token, parse_date |

Headers: CABOVL.H, CONFIG.H, REQUEST.H, HEADER.H, LOG.H, TCP.H, GUI.H, REPORT.H and OVLINFO.H (module identification strings, with the e-mail address and site replaced by xxx).

### Not reconstructed (libraries to link)

- Form window library (wf_*, addresses 0x1e33c to 0x202b9): probably windform 3.0 (name to be confirmed).
- AES, VDI and GEMDOS bindings, Pure C standard library, floating point support.
- Memory checking (NewMemBlk, FreeMemBlk, mem_check).
- IConnect calls (ic_*) and STiK initialization (stik_lib_init).

### Notes on the reconstruction

- Function names are reliable when they come from a trace string "> Name". The others are reconstruction names (marked in a comment).
- Binary defects are reproduced as is and flagged in a comment (two-digit year in dash-separated dates, buffers without size checks, and so on).
- Unverified values: the `opt14` field of CONFIG, the offsets of the STiK structures, the code 0x3039 of cab->progress.
- Interface with CAB: CABOVL.H follows the official module interface (MODULE.H and INIT.C from the CAB 1.5 developer documentation). ovl_entry returns SUPPORT_PEXEC, SUPPORT_HTTP and SUPPORT_MAILTO (0x23).
- Source comments are in French.

### Building

Not done yet. A Pure C project file (`.PRJ`) is still to be written. It needs the `.C` files above and the Pure C standard, floating point, TOS and GEM libraries (not verified). A syntax check on a PC:

    gcc -x c -std=gnu89 -fsyntax-only src/*.C

## Origin and license

CAB.OVL descends from the STiK module published by Tim in 1996 (STIKCOVL, one single CABOVL.C file, with its README and LICENSE.TXT). That module reuses the example skeleton by Stephane Boisson (1995, MODULE.H and INIT.C). Both are licensed under the GNU GPL version 2 or later. This reconstruction is a derivative work and is released under the same terms: see [LICENSE](LICENSE).

The functions parse_date, parse_header, clear_info, split_URL, http_proxy_cfg, ftp_proxy_cfg and open_connection of CABOVL.C have the same names as those reconstructed here. parse_date is identical to the binary's, including the two-digit year defect.

The software is provided without warranty and used at your own risk.

### Related

[Atarimania](https://www.atarimania.com/games/atari-st-utilities-cab-ovl-44809) lists an earlier CAB.OVL for STiK: version 1.2618, July 1997, credited to Dan Ackerman, freeware. It ships as two builds (68000 and 68030) and is documented there with its configuration variables and error codes. Its relation to the STiK module by Tim above is not verified.

## Credits

- Author: Olivier Booklage.
- Dialog windows: windform library by Jacques Delavoix.
- German resource file and manual: Harald Placke.
- Beta testers: Ronald Andersson, Jan Daldrup, Bernd Maedicke, the beazt, Pat Morales, Derryck Croker, Joel Houplon, Harald Placke, Pascal Ricard, Matthias Jaap, Ari Laine, Juergen Koneczny and John Banbury.
- Manual built with UDO (http://www.dirk-hagedorn.de/et/).
