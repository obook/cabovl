/*
 * HEADER.H - analyse des en-têtes HTTP (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL.
 */

#ifndef HEADER_H
#define HEADER_H

#include "REQUEST.H"

/* Tampon où s'accumule la réponse du serveur. */
typedef struct
{
    char *data;                 /* 0x00 */
    char *end;                  /* 0x04, fin de la zone à parcourir */
    long len;                   /* 0x08, capacité */
    long used;                  /* 0x0c, octets présents */
    long f10;
    long f14;
} HEADERBUF;

/* Une ligne d'en-tête, 6 octets. */
typedef struct
{
    char *line;
    short info;                 /* rôle non déterminé */
} HEADENTRY;

/* Liste des lignes d'un en-tête, 40 octets alloués. */
typedef struct
{
    HEADENTRY *list;            /* 0x00 */
    long max;                   /* 0x04, 200 */
    long count;                 /* 0x08, rempli par MakeHeadlist */
    char pad[28];
} HEADLIST;

/* Bloc mémoire suivi par le module de contrôle mémoire, 24 octets. */
typedef struct
{
    void *ptr;                  /* 0x00 */
    long f04;
    long size;                  /* 0x08 */
    long f0c;
    char *file;                 /* 0x10 */
    long line;                  /* 0x14 */
} MEMBLK;

/* Valeurs de retour de HeaderIsReady : -1 tampon vide, 1 en-tête incomplet,
   -2 mémoire insuffisante, -3 en-tête illisible, sinon code HTTP. */
int HeaderIsReady(REQUEST *req, HEADERBUF *hb);     /* 0x13680 */
long MakeHeadlist(char *hdr, HEADLIST *hl, HEADERBUF *hb);  /* 0x13874 */
int parse_header(REQUEST *req, HEADLIST *hl);       /* 0x13914 */
long FindHeader(char *name, HEADLIST *hl);          /* 0x13e34 */

#endif
