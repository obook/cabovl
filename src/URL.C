/*
 * URL.C - analyse d'URL et configuration de proxy de CAB.OVL (V5.68)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C).
 * Fonctions en 0x1598c (clear_info), 0x159f2 (split_URL), 0x15b06 (fix_proxy),
 * 0x15d80 (http_proxy_cfg), 0x15e06 (ftp_proxy_cfg).
 * Le nom de module d'origine est inconnu.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdlib.h>
#include <string.h>
#include "CABOVL.H"
#include "REQUEST.H"
#include "LOG.H"

/* Lit une variable de la pile TCP (ICONNECT ou STiK), "0" si aucune pile.
   Nom non fiable : hypothèse d'après les appels à ICONNECT_getvstr et
   STIK_getvstr. */
extern char *X_getvstr(char *name);      /* 0x1636c */

/* Efface les champs d'en-tête de la réponse. */
void clear_info(r)
REQUEST *r;
{
    trace("> clear_info\n");
    r->www_auth[0] = 0;
    r->server_hdr[0] = 0;
    r->pragma[0] = 0;
    r->location[0] = 0;
    r->content_type[0] = 0;
    r->encoding[0] = 0;
    r->last_modified = -1L;
    r->expires = -1L;
    r->date = -1L;
    r->mime_version = -1L;
    r->content_length = -1L;
    r->status_text[0] = 0;
}

/* Découpe url en protocole, serveur, port et chemin dans r.
   Le protocole va en r+0x62 (10 octets, champ absent de REQUEST.H, non
   nommé).
   Défauts : pas de contrôle de taille (host 0x100, protocole 10 octets) et
   url sans "://" donne un résultat indéfini. r->url n'est pas renseigné. */
void split_URL(url, r)
char *url;
REQUEST *r;
{
    long colon;
    long slash;

    trace("> split_URL\n");

    colon = strcspn(url, ":");
    strncpy((char *)r + 0x62, url, colon);
    ((char *)r)[0x62 + colon] = 0;
    strcpy(r->path, url + colon + 3);

    slash = strcspn(r->path, "/");
    strncpy(r->server, r->path, slash);
    r->server[slash] = 0;
    strcpy(r->path, url + colon + slash + 3);

    colon = strcspn(r->server, ":");
    if (colon < (long)strlen(r->server) && colon != 0)
    {
        r->port = atoi(r->server + colon + 1);
        r->server[colon] = 0;
    }
    else
        r->port = 80;
    trace("! DEBUG colon_index <%ld> strlen <%ld> request->port <%d>\n",
          colon, (long)strlen(r->server), r->port);

    if (r->port == 0)
        r->port = 80;
    if (r->path[0] == 0 || r->path[0] == ' ')
    {
        r->path[0] = 0;
        strncat(r->path, "/", 0x7ff);
    }
}

/* Ajoute value à dst, au plus 0x7e caractères. */
static void append_proxy(dst, value)
char *dst;
char *value;
{
    strncat(dst, value, 0x7e);
}

/* Valeur absente : la pile renvoie "0" ou "1". */
static int is_unset(v)
char *v;
{
    return *v == '0' || *v == '1';
}

/* Remplit les chaînes de proxy de la table p depuis les variables de la pile.
   Table de pointeurs : 0 FTP, 4 HTTP, 8 WAIS, 0xc GOPHER, 0x10 NEWS,
   0x18 SMTP_SERVER, 0x1c NNTP_SERVER (0x14 n'est pas touché).
   Chaque chaîne reçoit au plus 0x7e caractères par ajout. Le port HTTP vaut
   "80" si HTTP_PROXY_PORT est absent. Les autres protocoles n'ont pas de
   port par défaut. Renvoie p. */
char **fix_proxy(p)
char **p;
{
    char *v;
    char *port;

    trace("> fix_proxy\n");

    p[0][0] = 0;
    v = X_getvstr("FTP_PROXY");
    if (!is_unset(v))
    {
        append_proxy(p[0], v);
        port = X_getvstr("FTP_PROXY_PORT");
        if (!is_unset(port))
        {
            append_proxy(p[0], ":");
            append_proxy(p[0], port);
        }
    }

    p[1][0] = 0;
    v = X_getvstr("HTTP_PROXY");
    if (!is_unset(v))
    {
        append_proxy(p[1], v);
        port = X_getvstr("HTTP_PROXY_PORT");
        append_proxy(p[1], ":");
        append_proxy(p[1], is_unset(port) ? "80" : port);
    }

    p[2][0] = 0;
    v = X_getvstr("WAIS_PROXY");
    if (!is_unset(v))
    {
        append_proxy(p[2], v);
        port = X_getvstr("WAIS_PROXY_PORT");
        if (!is_unset(port))
        {
            append_proxy(p[2], ":");
            append_proxy(p[2], port);
        }
    }

    p[3][0] = 0;
    v = X_getvstr("GOPHER_PROXY");
    if (!is_unset(v))
    {
        append_proxy(p[3], v);
        port = X_getvstr("GOPHER_PROXY_PORT");
        if (!is_unset(port))
        {
            append_proxy(p[3], ":");
            append_proxy(p[3], port);
        }
    }

    p[4][0] = 0;
    v = X_getvstr("NEWS_PROXY");
    if (!is_unset(v))
    {
        append_proxy(p[4], v);
        port = X_getvstr("NEWS_PROXY_PORT");
        if (!is_unset(port))
        {
            append_proxy(p[4], ":");
            append_proxy(p[4], port);
        }
    }

    p[6][0] = 0;
    v = X_getvstr("SMTP_SERVER");
    if (!is_unset(v))
        append_proxy(p[6], v);

    p[7][0] = 0;
    v = X_getvstr("NNTP_SERVER");
    if (!is_unset(v))
        append_proxy(p[7], v);

    return p;
}

/* Lit le proxy HTTP "serveur[:port]" fourni par CAB (cab->proxy est une
   table de chaînes, l'entrée 1 est HTTP). Port 80 par défaut.
   size est la taille maximale copiée dans host. Renvoie 1 si un proxy est
   défini, 0 sinon.
   Défaut : sans ":" la copie par strncpy n'ajoute pas de zéro final. */
int http_proxy_cfg(host, port, size)
char *host;
short *port;
short size;
{
    char *s;
    long colon;

    trace("> http_proxy_cfg (Getting proxy)\n");

    s = ((char **)cab->proxy)[1];
    if (*s == 0)
        return 0;

    colon = strcspn(s, ":");
    if (colon < (long)strlen(s) && colon != 0)
    {
        host[0] = 0;
        strncat(host, s, colon);
        *port = atoi(s + colon + 1);
    }
    else
    {
        strncpy(host, s, (long)size);
        *port = 80;
    }
    return 1;
}

/* Lit le proxy FTP dans les variables de la pile. Renvoie 1 si FTP_PROXY et
   FTP_PROXY_PORT sont définis, 0 sinon (pas de port par défaut).
   Défaut : host est déjà rempli, sans zéro final garanti (strncpy), quand
   seul le port manque. */
int ftp_proxy_cfg(host, port, size)
char *host;
short *port;
short size;
{
    char *v;

    trace("> ftp_proxy_cfg (Getting proxy)\n");

    v = X_getvstr("FTP_PROXY");
    if (is_unset(v))
        return 0;
    strncpy(host, v, (long)size);

    v = X_getvstr("FTP_PROXY_PORT");
    if (is_unset(v))
        return 0;
    *port = atoi(v);
    return 1;
}
