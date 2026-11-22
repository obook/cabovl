/*
 * CONFIG.C - fichier de configuration de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C).
 * Les noms de fonctions viennent des chaînes de trace du binaire.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include "CONFIG.H"
#include "LOG.H"

CONFIG conf;                        /* 0x246aa */

extern int my_form_alerte(char *title, char *fmt, ...);     /* 0x15320 */

/* Charge le fichier, ou applique les valeurs par défaut s'il est illisible.
   Renvoie toujours 1. */
int InitConf(file)
char *file;
{
    trace("> InitConf <%s>\n", file);
    if (!LoadConf(file))
    {
        conf.version = CONF_VERSION;
        conf.log_on = 0;
        conf.log_all = 0;
        conf.timeout = 60;
        conf.event_interval = 500.0;    /* copie du long 0x43fa0000 en 0x2050c */
        conf.event_duration = 5;
        conf.cookie_mode = 5;
        conf.online = 1;
    }
    return 1;
}

/* Lit conf depuis le fichier. Renvoie 0 si le fichier est absent, illisible
   ou d'une autre version, 1 sinon. */
int LoadConf(file)
char *file;
{
    FILE *f;
    int ok = 1;

    trace("> LoadConf <%s>\n", file);
    f = fopen(file, "rb");
    if (f == 0)
        return 0;

    if (fread(&conf, 0x16, 1, f) == 0)
    {
        my_form_alerte("Error",
            "[1][ An error occure reading config file :| %s | please re-set your CAB.OVL setup ][ OK ]",
            file);
        trace("Read w%d> error, please set CAB.OVL setup\n", file);
        ok = 0;
    }
    /* Le test de version a lieu même si la lecture a échoué. */
    if (conf.version != CONF_VERSION)
    {
        my_form_alerte("Error",
            "[1][ Your CAB.OVL config file :| %s | is changed, please re-set it ][ OK ]",
            file);
        trace("Read w%d> error, please set CAB.OVL setup\n", file);
        ok = 0;
    }
    fclose(f);
    return ok;
}

/* Écrit conf dans le fichier. Renvoie 1 si l'écriture réussit, 0 sinon. */
int SaveConf(file)
char *file;
{
    FILE *f;

    trace("> SaveConf <%s>\n", file);
    f = fopen(file, "wb");
    if (f == 0)
    {
        trace_err("ERR_open_write_file %s\n", file);
        return 0;
    }
    conf.version = CONF_VERSION;
    if (fwrite(&conf, 0x16, 1, f) != 0)
    {
        fclose(f);
        return 1;
    }
    trace_err("ERR_write_file %s\n", file);
    fclose(f);
    return 0;
}
