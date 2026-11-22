/*
 * HTTP_GET.C - requêtes HTTP de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C). Les noms
 * http_get, restore_module, get_url, get_url_info, post, get_url_if_modified,
 * get_url_cookie, get_url_ifmod_cookie et post_cookie viennent des chaînes
 * de trace. send_request, http_loop, ask_auth et hz200 sont des noms de
 * reconstitution. Le nom du module d'origine pour ask_auth est inconnu.
 * Non compilé avec Pure C, à vérifier.
 *
 * Adresses : restore_module 0x10590, get_url_info 0x10788, get_url 0x107c2,
 * post 0x10800, get_url_if_modified 0x1083a, get_url_cookie 0x1087c,
 * get_url_ifmod_cookie 0x108c4, post_cookie 0x10910, ask_auth 0x1157c,
 * http_get 0x1232c, http_loop 0x12266, send_request 0x13226, hz200 0x1974a.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "CONFIG.H"
#include "CABOVL.H"
#include "REQUEST.H"
#include "HEADER.H"
#include "LOG.H"
#include "TCP.H"
#include "GUI.H"
#include "REPORT.H"

#define BANNER "- ***************************************************\n"

#define HTTP_GET 1
#define HTTP_POST 2

REQUEST request;                    /* 0x2578e */

short ask_offline = 1;              /* 0x21218, 1 : questionner si la liaison est hors ligne */
short first_request = 1;            /* 0x2121a, 1 : la première requête n'a pas de référent */
short lazy_events = 0;              /* 0x22436, 1 : événements GEM seulement sans données reçues */
short use_progress = 0;             /* 0x22438, 1 : signale l'avancement à CAB */

static char req_buf[0x800];         /* 0x2a506, requête envoyée au serveur */

extern char ck_tmpname[];           /* OVL.C */
extern char auth_cred[], auth_host[], auth_b64[];   /* OVL.C */
extern char user_agent[];           /* OVL.C */
extern char *ck_current;            /* COOKIE.C */

/* Appels GEMDOS et MiNT (bibliothèque Pure C). */
extern long Supexec(long (*f)());
extern long Fcreate(char *name, short attr);
extern short Fclose(short handle);
extern short Fdelete(char *name);
extern long Fwrite(short handle, long count, void *buf);
extern long Fxattr(short flag, char *name, void *xattr);    /* 0x1be4a, MiNT */
extern long Fdatime(void *timeptr, short handle, short wflag);  /* 0x1bdf0 */

/* Autres modules de CAB.OVL. */
extern int SendCookie(char *host, char *path, short max, char *out);    /* COOKIE.C */
extern void ClearCookie(void);                      /* COOKIE.C */
extern int CheckCookieDate(void);                   /* COOKIE.C */
extern int SendSetup(REQUEST *req, char *name);     /* SETUP.C */
extern int SendIcon(char *name, char *file);        /* SENDICON.C */
extern void split_URL(char *url, REQUEST *req);     /* URL.C, 0x159f2 */
extern int http_proxy_cfg(char *host, short *port, short size);     /* URL.C, 0x15d80 */
extern char *skip_blanks(char *s);                  /* UTIL.C */
extern int tobase64(char *in, char *out, short size);   /* MAILTO.C, 0x11624, 0 si erreur */
extern int mailto(char *addr, char *ctype, char *post); /* MAILTO.C, 0x11768 */
extern void dns_drop_last(void);                    /* TCP.C, 0x131d8 */
extern char *escape_string(char *s);                /* MAILTO.C, 0x12212 */
extern int mem_check(void);                         /* contrôle mémoire, 0x1c6fe */

/* Contrôle mémoire : le fichier et la ligne de l'appelant sont mémorisés. */
extern int NewMemBlk(MEMBLK *blk, long n, long size, char *file, long line);    /* 0x1c74c */
extern void FreeMemBlk(MEMBLK *blk, char *file, long line);                     /* 0x1c880 */

/* Lit l'horloge à 200 Hz du système, en mode superviseur. */
static long hz200_read()
{
    return *(long *)0x4baL;
}

long hz200()
{
    return Supexec(hz200_read);
}

/* Arrête l'application du module et remet CAB dans son état d'origine. */
int restore_module()
{
    trace(BANNER);
    trace("> restore_module\n");
    cab->proxy = 0;                 /* le binaire y place la table de proxy statique en 0x24586 */
    ck_current = 0;
    CheckCookieDate();
    gem_cleanup();
    log_end();
    log_close();
    mem_check();
    return 0;
}

/* Interface demandée par CAB mais non gérée : renvoie -1. */
int get_url_info(a, b, c, d)
long a;
long b;
long c;
long d;
{
    trace(BANNER);
    trace("> get_url_info\n", a, b, c, d);
    return -1;
}

/* Demande l'identifiant et le mot de passe de l'utilisateur pour le serveur
   de la requête, les code en base64 et les mémorise. Renvoie 1 si l'utilisateur
   valide, 0 sinon. */
int ask_auth(req)
REQUEST *req;
{
    char *a;
    char *b;

    auth_cred[0] = 0;
    auth_b64[0] = 0;
    auth_host[0] = 0;

    if (ask_authentification(req->host, &a, &b) != 7)
    {
        trace("~ User do not use AUTH_VALID button\n");
        return 0;
    }
    if (tobase64(auth_cred, auth_b64, 0x100) == 0)
    {
        trace("! ERROR in Codebase64\n");
        return 0;
    }
    strncat(auth_host, req->host, 0x7e);
    trace("~ Restricted access on  = <%s>\n", auth_host);
    trace("~ Authentification      = <%s>\n", auth_cred);
    trace("~ Coded on base64       = <%s>\n", auth_b64);
    return 1;
}

/* Construit la requête HTTP 1.0 et l'envoie. via_proxy vaut 1 si le serveur
   contacté est un proxy : la requête porte alors l'URL complète. Renvoie 0,
   ou le code d'erreur de l'envoi (la connexion est alors fermée). */
static short send_request(req, via_proxy)
REQUEST *req;
short via_proxy;
{
    short r = 0;
    char date[50];
    char len[24];
    char *tok;
    time_t t;

    trace("~ Creating header...\n");
    t = (time_t)req->ifmod;
    strftime(date, 50, "%a, %d %b %Y %X GMT", gmtime(&t));

    req_buf[0] = 0;
    if (req->method == HTTP_GET)
        strncat(req_buf, "GET ", 0x7ff);
    else if (req->method == HTTP_POST)
        strncat(req_buf, "POST ", 0x7ff);
    else
        strncat(req_buf, "HEAD ", 0x7ff);

    /* Les espaces de l'adresse deviennent %20. */
    tok = strtok(via_proxy == 1 ? req->url : req->path, " ");
    while (tok != 0)
    {
        strncat(req_buf, tok, 0x7ff);
        tok = strtok(0, " ");
        if (tok != 0)
            strncat(req_buf, "%20", 0x7ff);
    }

    strncat(req_buf, " HTTP/1.0\r\n", 0x7ff);
    strncat(req_buf, "Host: ", 0x7ff);
    strncat(req_buf, req->host, 0x7ff);
    strncat(req_buf, "\r\n", 0x7ff);
    strncat(req_buf, "Pragma: no-cache\r\n", 0x7ff);
    strncat(req_buf, "Cache-Control: no-cache\r\n", 0x7ff);
    strncat(req_buf, "User-Agent: ", 0x7ff);
    strncat(req_buf, user_agent, 0x7ff);
    strncat(req_buf, "\r\n", 0x7ff);
    strncat(req_buf, "Accept: */*\r\n", 0x7ff);

    if (req->ifmod > 42 && *cab->reload == 0)
    {
        strncat(req_buf, "If-Modified-Since: ", 0x7ff);
        strncat(req_buf, date, 0x7ff);
        strncat(req_buf, "\r\n", 0x7ff);
    }

    if (req->method == HTTP_POST)
    {
        strncat(req_buf, "Content-Length: ", 0x7ff);
        sprintf(len, "%ld", (long)strlen(req->post));
        strncat(req_buf, len, 0x7ff);
        strncat(req_buf, "\r\n", 0x7ff);
        strncat(req_buf, "Content-type: ", 0x7ff);
        strncat(req_buf, req->ctype, 0x7ff);
        strncat(req_buf, "\r\n", 0x7ff);
    }

    if (req->last_read[0] != 0)
    {
        strncat(req_buf, "Referer: ", 0x7ff);
        strncat(req_buf, req->last_read, 0x7ff);
        strncat(req_buf, "\r\n", 0x7ff);
    }

    if (strcmp(auth_host, req->host) == 0)
    {
        strncat(req_buf, "Authorization: Basic ", 0x7ff);
        strncat(req_buf, auth_b64, 0x7ff);
        strncat(req_buf, "\r\n", 0x7ff);
    }

    SendCookie(req->host, req->path, 0x7ff, req_buf);
    strncat(req_buf, "\r\n", 0x7ff);

    trace("~ Send my header: <\n");
    trace_str("~", req_buf);
    trace_str("~", ">\n");

    if (use_progress == 1)
        (*cab->progress)(5L, 0L);
    r = X_TCP_send(req->handle, req_buf, (short)strlen(req_buf));
    if (use_progress == 1)
        (*cab->progress)(3L, 0L);

    if (r == 0 && req->method == HTTP_POST)
    {
        r = X_TCP_send(req->handle, req->post, (short)strlen(req->post));
        trace("~ Sending form <%s>, <%d> bytes using the POST method : <\n%s>\n",
              escape_string(req->ctype), (int)strlen(req->post), req->post);
    }
    if (r != 0)
    {
        trace("! TCP_send() to <%s> returns: <%s>\n", req->server, X_get_err_text(r));
        X_TCP_close(req->handle, 1);
    }
    return r;
}

/* Fait une requête HTTP et écrit la réponse dans le fichier de la requête.
   Renvoie 0 si le document est reçu, sinon un code d'erreur négatif. Si le
   serveur redirige, request.redirect_flag et request.redirect décrivent la
   nouvelle requête. Les codes passés à cab->event sont des codes d'état de CAB. */
long http_get(url, post, method, ifmod, ctype, file)
char *url;
char *post;
short method;
long ifmod;
char *ctype;
char *file;
{
    MEMBLK blk;
    HEADERBUF *hb = (HEADERBUF *)&blk;
    char xattr[0x34];
    short words[2];
    struct tm *tm;
    long code = 0;              /* valeur renvoyée */
    long total = 0;             /* octets reçus */
    long last_progress = 0;
    long deadline;              /* fin de l'attente d'une réponse */
    long next_event;            /* prochain appel des événements GEM */
    long cached;
    long n;
    short done = 0;             /* 0 : continuer, 1 : fin normale, 2 à 4 : erreur */
    short proxy = 0;
    short avail = 0;
    short take;
    short got = 0;
    short counter = 3;
    short choice;
    short r;

    memset(&blk, 0, sizeof(blk));

    if (request.redirect_flag == 0)
    {
        request.newname[0] = 0;
        strncat(request.newname, file, 0x3ff);
    }
    request.redirect_flag = 0;

    trace("- URL   : <");
    trace_str("-", url);
    trace_str("-", ">\n");

    if (strnicmp(url, "mailto:", 7) == 0)
    {
        mailto(skip_blanks(url + 7), ctype, post);
        goto finish;
    }

    if (file[0] == 0)
        file = ck_tmpname;

    request.handle = -0x29a;
    request.url = url;
    request.post = post;
    request.ctype = ctype;
    request.file = file;
    request.method = method;
    request.ifmod = ifmod;

    /* Référent : l'URL lue lors de la requête précédente. */
    if (first_request == 1)
    {
        request.last_write[0] = 0;
        request.last_read[0] = 0;
        first_request = 0;
    }
    else
    {
        request.last_read[0] = 0;
        strncat(request.last_read, request.last_write, 0x400);
    }
    request.last_write[0] = 0;
    strncat(request.last_write, request.url, 0x400);
    if (strchr(request.last_write, '?') != 0)
        *strchr(request.last_write, '?') = 0;
    trace("~ Write last URL <%s>.\n", request.last_write);
    trace("~ Read last URL  <%s>.\n", request.last_read);

    /* Le fichier est-il déjà en cache ? */
    cached = Fxattr(0, file, xattr);
    if (cached < 0)
    {
        request.cached = 0;
        trace("~ http_get: File <%s>=<%ld> is not cached\n", file, cached);
    }
    else
    {
        request.cached = 1;
        trace("~ http_get: File <%s>=<%ld> is cached\n", file, cached);
    }

    if (strnicmp(request.url, "http://cab.ovl", 14) == 0)
    {
        code = SendSetup(&request, request.url + 15);
        goto finish;
    }

    /* Liaison hors ligne : demande à l'utilisateur. */
    if (conf.online == 0)
    {
        (*cab->event)(-0x138L);
        if (ask_offline == 1)
            choice = my_form_alerte("Offline", get_rsc_string(1));
        else
            choice = 1;
        if (choice == 2)
            conf.online = 1;
        else
        {
            if (choice == 3)
                ask_offline = 0;
            code = -0x138L;
            goto finish;
        }
    }

    code = 0;
    split_URL(url, &request);
    request.host[0] = 0;
    strncat(request.host, request.server, 0xff);
    trace("- Server: <%s>\n", request.host);
    trace("- URI   : <%s>\n", request.path);

    /* Adresses /internal-gopher-... : icône fournie par le module. */
    n = SendIcon(request.path, file);
    if (n > 0)
    {
        code = 0;
        goto finish;
    }

    if (!NewMemBlk(&blk, 0x2001L, 1L, __FILE__, (long)__LINE__))
    {
        (*cab->event)(-0x27L);
        report_error(&request, request.file, 3, &request.file_handle);
        goto finish;
    }

connect:
    hb->used = 0;
    proxy = http_proxy_cfg(request.proxy, &request.proxy_port, 0x100);
    if (proxy == 1)
    {
        trace("- Proxy : <%s>:(%d)\n", request.proxy, request.proxy_port);
        request.server[0] = 0;
        strncat(request.server, request.proxy, 0x100);
        request.port = request.proxy_port;
    }
    request.proxy_flag = 1;
    deadline = hz200() + (long)conf.timeout * 200L;

    request.handle = open_connection(request.server, request.port, 0);
    trace("~ open_connection = (%i) on <", request.handle);
    trace_str("~", request.server);
    trace_str("~", ">\n");
    if (request.handle < 0)
    {
        /* Le proxy ne répond pas : nouvel essai sans proxy. */
        if (proxy == 1)
        {
            split_URL(url, &request);
            request.handle = open_connection(request.server, request.port, 0);
            proxy = 0;
        }
        if (request.handle < 0)
        {
            (*cab->event)(-0x258L);
            if (request.handle != E_UNREACHABLE && request.handle != E_CANTRESOLVE)
                ReportRequestText(&request, X_get_err_text(request.handle));
            goto cleanup;
        }
    }

    r = send_request(&request, proxy);
    if (r != 0)
    {
        (*cab->event)(-0x137L);
        ReportRequestText(&request, X_get_err_text(r));
        goto cleanup;
    }

    next_event = hz200() + (long)(conf.event_interval / 1000.0 * 200.0);
    request.header_state = 1;
    hb->used = 0;
    total = 0;
    last_progress = 0;
    got = 0;
    counter = 3;
    request.file_handle = -1;
    done = 0;

    while (done == 0)
    {
        avail = X_CNbyte_count(request.handle);
        if (avail < 0)
            trace("~ CNbyte_count() returns <%s>\n", X_get_err_text(avail));

        if (avail > 0)
        {
            take = (short)(hb->len - hb->used - 1);
            if (avail < take)
                take = avail;
            deadline = hz200() + (long)conf.timeout * 200L;
            counter = 0;
            trace("~ bytes_available = <%d>, take = <%d> (header.len - header.used) = <%ld>\n",
                  avail, take, hb->len - hb->used);

            if (take + hb->used <= 0x2000L)
                got = X_CNget_block(request.handle, hb->data + hb->used, take);
            else
            {
                (*cab->event)(-0x13aL);
                report_error(&request, request.file, 8, &request.file_handle);
                done = 2;
            }
            if (got != 0)
                trace("~ X_CNget_block = <%d> bytes read\n", got);

            if (got > 0)
            {
                hb->used += got;
                total += hb->used;      /* l'original ajoute used et non got */

                if (request.header_state == 1)
                {
                    request.header_state = HeaderIsReady(&request, hb);

                    if (request.header_state == -3)
                    {
                        (*cab->event)(-0x13bL);
                        trace("! HTTP missed by server administrator, get raw data.\n");
                        request.header_state = 200;
                    }
                    else if (request.header_state == 0x191)
                    {
                        trace("~ Document protected\n");
                        if (ask_auth(&request) == 1)
                        {
                            /* Nouvel essai avec l'identifiant saisi. */
                            if (request.file_handle > 0)
                            {
                                Fclose(request.file_handle);
                                request.file_handle = -1;
                            }
                            Fdelete(file);
                            X_TCP_close(request.handle, 0);
                            goto connect;
                        }
                        trace("~ Authentification aborted\n");
                        Fdelete(file);
                        code = -0x24;
                        done = 2;
                    }
                    else if (request.header_state == 0x130)
                    {
                        trace("~ File not modified\n");
                        code = 0;
                        done = 2;
                    }
                    else if (request.header_state == -2)
                    {
                        (*cab->event)(-0x27L);
                        report_error(&request, request.file, 3, &request.file_handle);
                        done = 2;
                    }
                    else if (request.header_state != 1
                             && request.header_state != 200
                             && request.header_state != 201
                             && request.header_state != 301
                             && request.header_state != 302)
                    {
                        if (request.content_type[0] == 0)
                        {
                            (*cab->event)(-0x21L);
                            ReportRequestText(&request, request.status_text);
                            done = 2;
                        }
                        else
                            request.header_state = 200;
                    }

                    if (request.header_state == 200 || request.header_state == 201
                        || request.header_state == 301 || request.header_state == 302)
                    {
                        /* Début du document : crée le fichier et y écrit
                           la partie du corps déjà reçue. */
                        request.file_handle = (short)Fcreate(request.file, 0);
                        if (request.file_handle < 0)
                        {
                            trace("! Error, request.file_handle = <%d>\n", request.file_handle);
                            (*cab->event)(-0x41L);
                            report_error(&request, request.file, 9, &request.file_handle);
                            done = 2;
                        }
                        total = hb->used;
                        if (request.content_length > 0)
                            (*cab->progress)(0x3039L, request.content_length);
                        if (hb->used != 0)
                        {
                            n = Fwrite(request.file_handle, hb->used, hb->data);
                            if (n < 0)
                            {
                                trace("! Fwrite error <%ld>\n", n);
                                (*cab->event)(-0x41L);
                                report_error(&request, request.file, 10, &request.file_handle);
                                hb->used = 0;
                                done = 2;
                            }
                            hb->used = 0;
                        }
                    }
                }
                else
                {
                    /* Suite du corps du document. */
                    n = Fwrite(request.file_handle, hb->used, hb->data);
                    if (n < 0)
                    {
                        trace("! Fwrite error <%ld>\n", n);
                        (*cab->event)(-0x41L);
                        report_error(&request, request.file, 5, &request.file_handle);
                        done = 2;
                    }
                    hb->used = 0;
                    if (total > 0
                        && (request.header_state == 200 || request.header_state == 201))
                    {
                        last_progress = total;
                        if (request.header_state != 1)
                            (*cab->progress)(2L, total);
                    }
                }
            }
            else if (got == E_EOF)
            {
                trace("~ X_CNget_block:EOF\n");
                done = 1;
            }
            else if (got < 0)
            {
                (*cab->event)(-0x137L);
                ReportRequestText(&request, X_get_err_text(got));
                done = 3;
            }
        }
        else if (avail == E_RRESET)
            trace("~ bytes_available==E_RRESET, do nothing..\n");
        else if (avail == E_EOF)
        {
            trace("~ bytes_available:EOF\n");
            if (total > last_progress && total > 0
                && (request.header_state == 200 || request.header_state == 201))
                (*cab->progress)(2L, total);
            done = 1;
        }
        else if (avail != E_NODATA && avail != 0)
        {
            (*cab->event)(-0x137L);
            ReportRequestText(&request, X_get_err_text(avail));
            done = 3;
        }

        /* Événements GEM, à intervalle régulier. */
        if (hz200() > next_event && !(lazy_events == 1 && avail != 0))
        {
            if ((*cab->gem_events)((long)conf.event_duration) == -1L)
            {
                trace("! aes_events : STOP icon or Esc key pressed\n");
                (*cab->event)(-0x13bL);
                if (request.file_handle > 0)
                {
                    Fclose(request.file_handle);
                    request.file_handle = -1;
                }
                Fdelete(request.file);
                total = 0;
                code = -1;
                done = 4;
            }
            next_event = hz200() + (long)(conf.event_interval / 1000.0 * 200.0);
            if (counter == 3 && done == 0)
                (*cab->progress)(10L, (deadline - hz200()) / 200L);
            else if (counter < 3)
                counter++;
        }

        if (hz200() > deadline)
        {
            trace("~ Timeout!\n");
            (*cab->event)(-0x24L);
            report_error(&request, request.file, 13, &request.file_handle);
            done = 4;
        }
    }

    if (request.handle != -0x29a)
    {
        r = X_TCP_close(request.handle, 0);
        trace("~ TCP_close() returns <%s>\n", X_get_err_text(r));
    }

    /* Le fichier prend la date du document (Last-Modified, sinon Date). */
    if (request.file_handle > 0)
    {
        if (request.last_modified > 0)
        {
            request.last_modified++;
            tm = gmtime((time_t *)&request.last_modified);
        }
        else
            tm = gmtime((time_t *)&request.date);
        words[0] = tm->tm_sec / 2 + (tm->tm_min << 5) + (tm->tm_hour << 11);
        words[1] = tm->tm_mday + ((tm->tm_mon + 1) << 5) + ((tm->tm_year - 80) << 9);
        Fdatime(words, request.file_handle, 1);
        r = Fclose(request.file_handle);
        request.file_handle = -1;
        if (r < 0)
        {
            (*cab->event)(-0x41L);
            report_error(&request, request.file, 7, &request.file_handle);
        }
    }

cleanup:
    FreeMemBlk(&blk, __FILE__, (long)__LINE__);

    /* Redirection : le serveur a donné une nouvelle adresse. */
    if (strlen(request.location) > 0 && done == 1)
    {
        request.redirect[0] = 0;
        if (strnicmp(request.location, "http://", 7) != 0)
        {
            strncat(request.redirect, "http://", 0xff);
            strncat(request.redirect, request.host, 0xff);
            if (request.location[0] != '/')
                strncat(request.redirect, "/", 0xff);
        }
        strncat(request.redirect, request.location, 0xff);
        trace("~ New Location <");
        trace_str("~", request.redirect);
        trace_str("~", ">\n");

        if ((*cab->new_filename)(request.redirect, &request.newname_ptr) == 0)
        {
            request.newname[0] = 0;
            strncat(request.newname, request.newname_ptr, 0x3ff);
            if (request.newname[0] == 0)
                strncat(request.newname, ck_tmpname, 0x3ff);
        }
        else
        {
            trace("! browser can not give a new filename, set as old <%s>\n", request.newname);
            Fdelete(request.file);
        }
        request.redirect_flag = 1;
    }
    else
        request.redirect_flag = 0;

finish:
    if (code != 0)
        dns_drop_last();
    ClearCookie();
    request.retcode = code;
    request.bytes = total;
    return code;
}

/* Lance http_get, et la relance tant que le serveur redirige. Après la
   première requête, les données de formulaire sont vides et un POST devient
   un GET. */
static long http_loop(url, post, method, ifmod, ctype, file)
char *url;
char *post;
short method;
long ifmod;
char *ctype;
char *file;
{
    long rc = 0;

    request.redirect_flag = 1;
    request.retcode = 0;
    request.bytes = 0;
    request.redirect[0] = 0;
    strncat(request.redirect, url, 0x100);
    request.newname[0] = 0;
    strncat(request.newname, file, 0x400);

    while (request.redirect_flag == 1)
    {
        trace("~ Call Reception for <%s>\n", request.redirect);
        rc = http_get(request.redirect, post, method, ifmod, ctype, request.newname);
        post = "";
        ctype = post;
        if (method == HTTP_POST)
            method = HTTP_GET;
    }
    trace("~ END : return_code (%ld) for file contain (%ld) bytes\n",
          request.retcode, request.bytes);
    if (request.bytes > 0)
        (*cab->progress)(6L, 0L);
    return rc;
}

/* Interface demandée par CAB : charge url dans file. */
long get_url(url, file)
char *url;
char *file;
{
    trace(BANNER);
    trace("> get_url\n");
    return http_loop(url, "", HTTP_GET, 0L, "", file);
}

/* Envoie des données de formulaire par POST. */
long post(url, data, ctype, file)
char *url;
char *data;
char *ctype;
char *file;
{
    trace(BANNER);
    trace("> post \n");
    return http_loop(url, data, HTTP_POST, 0L, ctype, file);
}

/* Charge url si le document a changé depuis *ifmod. */
long get_url_if_modified(url, file, ifmod)
char *url;
char *file;
long *ifmod;
{
    trace(BANNER);
    trace("> get_url_if_modified\n");
    return http_loop(url, "", HTTP_GET, *ifmod, "", file);
}

/* Comme get_url, avec le cookie que CAB a mémorisé pour cette adresse. */
long get_url_cookie(url, file, cookie)
char *url;
char *file;
char *cookie;
{
    trace(BANNER);
    trace("> get_url_cookie\n");
    ck_current = cookie;
    return http_loop(url, "", HTTP_GET, 0L, "", file);
}

long get_url_ifmod_cookie(url, file, ifmod, cookie)
char *url;
char *file;
long *ifmod;
char *cookie;
{
    trace(BANNER);
    trace("> get_url_ifmod_cookie\n");
    ck_current = cookie;
    return http_loop(url, "", HTTP_GET, *ifmod, "", file);
}

long post_cookie(url, data, ctype, file, cookie)
char *url;
char *data;
char *ctype;
char *file;
char *cookie;
{
    trace(BANNER);
    trace("> post_cookie\n");
    ck_current = cookie;
    return http_loop(url, data, HTTP_POST, 0L, ctype, file);
}
