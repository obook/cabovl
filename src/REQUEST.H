/*
 * REQUEST.H - état de la requête HTTP en cours (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. Variable globale "request"
 * en 0x2578e (le nom vient de la chaîne "request.file_handle").
 * Les champs sont placés par leur offset. Les zones non décrites sont du
 * remplissage, les noms f... sont des champs dont le rôle n'est pas déterminé.
 */

#ifndef REQUEST_H
#define REQUEST_H

typedef struct
{
    short handle;               /* 0x000, connexion TCP, négatif si erreur */
    char *file;                 /* 0x002, fichier de destination */
    short cached;               /* 0x006, 1 si le document est en cache */
    short file_handle;          /* 0x008, -1 si aucun fichier ouvert */
    char pad_00a[0x58];
    char scheme[10];            /* 0x062, protocole de l'URL */
    char server[0x100];         /* 0x06c, serveur ou proxy à contacter */
    char host[0x100];           /* 0x16c, serveur de l'URL */
    short port;                 /* 0x26c */
    char pad_26e[6];
    char *url;                  /* 0x274, URL complète */
    char *post;                 /* 0x278, données du formulaire */
    char *ctype;                /* 0x27c, Content-type des données */
    short method;               /* 0x280, 1 GET, 2 POST, 3 HEAD */
    long ifmod;                 /* 0x282, date pour If-Modified-Since */
    char path[0x400];           /* 0x286, chemin de l'URL */
    char last_write[0x400];     /* 0x686, dernière URL écrite */
    char last_read[0x400];      /* 0xa86, dernière URL lue */
    short header_state;         /* 0xe86 */
    char encoding[0x100];       /* 0xe88, Content-Encoding */
    long content_length;        /* 0xf88, -1 si inconnue */
    char content_type[0x100];   /* 0xf8c, Content-Type */
    long date;                  /* 0x108c, Date, -1 si absente */
    long expires;               /* 0x1090, Expires, -1 si absente */
    long last_modified;         /* 0x1094, Last-Modified, -1 si absente */
    char location[0x100];       /* 0x1098, Location */
    long double mime_version;   /* 0x1198, 10 octets avec Pure C */
    char pragma[0x100];         /* 0x11a2, Pragma */
    char server_hdr[0x100];     /* 0x12a2, Server */
    char www_auth[0x100];       /* 0x13a2, WWW-Authenticate */
    char status_text[0x100];    /* 0x14a2, ligne d'état après "HTTP/x.y " */
    short proxy_flag;           /* 0x15a2 */
    char proxy[0x100];          /* 0x15a4 */
    short proxy_port;           /* 0x16a4 */
    char *newname_ptr;          /* 0x16a6, nom donné par CAB pour une redirection */
    char redirect[0x100];       /* 0x16aa, URL de redirection */
    char newname[0x400];        /* 0x17aa, fichier de destination */
    short redirect_flag;        /* 0x1baa, 1 si la requête doit être refaite */
    long retcode;               /* 0x1bac */
    long bytes;                 /* 0x1bb0 */
} REQUEST;

extern REQUEST request;         /* 0x2578e */

#endif
