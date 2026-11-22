/*
 * HEADER.C - analyse des en-têtes HTTP de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C). Les noms
 * HeaderIsReady, MakeHeadlist et parse_header viennent des chaînes de
 * trace du binaire. FindHeader est un nom de reconstitution.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HEADER.H"
#include "LOG.H"

extern char *trim_token(char *s);                   /* UTIL.C */
extern unsigned long parse_date(char *s);           /* UTIL.C */
extern int AddCookie();                             /* COOKIE.C */
extern void clear_info(REQUEST *req);               /* 0x1598c */

/* Contrôle mémoire : le fichier et la ligne de l'appelant sont mémorisés. */
extern int NewMemBlk(MEMBLK *blk, long n, long size, char *file, long line);    /* 0x1c74c */
extern void FreeMemBlk(MEMBLK *blk, char *file, long line);                     /* 0x1c880 */

/* Cherche la première ligne qui commence par name, sans tenir compte de la
   casse. S'arrête à la première ligne vide. Renvoie son indice, ou -1. */
long FindHeader(name, hl)
char *name;
HEADLIST *hl;
{
    long i;

    for (i = 0; i < hl->count; i++)
    {
        if (hl->list[i].line[0] == 0)
            break;
        if (strnicmp(name, hl->list[i].line, strlen(name)) == 0)
            return i;
    }
    return -1;
}

/* Découpe l'en-tête en lignes : chaque fin de ligne est remplacée par 0 et
   le début de chaque ligne est rangé dans hl->list. L'en-tête se termine à
   la première ligne vide. Renvoie sa taille en octets, 0 en cas d'échec. */
long MakeHeadlist(hdr, hl, hb)
char *hdr;
HEADLIST *hl;
HEADERBUF *hb;
{
    char *p = hdr;
    long n = 0;

    hl->count = 0;
    if (p < hb->data || p >= hb->end || hl->max <= 0)
        return 0;
    hl->list[n++].line = p;

    while (p <= hb->end)
    {
        if (*p != '\n')
        {
            p++;
            continue;
        }
        *p++ = 0;
        if (p > hb->end)
            continue;
        if (n < hl->max)
            hl->list[n++].line = p;
        if (p < hb->end && p[0] == '\r' && p[1] == '\n')
            *p++ = 0;
        if (*p == '\n')
        {
            *p++ = 0;
            hl->count = n;
            return (long)(p - hdr);
        }
        if (n == hl->max)
            return 0;
    }
    return 0;
}

/* Lit les champs de l'en-tête et les range dans req. Renvoie le code HTTP,
   ou 0 si la ligne d'état est absente ou n'est pas en HTTP 1.0 ou 1.1. */
int parse_header(req, hl)
REQUEST *req;
HEADLIST *hl;
{
    int status = 0;
    long i;
    char v[4];
    long double x;
    char *value;

    trace("> parse_header\n");
    clear_info(req);

    i = FindHeader("HTTP/", hl);
    if (i >= 0)
    {
        req->status_text[0] = 0;
        strncat(req->status_text, trim_token(hl->list[i].line + 9), 0xff);
        trace("~ HTTP/ = <%s>\n", req->status_text);

        v[0] = 0;
        strncat(v, hl->list[i].line + 5, 3);
        x = atof(v);
        if (x == 1.0 || x == 1.1)
        {
            v[0] = 0;
            strncat(v, hl->list[i].line + 9, 3);
            status = atoi(v);
            if (status >= 400)
                trace("! HTTP error <%s>\n", req->status_text);
        }
        else
        {
            /* L'original passe ici l'adresse de l'entrée de la liste, pas la ligne. */
            trace("! HTTP/ is not 1.0 or 1.1 in <%s>\n", (char *)&hl->list[i]);
            status = 0;
        }
    }

    i = FindHeader("Content-Length:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 15);
        req->content_length = atol(value);
        trace("~ Content-Length = <%ld>\n", req->content_length);
    }

    i = FindHeader("Content-Encoding:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 17);
        req->encoding[0] = 0;
        strncpy(req->encoding, value, 0xff);
        trace("~ Content-Encoding = <%s>\n", req->encoding);
    }

    i = FindHeader("Content-Type:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 13);
        if (strchr(value, ';') != 0)
            *strchr(value, ';') = 0;
        req->content_type[0] = 0;
        strncpy(req->content_type, value, 0xff);
        trace("~ Content-Type = <%s>\n", req->content_type);
    }

    i = FindHeader("Location:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 9);
        req->location[0] = 0;
        strncpy(req->location, value, 0xff);
        trace("~ Location = <");
        trace_str("~", req->location);
        trace_str("~", ">\n");
    }

    i = FindHeader("MIME-Version:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 13);
        req->mime_version = atof(value);
        /* Le format "%f.1" est celui de l'original. */
        trace("~ MIME-Version = <%f.1>\n", req->mime_version);
    }

    i = FindHeader("Pragma:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 7);
        req->pragma[0] = 0;
        strncpy(req->pragma, value, 0xff);
        trace("~ Pragma = %s\n", req->pragma);
    }

    i = FindHeader("Server:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 7);
        req->server_hdr[0] = 0;
        strncpy(req->server_hdr, value, 0xff);
        trace("~ Server = %s\n", req->server_hdr);
    }

    i = FindHeader("WWW-Authenticate:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 16);
        req->www_auth[0] = 0;
        strncpy(req->www_auth, value, 0xff);
        trace("~ WWW-Authenticate = %s\n", req->www_auth);
    }

    i = FindHeader("Last-Modified:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 14);
        req->last_modified = (long)parse_date(value);
        trace("~ Last-Modified = %s (%ld)\n", value, req->last_modified);
    }

    i = FindHeader("Date:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 5);
        req->date = (long)parse_date(value);
        trace("~ Date = %s (%ld)\n", value, req->date);
    }

    i = FindHeader("Expires:", hl);
    if (i >= 0)
    {
        value = trim_token(hl->list[i].line + 8);
        req->expires = (long)parse_date(value);
        trace("~ Detect 'Expires' = %s (%ld)\n", value, req->expires);
    }

    /* Tous les Set-cookie de l'en-tête, pas seulement le premier. */
    for (i = 0; i < hl->count; i++)
    {
        if (hl->list[i].line[0] != 0
            && strnicmp(hl->list[i].line, "Set-cookie:", 11) == 0)
        {
            value = trim_token(hl->list[i].line + 11);
            trace("~ Detect 'Set-cookie' = <");
            trace_str("~", value);
            trace_str("~", ">\n");
            AddCookie(req, value);
        }
    }

    trace("~ parse_header return(%d)\n", status);
    return status;
}

/* Indique si l'en-tête de la réponse est complet. Si oui, l'analyse puis
   le retire du tampon. Voir HEADER.H pour les valeurs de retour. */
int HeaderIsReady(req, hb)
REQUEST *req;
HEADERBUF *hb;
{
    MEMBLK blk;
    HEADLIST hl;
    long n;
    long i;
    int status;

    memset(&blk, 0, sizeof(blk));
    memset(&hl, 0, sizeof(hl));

    trace("> HeaderIsReady ? on header buf of %ld bytes\n", hb->used);
    if (hb->used <= 0)
        return -1;

    /* Fin d'en-tête : une ligne vide, avec toutes les variantes CR et LF. */
    if (strstr(hb->data, "\r\n\r\n") == 0
        && strstr(hb->data, "\n\r\n\r") == 0
        && strstr(hb->data, "\n\n") == 0
        && strstr(hb->data, "\r\r") == 0)
        return 1;

    trace("~ HeaderIsReady: Header complete\n");

    if (!NewMemBlk(&blk, 200L, 6L, __FILE__, (long)__LINE__))
    {
        trace("! HeaderIsReady: NewMemBlk error, low memory.\n");
        FreeMemBlk(&blk, __FILE__, (long)__LINE__);
        return -2;
    }
    hl.list = (HEADENTRY *)blk.ptr;
    hl.max = 200;

    n = MakeHeadlist(hb->data, &hl, hb);
    if (n <= 0)
    {
        trace("! HeaderIsReady: No MakeHeadlist in <\n");
        trace_str("!", hb->data);
        trace_str("!", "\n>\n");
        FreeMemBlk(&blk, __FILE__, (long)__LINE__);
        return -3;
    }

    trace("~ MakeHeadlist count %ld lines (0 to %ld) in %ld bytes of header\n",
          hl.count, hl.count - 1, n);
    for (i = 0; i < hl.count; i++)
    {
        trace("~ MakeHeadlist Line (%d):<", (int)i);
        trace_str("~", trim_token(hl.list[i].line));
        trace_str("~", ">\n");
    }

    status = parse_header(req, &hl);

    /* Retire l'en-tête du tampon, le corps du document reste. */
    hb->used -= n;
    memmove(hb->data, hb->data + n, (size_t)hb->used);
    FreeMemBlk(&blk, __FILE__, (long)__LINE__);

    if (status == 0)
    {
        trace("! parse_header error <%d>\n", status);
        return -3;
    }
    return status;
}
