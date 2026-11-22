/*
 * OVL.C - point d'entrée de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions en
 * 0x10168 (ovl_entry) et 0x10570 (ovl_infos). Le nom du module d'origine
 * est inconnu : aucune chaîne __FILE__ ne le donne.
 * Non compilé avec Pure C, à vérifier.
 *
 * OVLINFO.H définit OVL_INFO, la chaîne d'information du module
 * (auteur, titre, contact, adresse web) lue dans le binaire en 0x20442,
 * et OVL_ADDRESS, la ligne de signature lue en 0x21e32 (utilisée par SETUP.C).
 */

#include <stdio.h>
#include <string.h>
#include "CONFIG.H"
#include "CABOVL.H"
#include "OVLINFO.H"
#include "LOG.H"
#include "TCP.H"

CABINFO *cab;                       /* 0x245a6 */
long cab_version;                   /* 0x21214 */

/* Noms de la pile TCP détectée, initialisés à "". */
char *tcp_version = "";             /* 0x2242e */
char *tcp_transport = "";           /* 0x22432 */

/* Chemins de fichiers, 0x81 octets chacun, tous préfixés par le dossier du module. */
char ovl_dir[0x81];                 /* 0x25204 */
char ovl_rsc[0x81];                 /* 0x25285, cab_ovl.rsc */
char ck_tmpname[0x81];              /* 0x25306, cabovl.tmp */
char ovl_log[0x81];                 /* 0x25387, cabovl.log */
char ovl_conf[0x81];                /* 0x25408, cabovl.set */
char ck_filename[0x81];             /* 0x25489, cookie.set */
char ovl_lang[0x81];                /* 0x2550a, language.str */
char ovl_debug[0x81];               /* 0x2558b, debug.txt */
char ovl_java[0x81];                /* 0x2560c, plug-in\java.ovl */

char auth_cred[0x81];               /* 0x24ee2, "utilisateur:mot de passe" saisi */
char auth_host[0x81];               /* 0x24f63, serveur protégé par mot de passe */
char auth_b64[0x81];                /* 0x24fe4, identifiants codés en base64 */
char cab_ver_str[0x81];             /* 0x25102, version de CAB en texte */
char ovl_ver_str[0x81];             /* 0x25183, version du module en texte */
char user_agent[0x100];             /* 0x245aa, taille non vérifiée */

extern char *ck_current;                            /* COOKIE.C */

extern int my_form_alerte(char *title, char *fmt, ...);     /* 0x15320 */
extern int rsc_load(char *file);                    /* 0x14c9c, 1 si réussi */
extern char *get_rsc_string(int index);             /* 0x1555e, chaîne de la ressource */
extern int form_alert_abort(char *alert, int button);       /* 0x1c1ca, non analysé */
extern char *format_cab_version(long v);            /* 0x16018, "2.70" */
extern char *format_ovl_version(long v);            /* 0x1609e, "5.68" */
extern void gem_cleanup(void);                      /* 0x14d30 */
extern char **fix_proxy(char **proxy);              /* URL.C, 0x15b06 */
extern int ftp_proxy_cfg(char *buf, short *port, short size);   /* 0x15e06 */

extern int restore_module();                        /* 0x10590 */
extern int get_url();                               /* 0x107c2 */
extern int get_url_info();                          /* 0x10788 */
extern int post();                                  /* 0x10800 */
extern int get_url_if_modified();                   /* 0x1083a */
extern int get_url_cookie();                        /* 0x1087c */
extern int get_url_ifmod_cookie();                  /* 0x108c4 */
extern int post_cookie();                           /* 0x10910 */

/* Remplit les trois valeurs que CAB demande pour décrire le module. */
void ovl_infos(info, version, date)
char **info;
long *version;
long *date;
{
    *version = OVL_VERSION;
    *date = OVL_DATE;
    *info = OVL_INFO;
}

/* Concatène dir et name dans dest (0x81 octets). */
static void make_path(dest, dir, name)
char *dest;
char *dir;
char *name;
{
    dest[0] = 0;
    strncat(dest, dir, 0x80);
    strncat(dest, name, 0x80);
}

/* Entrée du module. CAB passe la table de fonctions à remplir, sa propre
   structure et le dossier du module (init_module dans l'interface officielle).
   Renvoie OVL_SUPPORT, ou 0 en cas d'échec (ressource introuvable, pas de
   pile TCP). */
int ovl_entry(funcs, cab_info, dir)
OVLFUNCS *funcs;
CABINFO *cab_info;
char *dir;
{
    int tries;
    int len;
    int ok;
    char proxy_buf[0x100];
    short proxy_port;

    ovl_dir[0] = 0;
    strncat(ovl_dir, dir, 0x80);

    make_path(ovl_rsc, ovl_dir, "cab_ovl.rsc");
    if (rsc_load(ovl_rsc) != 1)
    {
        form_alert_abort("[3][CAB.OVL can not found | is resource file][ Abort ]", 1);
        return 0;
    }

    make_path(ck_tmpname, ovl_dir, "cabovl.tmp");

    cab = cab_info;
    funcs->restore_module = restore_module;
    funcs->get_url = get_url;
    funcs->get_url_info = get_url_info;
    funcs->infos = (int (*)())ovl_infos;
    funcs->post = post;
    funcs->get_url_if_modified = get_url_if_modified;

    make_path(ovl_log, ovl_dir, "cabovl.log");
    make_path(ovl_conf, ovl_dir, "cabovl.set");
    make_path(ck_filename, ovl_dir, "cookie.set");
    make_path(ovl_lang, ovl_dir, "language.str");
    make_path(ovl_debug, ovl_dir, "debug.txt");
    make_path(ck_tmpname, ovl_dir, "cabovl.tmp");
    make_path(ovl_java, ovl_dir, "plug-in\\");
    strncat(ovl_java, "java.ovl", 0x80);

    auth_host[0] = 0;
    auth_b64[0] = 0;

    cab_version = cab->version;
    cab_ver_str[0] = 0;
    strncat(cab_ver_str, format_cab_version(cab_version), 0x80);
    ovl_ver_str[0] = 0;
    strncat(ovl_ver_str, format_ovl_version(OVL_VERSION), 0x80);

    InitConf(ovl_conf);

    if (conf.log_on == 1)
    {
        /* Journal : essaie cabovl.log puis, en cas d'échec, remplace le
           dernier caractère du nom par 1, 2, ... jusqu'à 9 essais. */
        tries = 0;
        len = (int)strlen(ovl_log);
        do
        {
            ok = log_open(ovl_log, "OVL ", 500L, 0x811L, 13L);
            if (ok == 0)
            {
                if (tries == 0)
                    ovl_log[len - 1] = '1';
                else
                    ovl_log[len - 1] = ovl_log[len - 1] + 1;
            }
            tries++;
        } while (ok != 1 && tries < 9);

        log_welcome();
        if (conf.log_all == 1)
            log_set_mask(0x100fL);
        trace("~ CAB version <V%s>\n", cab_ver_str);
        trace("~ OVL version <V%s>\n", ovl_ver_str);
    }

    if (init_tcp() != 1)
    {
        my_form_alerte("NetWorkError", get_rsc_string(2));
        trace("! Network Error: no StiK/STinK/Draconis\n");
        log_close();
        gem_cleanup();
        log_end();
        log_close();
        return 0;
    }

    trace("~ Transport <%s> detected version <%s>\n", tcp_transport, tcp_version);
    sprintf(user_agent, "Mozilla/3.0 (compatible; CAB%s/%s; Atari)",
            cab_ver_str, ovl_ver_str);

    if (cab_version >= CAB_VERSION_2_70)
    {
        funcs->get_url_cookie = get_url_cookie;
        funcs->post_cookie = post_cookie;
        funcs->get_url_ifmod_cookie = get_url_ifmod_cookie;
    }

    ck_current = 0;
    cab->proxy = fix_proxy(cab->proxy);
    ftp_proxy_cfg(proxy_buf, &proxy_port, 0x100);
    return OVL_SUPPORT;
}
