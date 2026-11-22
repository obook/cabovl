/*
 * TCP.C - couche TCP et connexion de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C, convention
 * d'appel par registres). Le nom de module d'origine est inconnu : aucune
 * chaîne __FILE__ ne le donne dans cette zone. Non compilé avec Pure C,
 * à vérifier.
 *
 * Schéma : init_tcp essaie d'abord STiK (ou STinG, même interface), puis
 * IConnect. Elle lève tcp_is_stik ou tcp_is_iconnect. Les fonctions X_*
 * testent ces deux drapeaux, IConnect en premier, et appellent la variante
 * STIK_* ou ICONNECT_*. Sans pile, elles renvoient une valeur d'échec.
 *
 * STiK : le pilote est la table de fonctions TPL (TCP.H) dont le pointeur
 * est en stik_tpl (0x2c344). Le cookie "STiK" et la recherche du module
 * "TRANSPORT_TCPIP" sont faits par stik_lib_init (0x1c4a6, bibliothèque).
 *
 * IConnect : appels de sockets de la bibliothèque (0x1d4xx-0x1db06),
 * détectée par le cookie "SLIP" (0x1da48). Noms ic_* choisis pour la
 * reconstitution, rôle déduit du code (non vérifié).
 *
 * Reconstituées : init_tcp, X_TCP_open, X_TCP_send, X_TCP_close,
 * X_get_err_text, X_resolve, X_CNbyte_count, X_CNget_block, X_getvstr,
 * STIK_*, ICONNECT_*, open_connection, my_resolver et le cache de noms.
 */

#include <stdio.h>
#include <string.h>
#include "CABOVL.H"
#include "LOG.H"
#include "TCP.H"

/* Fonctions de bibliothèque, non reconstituées. */
extern int stik_lib_init(void);         /* 0x1c4a6, 0 ou -0x67 si le pilote est utilisable, -0x64 à -0x68 sinon */
extern char *ip_to_str(long ip);        /* 0x1c6b2, "%u.%u.%u.%u" dans un tampon statique */
extern char *stik_error_text(int code); /* 0x15574, texte d'erreur STiK, code négatif ou non */
extern char *itoa(int value, char *buf, int radix);     /* Pure C, 0x1acac */

/* Sockets IConnect (noms choisis). */
extern int ic_init(void);                       /* 0x1da48, -1 si pas de cookie "SLIP", -2 si trop ancien */
extern int ic_socket(int family, int type, int proto);      /* 0x1d962 */
extern long ic_fcntl(int fd, long cmd, long arg);           /* 0x1dae0 */
extern int ic_connect(int fd, void *addr, int len);         /* 0x1d93c */
extern int ic_send(int fd, char *buf, int len);             /* 0x1da7a */
extern int ic_recv(int fd, char *buf, long len);            /* 0x1da9e */
extern int ic_close(int fd);                                /* 0x1dac4 */
extern int ic_select(int nfds, void *rd, void *wr, void *ex, void *timeout);    /* 0x1d9b6 */
extern int ic_getconf(void *conf);              /* 0x1db06, lit une variable de configuration, 0 si réussi */
extern char *ic_ntoa(long ip);                  /* 0x1d69e */

/* Hôte renvoyé par ic_gethostbyname, même disposition que struct hostent
   avec des int de 16 bits. */
typedef struct
{
    char *h_name;               /* 0x00 */
    char **h_aliases;           /* 0x04 */
    short h_addrtype;           /* 0x08 */
    short h_length;             /* 0x0a */
    char **h_addr_list;         /* 0x0c, terminée par 0 */
} HOSTENT;

extern HOSTENT *ic_gethostbyname(char *name);   /* 0x1d556 */

/* Appel de notification de CAB, fonction en cab + 0x24. Rôle non vérifié :
   appelée avec le code 1 autour de la connexion et 4 avant la résolution. */
#define CAB_NOTIFY(code) \
    ((*(void (**)())((char *)cab + 0x24))((long)(code), 0L))

short tcp_is_stik = 0;              /* 0x22436 */
short tcp_is_iconnect = 0;          /* 0x22438 */

/* Valeurs du module IConnect, chaque tampon fait 0x7f octets, base 0x2b5e2. */
#define IC_EMAIL 0
#define IC_HTTP_PORT 1
#define IC_HTTP_PROXY 2
#define IC_FTP_PORT 3
#define IC_FTP_PROXY 4
#define IC_SMTP 5
static char ic_buf[6][0x7f];

static HOSTENT *ic_host;            /* 0x2b8dc */


/* ---- Détection ---- */

/* STiK ou STinG : lève tcp_is_stik. Renvoie 1 si le pilote est utilisable. */
static int STIK_init()
{
    int rc;

    trace("> STIK_init()\n");
    rc = stik_lib_init();
    if (rc == 0 || rc == -0x67)
    {
        tcp_version = stik_tpl->version;
        tcp_transport = "STiK";
        return 1;
    }
    return 0;
}

/* IConnect. Défaut : ic_init renvoie -2 quand le module est trop ancien,
   la valeur n'est pas testée, seul -1 compte comme échec. */
static int ICONNECT_init()
{
    int i;

    for (i = 0; i < 6; i++)
        ic_buf[i][0] = 0;
    if (ic_init() == -1)
        return 0;
    tcp_transport = "Iconnect";
    tcp_version = "00.00";
    return 1;
}

/* Cherche une pile TCP. Renvoie 1 si trouvée. */
int init_tcp()
{
    trace("> init_tcp\n");
    if (STIK_init() == 1)
        tcp_is_stik = 1;
    else if (ICONNECT_init() == 1)
        tcp_is_iconnect = 1;
    else
        return 0;
    return 1;
}


/* ---- STiK ---- */

static int STIK_TCP_open(ip, port, tos, buffer)
long ip;
int port;
int tos;
int buffer;
{
    trace("> STIK_TCP_open <%s> port <%d> tos <%d> buffer <%u>\n",
          ip_to_str(ip), port, tos, buffer);
    return stik_tpl->TCP_open(ip, port, tos, buffer);
}

static int STIK_TCP_send(handle, buf, size)
int handle;
char *buf;
int size;
{
    trace("> STIK_TCP_send handle <%d> size <%d>\n", handle, size);
    return stik_tpl->TCP_send(handle, buf, size);
}

static int STIK_TCP_close(handle, mode)
int handle;
int mode;
{
    trace("> STIK_TCP_close\n");
    return stik_tpl->TCP_close(handle, mode, 0L);
}

static int STIK_resolve(name, real_name, n, out)
char *name;
char **real_name;
int n;
long *out;
{
    trace("> STIK_resolve\n");
    return stik_tpl->resolve(name, real_name, out, n);
}

static int STIK_CNbyte_count(handle)
int handle;
{
    return stik_tpl->CNbyte_count(handle);
}

/* Lit un bloc. Défaut : si le bloc NDB reçu dépasse size, l'excédent est
   libéré avec le bloc et perdu. */
static int STIK_CNget_block(handle, buf, size)
int handle;
char *buf;
int size;
{
    NDB *ndb;
    unsigned short n;

    ndb = stik_tpl->CNget_NDB(handle);
    if (ndb == 0)
        return stik_tpl->CNget_block(handle, buf, size);

    n = (unsigned short)size;
    if (ndb->len < n)
        n = ndb->len;
    memcpy(buf, ndb->ndata, n);
    stik_tpl->KRfree(ndb->ptr);
    stik_tpl->KRfree(ndb);
    return n;
}

static char *STIK_getvstr(name)
char *name;
{
    trace("> STIK_getvstr <%s>\n", name);
    return stik_tpl->getvstr(name);
}


/* ---- IConnect ---- */

/* Masque de descripteur calculé comme le binaire : décalage sur 16 bits
   (lsl.w), donc 0 pour les descripteurs 16 à 31 et négatif pour 15.
   Défaut : les descripteurs de 16 à 31 ne sont jamais vus par select. */
static long fd_mask(fd)
int fd;
{
    int bit;

    bit = fd & 31;
    if (bit >= 16)
        return 0L;
    return (long)(short)(1 << bit);
}

/* Ouvre une connexion non bloquante. Renvoie le descripteur ou -1. */
static int ICONNECT_TCP_open(ip, port, tos, buffer)
long ip;
int port;
int tos;
int buffer;
{
    struct
    {
        short family;
        short port;
        long addr;
        char zero[8];
    } sa;
    int fd;
    long flags;
    int rc;

    /* Le binaire appelle ici une fonction vide (0x16126) avec buffer et tos. */
    fd = ic_socket(2, 1, 0);
    if (fd < 0)
        return -1;
    flags = ic_fcntl(fd, 3L, 0L);
    ic_fcntl(fd, 4L, flags | 0x10000L);
    memset(&sa, 0, sizeof(sa));
    sa.family = 6;              /* différent du 2 de ic_socket, non vérifié */
    sa.port = port;
    memcpy(&sa.addr, &ip, 4);
    rc = ic_connect(fd, &sa, 16);
    if (rc < 0 && rc != -0x145 && rc != -0x144)
        return -1;
    return fd;
}

/* Envoie size octets. Réessaie tant que ic_send renvoie -0x146. */
static int ICONNECT_TCP_send(handle, buf, size)
int handle;
char *buf;
int size;
{
    int rc;

    do
        rc = ic_send(handle, buf, size);
    while (rc == -0x146);
    if (rc == size)
        return 0;
    return -9;
}

static int ICONNECT_TCP_close(handle, mode)
int handle;
int mode;
{
    return ic_close(handle);
}

/* Défaut : ne garde que la dernière adresse de la liste, et renvoie 0 en
   cas de réussite, donc my_resolver ne met pas ce résultat en cache. */
static int ICONNECT_resolve(name, real_name, n, out)
char *name;
char **real_name;
int n;
long *out;
{
    int i;

    ic_host = ic_gethostbyname(name);
    if (ic_host == 0)
        return -0x11;
    for (i = 0; ic_host->h_addr_list[i] != 0; i++)
        *out = *(long *)ic_host->h_addr_list[i];
    return 0;
}

/* Renvoie 0x2000 si des données sont prêtes, 0 sinon, -3 en cas d'erreur. */
static int ICONNECT_CNbyte_count(fd)
int fd;
{
    unsigned long set[64];
    short timeout[2];
    long rc;
    long mask;

    memset(set, 0, sizeof(set));
    set[fd >> 5] |= fd_mask(fd);
    timeout[0] = 3;
    timeout[1] = 0;
    rc = ic_select(fd + 1, set, 0L, 0L, timeout);
    if (rc < 0)
    {
        trace("! select error <%ld>\n", rc);
        return -3;
    }
    mask = set[fd >> 5] & fd_mask(fd);
    trace("! FD_ISSET = <%ld>\n", mask);
    if (mask < 0)
        return -3;
    if (mask > 0)
        return 0x2000;
    return 0;
}

static int ICONNECT_CNget_block(handle, buf, size)
int handle;
char *buf;
int size;
{
    int n;

    n = ic_recv(handle, buf, (long)size);
    trace("! DEBUG X_CNget_block: sread return <%d>\n", n);
    if (n == -0x13c)
        return -3;
    if (n == -0x146)
        return 0;
    return n;
}

/* Trace une variable absente et renvoie "0". code : -3 hors ligne,
   -1 non définie, 0 aucune erreur, autre inconnu. */
static char *ICONNECT_vstr_error(code, name)
int code;
char *name;
{
    char *text;

    switch (code)
    {
    case -3:
        text = "You are offline";
        break;
    case -1:
        text = "Not Set";
        break;
    case 0:
        text = "No error";
        break;
    default:
        text = "Unknown";
        break;
    }
    trace("! Variable <%s> : <%s>\n", name, text);
    return "0";
}

/* Bloc passé à ic_getconf, disposition lue dans le binaire (non vérifié). */
typedef struct
{
    short kind;                 /* 0x00, 9 courriel, 10 proxy, 6 SMTP */
    long ip;                    /* 0x02, adresse renvoyée */
    short port;                 /* 0x06, port renvoyé */
    char *p[4];                 /* 0x08, chaînes d'entrée, p[0] renvoie le courriel */
} IC_CONF;

/* Table des variables : nom, longueur comparée, type, protocole, tampon,
   format de trace. */
typedef struct
{
    char *name;
    int len;
    short kind;
    char *proto;
    int slot;
    char *fmt;
} IC_VAR;

static IC_VAR ic_vars[6] =
{
    { "EMAIL", 5, 9, "", IC_EMAIL, "~ EMAIL <%s>\n" },
    { "HTTP_PROXY_PORT", 15, 10, "http", IC_HTTP_PORT, "~ UR_PROXY_IP http port <%s>\n" },
    { "HTTP_PROXY", 10, 10, "http", IC_HTTP_PROXY, "~ UR_PROXY_IP http <%s>\n" },
    { "FTP_PROXY_PORT", 14, 10, "ftp", IC_FTP_PORT, "~ UR_PROXY_IP ftp port <%s>\n" },
    { "FTP_PROXY", 9, 10, "ftp", IC_FTP_PROXY, "~ UR_PROXY_IP ftp <%s>\n" },
    { "SMTP_SERVER", 11, 6, "", IC_SMTP, "~ SMTP_SERVER <%s>\n" }
};

/* Valeur d'une variable de configuration IConnect, "0" si absente. Les
   tampons de ic_buf sont réutilisés d'un appel à l'autre. */
static char *ICONNECT_getvstr(name)
char *name;
{
    IC_CONF c;
    IC_VAR *v;
    char *buf;
    int i;
    int rc;

    trace("> ICONNECT_getvstr <%s>\n", name);
    for (i = 0; i < 6; i++)
    {
        v = &ic_vars[i];
        if (strnicmp(name, v->name, v->len) != 0)
            continue;

        c.kind = v->kind;
        c.p[0] = "";
        c.p[1] = v->proto;
        c.p[2] = "";
        c.p[3] = "";
        rc = ic_getconf(&c);
        if (rc != 0)
            return ICONNECT_vstr_error(rc, v->name);

        buf = ic_buf[v->slot];
        buf[0] = 0;
        if (v->slot == IC_EMAIL)
            strncat(buf, c.p[0], 0x7e);
        else if (v->slot == IC_HTTP_PORT || v->slot == IC_FTP_PORT)
            itoa(c.port, buf, 10);
        else
            strncat(buf, ic_ntoa(c.ip), 0x7e);
        trace(v->fmt, buf);
        return buf;
    }
    return ICONNECT_vstr_error(-2, name);
}


/* ---- Enveloppes X_* ---- */

/* Renvoie le descripteur ou un code négatif, -1 sans pile. */
int X_TCP_open(ip, port, tos, buffer)
long ip;
int port;
int tos;
int buffer;
{
    trace("> X_TCP_open\n");
    if (tcp_is_iconnect == 1)
        return ICONNECT_TCP_open(ip, port, tos, buffer);
    if (tcp_is_stik == 1)
        return STIK_TCP_open(ip, port, tos, buffer);
    return -1;
}

int X_TCP_send(handle, buf, size)
int handle;
char *buf;
int size;
{
    trace("> X_TCP_send handle <%d> size <%d>\n", handle, size);
    if (tcp_is_iconnect == 1)
        return ICONNECT_TCP_send(handle, buf, size);
    if (tcp_is_stik == 1)
        return STIK_TCP_send(handle, buf, size);
    return -9;
}

int X_TCP_close(handle, mode)
int handle;
int mode;
{
    trace("> X_TCP_close\n");
    if (tcp_is_iconnect == 1)
        return ICONNECT_TCP_close(handle, mode);
    if (tcp_is_stik == 1)
        return STIK_TCP_close(handle, mode);
    return -1;
}

/* Le texte vient de la table d'erreurs STiK, aussi pour IConnect. */
char *X_get_err_text(code)
int code;
{
    trace("> X_get_err_text <%d>\n", code);
    if (tcp_is_iconnect == 1 || tcp_is_stik == 1)
        return stik_error_text(code);
    return "??";
}

/* Renvoie le nombre d'adresses (STiK) ou 0 (IConnect), négatif si erreur. */
int X_resolve(name, real_name, n, out)
char *name;
char **real_name;
int n;
long *out;
{
    trace("> X_resolve\n");
    if (tcp_is_iconnect == 1)
        return ICONNECT_resolve(name, real_name, n, out);
    if (tcp_is_stik == 1)
        return STIK_resolve(name, real_name, n, out);
    return -1;
}

/* Octets disponibles sur la connexion, sans trace d'entrée. Nom non
   vérifié : aucune chaîne "> X_..." dans le binaire. */
int X_CNbyte_count(handle)
int handle;
{
    if (tcp_is_iconnect == 1)
        return ICONNECT_CNbyte_count(handle);
    if (tcp_is_stik == 1)
        return STIK_CNbyte_count(handle);
    return -1;
}

int X_CNget_block(handle, buf, size)
int handle;
char *buf;
int size;
{
    trace("> X_CNget_block\n");
    if (tcp_is_iconnect == 1)
        return ICONNECT_CNget_block(handle, buf, size);
    if (tcp_is_stik == 1)
        return STIK_CNget_block(handle, buf, size);
    return 0;
}

/* Valeur d'une variable de la pile (proxy, courriel, SMTP), "0" sans pile.
   Nom non vérifié. */
char *X_getvstr(name)
char *name;
{
    if (tcp_is_iconnect == 1)
        return ICONNECT_getvstr(name);
    if (tcp_is_stik == 1)
        return STIK_getvstr(name);
    return "0";
}


/* ---- Résolution et connexion ---- */

/* Cache de noms : 50 entrées de 0x104 octets en 0x27342. Le cache est
   rempli en rond, dns_next compte de 0 à 0x31. Défaut : la recherche ne
   parcourt que 0x31 entrées (0 à 0x30), la 50e n'est jamais retrouvée. */
#define DNS_SIZE 0x32
#define DNS_SEARCH 0x31

typedef struct
{
    char name[0x100];
    long ip;
} DNS_ENTRY;

static short dns_next = 0;          /* 0x2165c */
static short dns_fresh = 1;         /* 0x2165e, 1 tant que le cache n'est pas initialisé */
static DNS_ENTRY dns_cache[DNS_SIZE];   /* 0x27342 */

/* 0x131fa : vide le cache (premier appel de my_resolver). */
static void dns_init()
{
    int i;

    for (i = 0; i < DNS_SEARCH; i++)
    {
        dns_cache[i].name[0] = 0;
        dns_cache[i].ip = 0;
    }
    dns_fresh = 0;
}

/* 0x131d8 : retire la dernière entrée ajoutée, quand la connexion a échoué.
   N'efface que le nom. Après un tour complet du cache, le recul d'un cran
   depuis 0 donne 0x31. */
void dns_drop_last()
{
    dns_next--;
    if (dns_next < 0)
        dns_next = 0x31;
    dns_cache[dns_next].name[0] = 0;
}

/* Résout name en adresse IP dans *ip. Renvoie 0 si trouvé dans le cache,
   sinon le résultat de X_resolve (nombre d'adresses, négatif si erreur).
   Seuls les résultats positifs sont mis en cache. */
int my_resolver(name, ip)
char *name;
long *ip;
{
    int i;
    int n;

    trace("> my_resolver <%s>\n", name);
    if (dns_fresh == 1)
        dns_init();
    *ip = 0;
    for (i = 0; i < DNS_SEARCH; i++)
    {
        if (stricmp(dns_cache[i].name, name) == 0)
        {
            *ip = dns_cache[i].ip;
            return 0;
        }
    }
    CAB_NOTIFY(4);
    n = X_resolve(name, 0, 1, ip);
    if (n > 0)
    {
        dns_cache[dns_next].name[0] = 0;
        strncat(dns_cache[dns_next].name, name, 0xff);
        dns_cache[dns_next].ip = *ip;
        dns_next++;
        if (dns_next > 0x31)
            dns_next = 0;
    }
    return n;
}

/* Résout server puis ouvre la connexion. Renvoie le descripteur ou un
   code négatif. Un échec de connexion retire le nom du cache. */
int open_connection(server, port, tos)
char *server;
int port;
int tos;
{
    long ip;
    int rc;

    trace("> open_connection server <%s> port <%i> TOS <%i>\n", server, port, tos);
    rc = my_resolver(server, &ip);
    if (rc < 0)
    {
        trace("! Resolve <%s> returns <%s>\n", server, X_get_err_text(rc));
        return rc;
    }
    CAB_NOTIFY(1);
    rc = X_TCP_open(ip, port, tos, 0x2000);
    if (tcp_is_iconnect == 1)
        CAB_NOTIFY(1);
    if (rc < 0)
        dns_drop_last();
    return rc;
}
