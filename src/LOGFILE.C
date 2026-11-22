/*
 * logfile.c
 * Bibliothäque de journal (logfile) : LOG_open, LOG_printf, ALL_printf, ...
 *
 * SOURCE RECONSTITUê Ö partir de logfile.lib (3126 octets, fÇvrier 1997).
 * Le source d'origine est perdu. Le code suit celui du binaire fonction
 * par fonction. Il n'a pas ÇtÇ compilÇ avec Pure C. Seul un essai sur PC
 * a ÇtÇ fait, avec des fonctions GEMDOS simulÇes.
 *
 * Fonctions exportÇes : LOG_open, LOG_close, LOG_bye, LOG_welcome, LOG_lf,
 * STD_printf, ALL_printf, LOG_printf, ERR_printf (voir logfile.h).
 *
 * Un message commence par un caractäre de niveau ("- coucou", "# coucou").
 * Il est Çcrit dans le journal si ce niveau est actif dans debug_level.
 * Un message qui commence par une espace est toujours Çcrit.
 *
 * Utilise : les fonctions GEMDOS Fopen, Fcreate, Fseek, Fread, Fwrite,
 * Fclose et la bibliothäque standard de Pure C.
 *
 * Auteur : Olivier Booklage
 * Date : 1997, source reconstituÇ en novembre 2026
 * Licence : GNU General Public License version 2 ou ultÇrieure
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <tos.h>
#include <portab.h>

#include "logfile.h"

/* Taille de la file en blocs de 1000 octets : valeur par dÇfaut */
#define DEFAULT_MAX_SIZE 20L

/* Une ligne de journal est coupÇe Ö cette taille */
#define LINE_SIZE 256

/* Longueur du nom de la session dans le journal */
#define NAME_LENGTH 4

/* Valeurs de Fseek */
#define SEEK_FROM_END 2

/* Octets lus ou Çcrits Ö la fois lors de la rÇduction du journal */
#define COPY_SIZE 255L

/* Les caractäres de niveau, dans l'ordre de DEBUG_0 Ö DEBUG_13 */
static const char debug_chars[] = "-~!?+:#*rw$&>/";

/* ---       Variables du journal       --- */

static WORD log_handle = -1;           /* -1 : journal fermÇ */
static char log_name[NAME_LENGTH + 2] = "UNKN";
static LONG log_debug_level = 0;
static char log_path[128];
static LONG log_max_size;              /* en blocs de 1000 octets */
static LONG log_mode;
static char log_line[LINE_SIZE];
static LONG log_start_time;
static char log_stamp[20];

/* ---       Fonctions internes       --- */

static BOOLEAN open_log(UBYTE *path);
static BOOLEAN reopen_if_discret(VOID);
static BOOLEAN close_if_discret(VOID);
static UBYTE level_of_message(UBYTE *message);
static BOOLEAN check_size(VOID);
static BOOLEAN shrink_log(VOID);
static VOID write_error(const char *what);
static VOID read_error(const char *what);
static VOID console_lf(VOID);
static WORD write_message(UBYTE *fmt, va_list args);

/* Rouvre le fichier du journal en ajout (mode LOG_DISCRET). */
static BOOLEAN open_log(UBYTE *path) {
    if (log_handle >= 0) {
        return FALSE;
    }
    log_handle = (WORD)Fopen(path, 2);
    if (log_handle < 0) {
        return FALSE;
    }
    Fseek(0L, log_handle, SEEK_FROM_END);
    return TRUE;
}

/* Rouvre le journal s'il est en mode LOG_DISCRET et fermÇ. */
static BOOLEAN reopen_if_discret(VOID) {
    if (log_handle < 0 && (log_mode & 0xf0) == LOG_DISCRET) {
        return open_log(log_path);
    }
    return FALSE;
}

/* Referme le journal s'il est en mode LOG_DISCRET et ouvert. */
static BOOLEAN close_if_discret(VOID) {
    if (log_handle >= 0 && (log_mode & 0xf0) == LOG_DISCRET) {
        return LOG_close();
    }
    return FALSE;
}

/*
 * Renvoie le caractäre de niveau du message s'il doit àtre Çcrit,
 * sinon 0. Les valeurs ' ' et 0x1f sont toujours acceptÇes, comme
 * dans le binaire.
 */
static UBYTE level_of_message(UBYTE *message) {
    UBYTE level = message[0];
    char *found;
    LONG bit;

    if (level == 0) {
        return 0;
    }
    if (level == ' ' || level == 0x1f) {
        return ' ';
    }
    found = strchr(debug_chars, level);
    if (found == NULL) {
        return 0;
    }
    bit = 1L << (found - debug_chars);
    if ((log_debug_level & bit) == 0) {
        return 0;
    }
    return *found;
}

/* RÇduit le journal quand il dÇpasse la taille maximale (LOG_SIZED). */
static BOOLEAN check_size(VOID) {
    LONG size;

    if (log_handle < 0) {
        return FALSE;
    }
    if ((log_mode & 0x0f) == LOG_SIZED) {
        size = Fseek(0L, log_handle, SEEK_FROM_END);
        if (size / 1000L + 1 > log_max_size) {
            shrink_log();
        }
    }
    return TRUE;
}

/*
 * Garde la fin du journal (500 octets par bloc de la taille maximale),
 * en repartant d'un dÇbut de ligne. êcrit un fichier temporaire Ö cìtÇ
 * du journal, puis le renomme.
 */
static BOOLEAN shrink_log(VOID) {
    char temp_path[128];
    char buffer[LINE_SIZE];
    char *last_slash;
    LONG count;
    LONG position;
    WORD temp_handle;

    position = Fseek(-(log_max_size * 500L), log_handle, SEEK_FROM_END);
    if (position == -1L) {
        return FALSE;
    }

    /* Avance jusqu'Ö la fin de la ligne courante */
    do {
        count = Fread(log_handle, 1L, buffer);
    } while (buffer[0] != '\n' && count > 0);

    strncpy(temp_path, log_path, 127);
    last_slash = strrchr(temp_path, '\\');
    if (last_slash != NULL) {
        last_slash[1] = 0;
        strcat(temp_path, "logfile.tmp");
    } else {
        strcpy(temp_path, "logfile.tmp");
    }

    temp_handle = (WORD)Fcreate(temp_path, 0);
    if (temp_handle < 0) {
        return FALSE;
    }

    count = Fread(log_handle, COPY_SIZE, buffer);
    while (count > 0) {
        Fwrite(temp_handle, count, buffer);
        count = Fread(log_handle, COPY_SIZE, buffer);
    }

    Fclose(log_handle);
    Fclose(temp_handle);
    log_handle = -1;
    remove(log_path);
    rename(temp_path, log_path);

    /* Le nom reáoit le chemin, comme dans le binaire */
    return LOG_open(log_path, log_max_size, log_mode, log_path,
                    log_debug_level);
}

/* Signale une erreur d'Çcriture sur la console. */
static VOID write_error(const char *what) {
    fprintf(stderr, "\n\n** DISK WRITE ERROR on %s **\n", what);
    console_lf();
}

/* Signale une erreur de lecture. Aucune fonction ne l'appelle. */
static VOID read_error(const char *what) {
    fprintf(stderr, "\n\n** DISK READ ERROR on %s **\n", what);
    console_lf();
}

/* Saut de ligne sur la sortie standard. */
static VOID console_lf(VOID) {
    fprintf(stdout, "\n");
}

/*
 * êcrit le message dans le journal, prÇcÇdÇ du caractäre de niveau,
 * de la date, de l'heure et du nom de session.
 * Le premier caractäre du message (le niveau) n'est pas rÇpÇtÇ.
 */
static WORD write_message(UBYTE *fmt, va_list args) {
    UBYTE level;
    time_t now;
    LONG written = 0;

    level = level_of_message(fmt);
    if (level != 0) {
        now = time(NULL);
        strftime(log_stamp, 20, "%d %b %X", localtime(&now));
        sprintf(log_line, "%c %s %s", level, log_stamp, log_name);
        written = Fwrite(log_handle, (LONG)strlen(log_line), log_line);
        vsprintf(log_line, fmt, args);
        written = Fwrite(log_handle, (LONG)strlen(log_line + 1),
                         log_line + 1);
    }
    if (written == -1L) {
        write_error("Logfile");
    }
    close_if_discret();
    return (WORD)written;
}

/* ---       Fonctions exportÇes       --- */

/*
 * Ouvre le journal.
 * Renvoie FALSE si le journal est dÇjÖ ouvert ou si le fichier ne
 * peut pas àtre ouvert.
 */
BOOLEAN LOG_open(UBYTE *file, long max_size, LONG mode, UBYTE *name,
                 LONG debug_level) {
    if (log_handle >= 0) {
        return FALSE;
    }

    log_max_size = max_size;
    log_mode = mode;
    strcpy(log_path, file);
    if (log_max_size <= 0) {
        log_max_size = DEFAULT_MAX_SIZE;
    }

    /* Le nom est complÇtÇ par des espaces jusqu'Ö 4 caractäres */
    strncpy(log_name, name, NAME_LENGTH);
    strncat(log_name, "    ", NAME_LENGTH - strlen(log_name));
    log_debug_level = debug_level;

    if ((log_mode & 0x0f) != 0) {
        log_handle = (WORD)Fopen(file, 2);
        if (log_handle >= 0) {
            Fseek(0L, log_handle, SEEK_FROM_END);
        }
    }
    if (log_handle < 0) {
        log_handle = (WORD)Fcreate(file, 0);
    }
    if (log_handle < 0) {
        return FALSE;
    }

    log_start_time = time(NULL);
    return TRUE;
}

/* êcrit un saut de ligne dans le journal. */
WORD LOG_lf(VOID) {
    LONG written;

    reopen_if_discret();
    if (log_handle < 0) {
        return 0;
    }
    strcpy(log_line, "\n");
    written = Fwrite(log_handle, (LONG)strlen(log_line), log_line);
    close_if_discret();
    return (WORD)written;
}

/* êcrit le message de dÇbut de session. */
BOOLEAN LOG_welcome(VOID) {
    LONG written;

    reopen_if_discret();
    if (log_handle < 0) {
        return FALSE;
    }
    strcpy(log_line, "\n");
    written = Fwrite(log_handle, (LONG)strlen(log_line), log_line);
    LOG_printf("  Welcome to logfile\n");
    close_if_discret();
    return (BOOLEAN)written;
}

/* êcrit le temps ÇcoulÇ depuis LOG_open. */
WORD LOG_bye(VOID) {
    WORD result;

    reopen_if_discret();
    if (log_handle < 0) {
        return 0;
    }
    result = LOG_printf("  Required: %lu seconds.\n\n",
                        (unsigned long)(time(NULL) - log_start_time));
    close_if_discret();
    return result;
}

/* Ferme le journal. */
WORD LOG_close(VOID) {
    reopen_if_discret();
    if (log_handle < 0) {
        return FALSE;
    }
    check_size();
    Fclose(log_handle);
    log_handle = -1;
    return TRUE;
}

/* êcrit sur la sortie standard seulement. */
WORD STD_printf(UBYTE *fmt, ...) {
    va_list args;
    WORD result;

    va_start(args, fmt);
    result = (WORD)vfprintf(stdout, fmt, args);
    va_end(args);
    return result;
}

/*
 * êcrit sur la sortie standard et dans le journal.
 * Les arguments sont relus pour le journal : sous Pure C, va_list est
 * un simple pointeur sur la pile.
 */
WORD ALL_printf(UBYTE *fmt, ...) {
    va_list args;
    WORD result;

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    reopen_if_discret();
    if (log_handle < 0) {
        va_end(args);
        return 0;
    }
    result = write_message(fmt, args);
    va_end(args);
    return result;
}

/* êcrit dans le journal seulement. */
WORD LOG_printf(UBYTE *fmt, ...) {
    va_list args;
    WORD result;

    reopen_if_discret();
    if (log_handle < 0) {
        return 0;
    }
    va_start(args, fmt);
    result = write_message(fmt, args);
    va_end(args);
    return result;
}

/* êcrit sur la sortie d'erreur et dans le journal. */
WORD ERR_printf(UBYTE *fmt, ...) {
    va_list args;
    WORD result;

    console_lf();
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    reopen_if_discret();
    if (log_handle < 0) {
        va_end(args);
        return 0;
    }
    result = write_message(fmt, args);
    va_end(args);
    return result;
}
