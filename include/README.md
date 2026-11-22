# include/

Headers that `src/` needs, copied unchanged.

| File | Origin | License |
|---|---|---|
| `cab/MODULE.H` | CAB 1.5 developer documentation, by Stephane Boisson (1995) | GPL, stated in the file |
| `sting/TRANSPRT.H` | STinG sources (Peter Rottengatter, revised by Ronald Andersson, Miro Kropacek and Thorsten Otto), from [th-otto/STinG](https://github.com/th-otto/STinG) | not stated in that repository, to be confirmed |
| `sting/STING.H` | header of `sting.lib`, by Olivier Booklage | same as this repository |
| `LOGFILE.H` | header of the log library, by Olivier Booklage (1996), from `UGATE.LZH`. The library is in [lib/](../lib/README.md), its source in `src/LOGFILE.C`. | same as this repository |
| `upoint/MEMORY.H` | memory control, by Olivier Booklage, from `INET.LZH` | same as this repository |
| `PORTAB.H` | portability header, from the `lib_com2` archive (April 1993). The `UGATE.LZH` copy (September 1993) differs. Author not stated in the file. | checked by the repository owner |
| `upoint/STRUCT.H` | shared types (comments in German), from `UGATE.LZH`. Probably from UPoint. | checked by the repository owner |

The files keep their Atari ST character set and CRLF line endings.

## Still missing

`CABOVL.PRJ` also needs:

- Headers: `nkcc\nkcc.h`, `yield\yield.h` (Ronald Andersson), the IConnect socket headers (`socklib` archive).
- Libraries: Pure C libraries, `sting.lib`, NKCC, windform 3.00, IConnect `socklib.lib`, Draconis `stsocket.lib`.
