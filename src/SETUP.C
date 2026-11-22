/*
 * SETUP.C - page de configuration http://cab.ovl/setup.html (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions de
 * 0x140a6 à 0x14674. SendSetup et regle_setup sont les noms des chaînes de
 * trace, les autres noms sont des noms de reconstitution.
 * La page HTML est un modèle de lignes de texte dans CAB_OVL.RSC, arbre 2
 * pour le formulaire, arbre 3 pour la réponse. rsc_next_line renvoie la
 * ligne suivante de l'arbre avec son numéro, qui sert de sélecteur ici.
 * Non compilé avec Pure C, à vérifier.
 *
 * OVLINFO.H définit OVL_ADDRESS, la ligne de signature (lue dans le binaire
 * en 0x21e32).
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "CONFIG.H"
#include "CABOVL.H"
#include "REQUEST.H"
#include "OVLINFO.H"
#include "LOG.H"

extern long Fcreate(char *name, int attr);          /* GEMDOS */
extern int Fclose(int handle);
extern long Fwrite(int handle, long count, void *buf);

extern char ovl_log[];                              /* OVL.C */
extern char ovl_conf[];
extern char cab_ver_str[];
extern char ovl_ver_str[];

extern void clear_info(REQUEST *req);               /* 0x1598c */
extern int rsc_next_line(char **line, int tree);    /* 0x1529c, indice de ligne, -1 à la fin */
extern void file_error(char *file, int op, short *err);     /* 0x14952, page d'erreur */

/* Ouvre le fichier de réponse et prépare les champs de la requête.
   Renvoie le handle, négatif en cas d'erreur. */
static int setup_open(req, file)
REQUEST *req;
char *file;
{
    short h;

    clear_info(req);
    req->content_length = 0;
    req->content_type[0] = 0;
    strncat(req->content_type, "text/html", 0xff);
    req->mime_version = 1.0;
    strncat(req->server, "cab.ovl", 0xff);

    h = (short)Fcreate(file, 0);
    if (h < 0)
    {
        (*cab->event)(-0x41L);
        file_error(file, 9, &h);
        trace("? Couldn't create %s!\n", file);
    }
    return h;
}

/* Ferme le fichier de réponse. Renvoie 0, ou -1 si h est invalide, -10 en cas d'erreur. */
static int setup_close(req, h)
REQUEST *req;
short h;
{
    short r;

    if (h <= 0)
        return -1;
    r = (short)Fclose(h);
    if (r < 0)
    {
        (*cab->event)(-0x41L);
        /* L'original passe ici h et non r à la page d'erreur. */
        file_error(req->file, 7, &h);
        trace("? Error with Fclose! (%i)\n", r);
        return -10;
    }
    return 0;
}

/* Écrit une ligne formatée dans le fichier de réponse et l'ajoute à la longueur. */
static void setup_write(REQUEST *req, short h, char *fmt, ...)
{
    static char buf[0x400];     /* 0x2ae06, taille non vérifiée */
    va_list ap;
    long len;

    if (h < 0)
        return;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    len = (long)strlen(buf);
    req->content_length += len;
    if (Fwrite(h, len, buf) < 0)
    {
        (*cab->event)(-0x41L);
        file_error(req->file, 10, &h);
        trace("! cab.ovl: WARNING during write on file, disk full?\n");
    }
}

/* Envoie la page de configuration, arbre 2 du fichier de ressources. */
static void setup_page(req, h)
REQUEST *req;
short h;
{
    char *line;
    char date[50];
    time_t now;
    int n;

    time(&now);
    strftime(date, 50, "%a, %d %b %Y %X", localtime(&now));

    while ((n = rsc_next_line(&line, 2)) > 0)
    {
        switch (n)
        {
        case 4:                 /* (date) */
            setup_write(req, h, date);
            break;
        case 61:                /* (address) */
            setup_write(req, h, OVL_ADDRESS);
            break;
        case 50:                /* case "a", journal */
            setup_write(req, h, line);
            if (conf.log_on == 1)
                setup_write(req, h, "checked");
            break;
        case 53:                /* case "b", mode debug */
            setup_write(req, h, line);
            if (conf.log_all == 1)
                setup_write(req, h, "checked");
            break;
        case 32:                /* champ "d", attente en secondes */
            setup_write(req, h, line, conf.timeout);
            break;
        case 37:                /* champ "g", intervalle des événements GEM */
            setup_write(req, h, line, (int)conf.event_interval);
            break;
        case 39:                /* champ "h", durée des événements GEM */
            setup_write(req, h, line, conf.event_duration);
            break;
        case 43:                /* case "o", mode en ligne */
            setup_write(req, h, line);
            if (conf.online == 1)
                setup_write(req, h, "checked");
            break;
        case 22:                /* liste "i", une option par mode de cookie */
        case 23:
        case 24:
        case 25:
        case 26:
            setup_write(req, h, line,
                        conf.cookie_mode == n - 21 ? " SELECTED" : "");
            break;
        default:
            setup_write(req, h, line);
            break;
        }
    }
}

/* Applique les réglages reçus du formulaire, "a=on&b=on&d=60&g=500&h=5&i=5&o=on".
   Une case absente de la requête est décochée. Enregistre la configuration
   puis envoie la page de réponse, arbre 3. */
static void regle_setup(req, h, query)
REQUEST *req;
short h;
char *query;
{
    char *line;
    char *tok;
    int log_was_open;
    int log_unchecked = 1;

    log_was_open = log_is_open();
    trace("> regle_setup\n");
    log_set_mask(1L);

    conf.log_on = 0;
    conf.log_all = 0;
    conf.cookie_mode = 5;
    conf.online = 0;

    for (tok = strtok(query, "&"); tok != 0; tok = strtok(0, "&"))
    {
        trace("regle_setup process <%s>\n", tok);
        if (strnicmp(tok, "a=on", 4) == 0)
        {
            conf.log_on = 1;
            trace("! regle_setup : logfile = ON\n");
            if (!log_was_open)
            {
                log_open(ovl_log, "OVL ", 500L, 0x811L, 13L);
                log_welcome();
            }
            log_unchecked = 0;
        }
        else if (strnicmp(tok, "b=on", 4) == 0)
        {
            conf.log_all = 1;
            log_set_mask(0x100fL);
            trace("~ CAB version <V%s>\n", cab_ver_str);
            trace("~ OVL version <V%s>\n", ovl_ver_str);
        }
        else if (strnicmp(tok, "d=", 2) == 0)
            conf.timeout = atoi(tok + 2);
        else if (strnicmp(tok, "g=", 2) == 0)
            conf.event_interval = (float)atoi(tok + 2);
        else if (strnicmp(tok, "h=", 2) == 0)
            conf.event_duration = atoi(tok + 2);
        else if (strnicmp(tok, "i=", 2) == 0)
            conf.cookie_mode = atoi(tok + 2);
        else if (strnicmp(tok, "o=on", 4) == 0)
            conf.online = 1;
    }

    /* Journal décoché : on le ferme. */
    if (log_unchecked == 1)
    {
        log_end();
        log_close();
        conf.log_on = 0;
    }
    SaveConf(ovl_conf);

    while (rsc_next_line(&line, 3) > 0)
        setup_write(req, h, line);
}

/* Répond à une adresse http://cab.ovl/name : page de configuration pour
   un nom vide, welcome.html ou setup.html, réglages pour setup-cgi?...
   Renvoie 0, ou -10 si le fichier de réponse ne s'ouvre pas. */
int SendSetup(req, name)
REQUEST *req;
char *name;
{
    short h;
    char *p;
    static char query[0x100];       /* 0x2ad06 */

    trace("> SendSetup <%s>\n", name);

    h = (short)setup_open(req, req->file);
    if (h < 0)
        return -10;

    query[0] = 0;
    if (name[0] == 0
        || strnicmp(name, "welcome.html", 12) == 0
        || strnicmp(name, "setup.html", 10) == 0)
        setup_page(req, h);
    else if (strnicmp(name, "setup-cgi", 9) == 0)
    {
        p = strchr(name, '?');
        if (p != 0)
            strncat(query, p + 1, 0xff);
        regle_setup(req, h, query);
    }

    setup_close(req, h);
    return 0;
}
