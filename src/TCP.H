/*
 * TCP.H - couche TCP de CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. Types du pilote STiK et STinG
 * (pile de Peter Rottengater) et prototypes des fonctions X_* (enveloppes
 * qui choisissent la pile détectée).
 * Les noms et les décalages de TPL et NDB sont ceux de l'en-tête officiel
 * transprt.h des sources de STinG 1.26 (ATARI/ref/STinG/include/transprt.h,
 * dépôt th-otto/STinG, aussi ATARI/glstk2b4/GLSTK2B4/TRANSPRT.H). Il confirme
 * les décalages utilisés par le binaire. Les appels de la table suivent la
 * convention cdecl : arguments sur la pile.
 */

#ifndef TCP_H
#define TCP_H

/* Bloc de données reçu (STiK, "NDB"). len en 0x08. */
typedef struct ndb
{
    char *ptr;                  /* 0x00, bloc à libérer avec KRfree */
    char *ndata;                /* 0x04, début des données */
    unsigned short len;         /* 0x08 */
    struct ndb *next;           /* 0x0a, non utilisé par le module */
} NDB;

/* Codes d'erreur de la pile (E_*, TRANSPRT.H). */
#define E_NORMAL 0
#define E_OBUFFULL -1
#define E_NODATA -2             /* pas de données disponibles */
#define E_EOF -3                /* fin de transmission du correspondant */
#define E_RRESET -4             /* RST reçu */
#define E_UA -5
#define E_NOMEM -6
#define E_REFUSE -7             /* connexion refusée */
#define E_BADSYN -8
#define E_BADHANDLE -9
#define E_LISTEN -10
#define E_NOCCB -11
#define E_NOCONNECTION -12
#define E_CONNECTFAIL -13
#define E_BADCLOSE -14
#define E_USERTIMEOUT -15
#define E_CNTIMEOUT -16
#define E_CANTRESOLVE -17       /* nom introuvable */
#define E_BADDNAME -18
#define E_LOSTCARRIER -19
#define E_NOHOSTNAME -20
#define E_DNSWORKLIMIT -21
#define E_NONAMESERVER -22
#define E_DNSBADFORMAT -23
#define E_UNREACHABLE -24       /* destination injoignable */
#define E_DNSNOADDR -25
#define E_LOCKED -26

/* Table de fonctions du pilote STiK ou STinG (TRANSPORT_TCPIP), pointeur en
   0x2c344. Seuls les appels utilisés par CAB.OVL ont leur prototype. */
typedef struct
{
    char *module;               /* 0x00 */
    char *author;               /* 0x04 */
    char *version;              /* 0x08, chaîne "00.00" */
    char *(*KRmalloc)();        /* 0x0c */
    void (*KRfree)();           /* 0x10, libère un bloc */
    long (*KRgetfree)();        /* 0x14 */
    char *(*KRrealloc)();       /* 0x18 */
    char *(*get_err_text)();    /* 0x1c, (short code), texte d'une erreur */
    char *(*getvstr)();         /* 0x20, (char *nom), valeur d'une variable */
    short (*carrier_detect)();  /* 0x24 */
    short (*TCP_open)();        /* 0x28, (long ip, short port, short tos, short buffer) */
    short (*TCP_close)();       /* 0x2c, (short handle, short mode, short *result) */
    short (*TCP_send)();        /* 0x30, (short handle, void *buf, short len) */
    short (*TCP_wait_state)();  /* 0x34 */
    short (*TCP_ack_wait)();    /* 0x38 */
    short (*UDP_open)();        /* 0x3c */
    short (*UDP_close)();       /* 0x40 */
    short (*UDP_send)();        /* 0x44 */
    short (*CNkick)();          /* 0x48 */
    short (*CNbyte_count)();    /* 0x4c, (short handle) */
    short (*CNget_char)();      /* 0x50 */
    NDB *(*CNget_NDB)();        /* 0x54, (short handle) */
    short (*CNget_block)();     /* 0x58, (short handle, void *buf, short len) */
    void (*housekeep)();        /* 0x5c */
    short (*resolve)();         /* 0x60, (char *nom, char **rnom, long *liste, short n) */
} TPL;

/* Pile détectée par init_tcp : 1 pour la pile trouvée, 0 sinon. */
extern short tcp_is_stik;           /* 0x22436, STiK ou STinG */
extern short tcp_is_iconnect;       /* 0x22438, IConnect */

/* Noms de la pile, définis dans OVL.C. Dans le binaire, tcp_transport est en
   0x22432 et tcp_version en 0x22432 - 4 = 0x2242e (OVL.C les place dans
   l'ordre inverse). */
extern char *tcp_transport;
extern char *tcp_version;

extern TPL *stik_tpl;               /* 0x2c344, posé par stik_lib_init */

int init_tcp(void);                                         /* 0x16128 */
/* Les int sont de 16 bits avec Pure C. */
int X_TCP_open(long ip, int port, int tos, int buffer);     /* 0x16162 */
int X_TCP_send(int handle, char *buf, int size);            /* 0x161bc */
int X_TCP_close(int handle, int mode);                      /* 0x16210 */
char *X_get_err_text(int code);                             /* 0x16258 */
int X_resolve(char *name, char **real_name, int n, long *out);      /* 0x16294 */
int X_CNbyte_count(int handle);                             /* 0x162ee */
int X_CNget_block(int handle, char *buf, int size);         /* 0x1631e */
char *X_getvstr(char *name);                                /* 0x1636c */

int open_connection(char *server, int port, int tos);       /* 0x1305c */
int my_resolver(char *name, long *ip);                      /* 0x13104 */

#endif
