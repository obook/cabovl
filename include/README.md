# include/

Headers that `src/` needs, copied unchanged.

| File | Origin | License |
|---|---|---|
| `cab/MODULE.H` | CAB 1.5 developer documentation, by Stephane Boisson (1995) | GPL, stated in the file |
| `sting/TRANSPRT.H` | STinG sources (Peter Rottengatter, revised by Ronald Andersson, Miro Kropacek and Thorsten Otto), from [th-otto/STinG](https://github.com/th-otto/STinG) | not stated in that repository, to be confirmed |
| `sting/STING.H` | header of `sting.lib`, by Olivier Booklage | same as this repository |
| `LOGFILE.H` | header of the log library, by Olivier Booklage (1996), from `UGATE.LZH`. The library is in [lib/](../lib/README.md), its source in `src/LOGFILE.C`. | same as this repository |
| `upoint/MEMORY.H` | memory control, by Olivier Booklage, from `INET.LZH` | same as this repository |

The files keep their Atari ST character set and CRLF line endings.

## Still missing

`CABOVL.PRJ` also needs:

- Headers: `portab.h` (a copy is in the `lib_com2` archive), `upoint\struct.h`, `nkcc\nkcc.h`, `yield\yield.h` (Ronald Andersson), the IConnect socket headers (`socklib` archive).
- Libraries: Pure C libraries, `sting.lib`, NKCC, windform 3.00, IConnect `socklib.lib`, Draconis `stsocket.lib`.
