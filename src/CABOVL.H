/*
 * CABOVL.H - interface entre CAB et CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL, puis aligné sur l'interface
 * officielle des modules de CAB : ATARI/cab15/CAB/DOC/PROGRAMM.ING/MODULE.H
 * et INIT.C (CABINFO correspond à browser_info_t, OVLFUNCS à url_methods_t,
 * ovl_entry à init_module). Les cases réservées de CAB 1.5 reçoivent ici les
 * extensions de CAB 2.70 (cookies, If-Modified-Since).
 */

#ifndef CABOVL_H
#define CABOVL_H

#define OVL_VERSION 0x56800L        /* V5.68 en BCD */
#define OVL_DATE 0x20000119L        /* 19 janvier 2000 en BCD */
#define CAB_VERSION_2_70 0x30323730L    /* "0270" en ASCII */

/* Services que le module déclare à CAB (valeur renvoyée par ovl_entry). */
#define SUPPORT_PEXEC 1
#define SUPPORT_HTTP 2
#define SUPPORT_FTP 4
#define SUPPORT_GOPHER 8
#define SUPPORT_WAIS 16
#define SUPPORT_MAILTO 32
#define SUPPORT_NNTP 64
#define SUPPORT_TELNET 128
#define OVL_SUPPORT (SUPPORT_PEXEC | SUPPORT_HTTP | SUPPORT_MAILTO)

/* Codes de cab->progress (msg_status). 0x3039 est une extension de CAB.OVL
   qui annonce la taille du document (non vérifié dans CAB). */
#define STATUS_CONNECTING_HOST 1
#define STATUS_RECEIVING_DATA 2     /* value = octets reçus */
#define STATUS_WAITING_RESPONSE 3
#define STATUS_RESOLVING_HOST 4
#define STATUS_SENDING_REQUEST 5
#define STATUS_FORMATTING_TEXT 6
#define STATUS_LOADING_IMAGE 7
#define STATUS_WAITINGFORDATA 10    /* value = secondes restantes */

/* Structure passée par CAB à l'entrée du module (browser_info_t).
   Pointeur en 0x245a6. Les int sont de 16 bits avec Pure C. */
typedef struct
{
    int *aes_control;               /* 0x00 */
    int *aes_global;                /* 0x04 */
    int *aes_intin;                 /* 0x08 */
    int *aes_intout;                /* 0x0c */
    long *aes_addrin;               /* 0x10 */
    long *aes_addrout;              /* 0x14 */
    void (*aes_crystal)(void);      /* 0x18, initialise l'AES */
    void (*messages)(short *msgbuf);    /* 0x1c, aes_messages : messages AES pour CAB */
    void (*event)(long code);       /* 0x20, msg_error : affiche une erreur, code négatif */
    void (*progress)(long code, long value);    /* 0x24, msg_status : état du chargement */
    long (*gem_events)(long duration);  /* 0x28, aes_events : -1 si arrêt demandé */
    long (*alert_box)(long button, long msg);   /* 0x2c */
    void (*begin_pexec)(void);      /* 0x30 */
    void (*end_pexec)(void);        /* 0x34 */
    int (*clear_cache)(long size);  /* 0x38, 0 si la place est faite, -1 sinon */
    int (*new_filename)(char *url, char **file);    /* 0x3c, new_url : 0 si CAB donne un nom */
    int (*ask_user)(long msg, char **answer);   /* 0x40, 1 si valider, 0 si annuler, -1 si erreur */
    long basepage;                  /* 0x44, base page du module */
    short *reload;                  /* 0x48, reloadflag : 1 si l'utilisateur recharge */
    long version;                   /* 0x4c, version de CAB, ASCII (V1.30 vaut 0x30313330) */
    char **proxy;                   /* 0x50, proxy_url : FTP, HTTP, WAIS, Gopher, News, no_proxy, SMTP, NNTP */
    void (*online)(int state);      /* 0x54, signale à CAB l'état en ligne */
    void (*cookie)(char *url, char *name, char *value, char *expires,
                   char *path, char *domain, int secure);   /* 0x58, extension CAB 2.70 */
    long reserved[2];               /* 0x5c */
} CABINFO;

/* Table de fonctions que le module remplit pour CAB (url_methods_t). */
typedef struct
{
    int (*restore_module)();        /* 0x00, restore */
    int (*get_url)();               /* 0x04 */
    int (*get_url_info)();          /* 0x08 */
    int (*infos)();                 /* 0x0c, get_version : auteur, version BCD, date BCD */
    int (*post)();                  /* 0x10 */
    int (*mailto)();                /* 0x14, non renseigné par CAB.OVL */
    int (*get_url_if_modified)();   /* 0x18, extension */
    int (*get_url_cookie)();        /* 0x1c, extension CAB 2.70 */
    int (*get_url_ifmod_cookie)();  /* 0x20, extension CAB 2.70 */
    int (*post_cookie)();           /* 0x24, extension CAB 2.70 */
} OVLFUNCS;

extern CABINFO *cab;                /* 0x245a6 */
extern long cab_version;            /* 0x21214, copie de cab->version */

#endif
