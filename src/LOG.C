/*
 * LOG.C - journal de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions de
 * 0x1db76 à 0x1e2a8. Aucune trace ne donne les noms : ce sont des noms de
 * reconstitution. Le nom du module d'origine est inconnu.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "LOG.H"

FILE *log_file;                     /* 0x241f6 */
char log_tag[5] = "UNKN";           /* 0x241fa */
long log_mask;                      /* 0x24200 */
static char log_levels[] = "-~!?+:#*rw$&>/";    /* 0x24204 */

char log_name[0x80];                /* 0x2c9cc */
long log_size_kb;                   /* 0x2ca4c */
long log_flags;                     /* 0x2ca50 */
long log_start;                     /* 0x2cb54 */
char log_date[30];                  /* 0x2cb58 */

/* Renvoie le premier caractère de msg s'il est un niveau actif, sinon 0.
   L'espace est toujours actif. */
static char log_level(msg)
char *msg;
{
    char *p;

    if (msg[0] == 0)
        return 0;
    if (msg[0] == ' ')
        return ' ';
    p = strchr(log_levels, msg[0]);
    if (p == 0)
        return 0;
    if (log_mask & (1L << (p - log_levels)))
        return msg[0];
    return 0;
}

/* Ouvre le fichier en ajout, ou en écriture s'il n'existe pas. */
static int log_reopen(name)
char *name;
{
    if (log_file != 0)
        return 0;
    log_file = fopen(name, "a+");
    if (log_file == 0)
    {
        log_file = fopen(name, "w");
        if (log_file == 0)
            return 0;
    }
    fseek(log_file, 0L, SEEK_END);
    return 1;
}

/* Avant une écriture : en mode "rouvrir à chaque message", ouvre le fichier. */
static int log_begin()
{
    if (log_file == 0 && (log_flags & 0xf0) == 0x20)
        return log_reopen(log_name);
    return 0;
}

static int log_rotate();

/* Si la taille dépasse log_size_kb * 1000 octets, réduit le fichier. */
static int log_trim()
{
    long size;

    if (log_file != 0 && (log_flags & 0xf) == 4)
    {
        fseek(log_file, 0L, SEEK_END);
        size = ftell(log_file);
        if (size > log_size_kb * 1000L)
            return log_rotate();
    }
    return 0;
}

/* Après une écriture : réduit le fichier si besoin, le ferme en mode
   "rouvrir à chaque message". */
static int log_release()
{
    if (log_file == 0)
        return 0;
    if ((log_flags & 0xf0) == 0x20)
    {
        log_trim();
        fclose(log_file);
        log_file = 0;
        return 1;
    }
    log_trim();
    return 0;
}

/* Signale une erreur d'écriture sur la sortie d'erreur et dans le journal. */
static void log_disk_error(where, err)
char *where;
int err;
{
    fprintf(stderr, "\n\n** DISK WRITE ERROR <%d> on <%s> **\n", err, where);
    fprintf(log_file, "\n\n** DISK WRITE ERROR <%d> on <%s> **\n", err, where);
    fprintf(stderr, "\007");
}

/* Garde la seconde moitié du fichier : se place à 500 * log_size_kb octets
   de la fin, avance jusqu'à une fin de ligne, copie le reste dans
   logfile.tmp puis remplace le journal par cette copie. */
static int log_rotate()
{
    char path[0x80];
    char buf[0x100];
    char *p;
    FILE *tmp;
    long n;

    if (fseek(log_file, -500L * log_size_kb, SEEK_END) == -1)
    {
        fprintf(stderr, "LogFile:Fseek middle file <%s>\n", log_name);
        return 0;
    }
    do
    {
        n = (long)fread(buf, 1, 1, log_file);
    } while (buf[0] != '\n' && n > 0);

    path[0] = 0;
    strncat(path, log_name, 0x7f);
    p = strrchr(path, '\\');
    if (p != 0)
    {
        p[1] = 0;
        strcat(path, "logfile.tmp");
    }
    else
    {
        path[0] = 0;
        strncat(path, "logfile.tmp", 0x7f);
    }

    tmp = fopen(path, "w");
    if (tmp == 0)
    {
        fprintf(stderr, "LogFile:Unable to open temporary file <%s>\n", path);
        fseek(log_file, 0L, SEEK_END);
        return 0;
    }
    while ((n = (long)fread(buf, 1, 0xff, log_file)) > 0)
        fwrite(buf, 1, (size_t)n, tmp);

    fclose(log_file);
    fclose(tmp);
    log_file = 0;
    remove(log_name);
    rename(path, log_name);
    return log_begin();
}

/* Ouvre le journal. Renvoie 0 s'il est déjà ouvert ou s'il ne s'ouvre pas.
   size est en milliers d'octets, 20 si négatif ou nul. tag a 4 caractères. */
int log_open(file, tag, size, flags, level)
char *file;
char *tag;
long size;
long flags;
long level;
{
    log_size_kb = size;
    log_flags = flags;
    log_mask = level;
    log_name[0] = 0;
    strncat(log_name, file, 0x80);
    if (log_size_kb <= 0)
        log_size_kb = 20;

    log_tag[0] = 0;
    strncat(log_tag, tag, 4);
    strncat(log_tag, "     ", 4 - strlen(log_tag));

    if (log_file != 0)
        return 0;
    if ((flags & 0xf) == 1)
    {
        log_file = fopen(file, "w");
        if (log_file == 0)
            return 0;
    }
    else
    {
        log_file = fopen(file, "a+");
        if (log_file == 0)
        {
            log_file = fopen(file, "w");
            if (log_file == 0)
                return 0;
        }
        else
            fseek(log_file, 0L, SEEK_END);
    }
    log_start = (long)time(0);
    return 1;
}

/* Ferme le journal et remet l'état initial. */
int log_close()
{
    log_begin();
    if (log_file == 0)
        return 0;
    log_trim();
    fclose(log_file);
    log_file = 0;
    log_size_kb = 0;
    log_flags = 0;
    log_name[0] = 0;
    log_tag[0] = 0;
    strncat(log_tag, "UNKN", 4);
    log_mask = 0;
    return 1;
}

/* Écrit l'en-tête d'une ligne : niveau, date si demandée, étiquette. */
static int log_header(c)
char c;
{
    time_t t;
    int r;

    time(&t);
    strftime(log_date, 30, "%d %b %X", localtime(&t));
    if ((log_flags & 0xf00) == 0x800)
        r = fprintf(log_file, "%c %s %s", c, log_date, log_tag);
    else
        r = fprintf(log_file, "%c %s", c, log_tag);
    if (r < 0)
        log_disk_error("Logfile", r);
    return r;
}

/* Écrit une ligne dans le journal. Le premier caractère de msg est le niveau,
   le reste est un format printf. */
int trace(char *msg, ...)
{
    va_list ap;
    char c;
    int r = 0;

    c = log_level(msg);
    if (c == 0)
        return 0;
    log_begin();
    if (log_file == 0)
        return 0;

    log_header(c);
    va_start(ap, msg);
    r = vfprintf(log_file, msg + 1, ap);
    va_end(ap);
    if (r < 0)
        log_disk_error("Logfile", r);
    fflush(log_file);
    log_release();
    return r;
}

/* Ajoute s à la ligne en cours, si le niveau est actif. */
int trace_str(level, s)
char *level;
char *s;
{
    int r = 0;

    if (log_level(level) == 0)
        return 0;
    log_begin();
    if (log_file == 0)
        return 0;
    r = (int)fwrite(s, 1, strlen(s), log_file);
    if (r < 0)
        log_disk_error("Logfile", r);
    fflush(log_file);
    log_release();
    return r;
}

/* Comme trace, pour les messages d'erreur. La sortie d'erreur reçoit le
   message entier, le journal reçoit l'en-tête et le message sans son niveau.
   Un message dont le premier caractère n'est pas un niveau (comme
   "ERR_open_write_file") n'est écrit nulle part dans l'original. */
int trace_err(char *msg, ...)
{
    va_list ap;
    char c;
    int r = 0;

    fprintf(stderr, "\007");
    c = log_level(msg);
    if (c != 0)
    {
        va_start(ap, msg);
        vfprintf(stderr, msg, ap);
        va_end(ap);
    }
    log_begin();
    if (log_file == 0)
        return 0;
    c = log_level(msg);
    if (c != 0)
    {
        log_header(c);
        va_start(ap, msg);
        r = vfprintf(log_file, msg + 1, ap);
        va_end(ap);
        if (r < 0)
            log_disk_error("Logfile", r);
        fflush(log_file);
    }
    log_release();
    return r;
}

/* Écrit la ligne d'accueil. */
int log_welcome()
{
    int r;

    log_begin();
    if (log_file == 0)
        return 0;
    r = fprintf(log_file, "\n  Welcome to logfile\n");
    log_release();
    return r;
}

/* Écrit la durée depuis l'ouverture du journal. */
int log_end()
{
    int r;

    log_begin();
    r = trace("  Required: %lu seconds.\n\n", (unsigned long)time(0) - (unsigned long)log_start);
    log_release();
    return r;
}

/* Fixe le masque des niveaux actifs. */
int log_set_mask(mask)
long mask;
{
    log_mask = mask;
    return 1;
}

/* Renvoie 1 si le journal est ouvert, ou s'il se rouvre à chaque message. */
int log_is_open()
{
    return log_file != 0 || (log_flags & 0xf0) == 0x20;
}
