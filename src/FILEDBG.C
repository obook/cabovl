/*
 * FILEDBG.C - enveloppes de trace des fichiers et formateurs de version
 * de CAB.OVL (V5.68)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C).
 * Dans le binaire, chaque appel Fcreate, Fclose, Fdelete, fopen, fclose et
 * remove du module passe par une enveloppe qui reçoit __FILE__ et __LINE__.
 * Ici les enveloppes portent le préfixe dbg_ pour ne pas masquer les
 * fonctions standard. Les sources appellent les fonctions standard.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include "LOG.H"

/* Appels GEMDOS (bibliothèque Pure C). */
extern long Fcreate(char *name, short attr);    /* 0x1be38 */
extern short Fclose(short handle);              /* 0x1be06 */
extern short Fdelete(char *name);               /* 0x1be28 */

/* Descripteur du fichier GEMDOS dans une structure FILE de Pure C : mot en
   décalage 0x10. Non vérifié contre stdio.h de Pure C. */
#define FILE_HANDLE(fp) (*(short *)((char *)(fp) + 0x10))

/* 0x15ea6, enveloppe de Fcreate. Renvoie le descripteur ou un code d'erreur. */
long dbg_Fcreate(name, file, attr, line)
char *name;
char *file;
short attr;
long line;
{
    long r;

    r = Fcreate(name, attr);
    trace("> Fcreate %s %ld <%s> = <%ld>\n", file, line, name, r);
    return r;
}

/* 0x15eda, enveloppe de Fclose. Défaut : si handle <= 0, Fclose n'est pas
   appelé et handle est renvoyé tel quel. */
short dbg_Fclose(file, handle, line)
char *file;
short handle;
long line;
{
    short r;

    r = handle;
    trace("> Fclose %s %ld <%d>\n", file, line, handle);
    if (handle > 0)
        r = Fclose(handle);
    return r;
}

/* 0x15f10, enveloppe de Fdelete.
   Défaut : le résultat (16 bits) est empilé pour un format %ld. */
short dbg_Fdelete(name, file, line)
char *name;
char *file;
long line;
{
    short r;

    r = Fdelete(name);
    trace("> Fdelete %s %ld <%s> = <%ld>\n", file, line, name, r);
    return r;
}

/* 0x15f42, enveloppe de fopen. Les registres sont les mêmes quel que soit
   l'ordre de file et line dans la déclaration. */
FILE *dbg_fopen(name, mode, file, line)
char *name;
char *mode;
char *file;
long line;
{
    FILE *fp;

    fp = fopen(name, mode);
    if (fp != NULL)
        trace("> fopen %s %ld <%s> = <%d>\n", file, line, name, FILE_HANDLE(fp));
    else
        trace("> fopen %s %ld <%s> = NULL\n", file, line, name);
    return fp;
}

/* 0x15f96, enveloppe de fclose. Renvoie -1 si fp est NULL.
   Défaut : le descripteur est lu dans fp après fclose. */
short dbg_fclose(fp, file, line)
FILE *fp;
char *file;
long line;
{
    short r;

    r = -1;
    if (fp != NULL)
    {
        r = fclose(fp);
        trace("> fclose %s %ld <%d> = <%d>\n", file, line, FILE_HANDLE(fp), r);
    }
    else
        trace("> fclose %s %ld NULL\n", file, line);
    return r;
}

/* 0x15fe6, enveloppe de remove.
   Défaut : le résultat (16 bits) est empilé pour un format %ld. */
short dbg_remove(name, file, line)
char *name;
char *file;
long line;
{
    short r;

    r = remove(name);
    trace("> remove %s %ld <%s> = <%ld>\n", file, line, name, r);
    return r;
}

/* Tampons statiques des formateurs de version (0x2b4d0 et 0x2b551). */
static char cab_version_buf[0x81];
static char ovl_version_buf[0x81];

/* 0x16018, version de CAB, 4 caractères ASCII : "0270" donne "2.70",
   "1050" donne "10.50". */
char *format_cab_version(v)
long v;
{
    unsigned char b3 = (unsigned char)(v >> 24);
    unsigned char b2 = (unsigned char)(v >> 16);
    unsigned char b1 = (unsigned char)(v >> 8);
    unsigned char b0 = (unsigned char)v;

    if (b3 != '0')
        sprintf(cab_version_buf, "%c%c.%c%c", b3, b2, b1, b0);
    else
        sprintf(cab_version_buf, "%c.%c%c", b2, b1, b0);
    return cab_version_buf;
}

/* 0x1609e, version du module : 0x56800 donne "5.68".
   Octet haut non nul : "%lx%lx.%02lx" sur les octets 3, 2 et 1. */
char *format_ovl_version(v)
long v;
{
    if (((v >> 24) & 0xff) != 0)
        sprintf(ovl_version_buf, "%lx%lx.%02lx",
                (v >> 24) & 0xffL, (v >> 16) & 0xffL, (v >> 8) & 0xffL);
    else
        sprintf(ovl_version_buf, "%lx.%02lx",
                (v >> 16) & 0xffL, (v >> 8) & 0xffL);
    return ovl_version_buf;
}
