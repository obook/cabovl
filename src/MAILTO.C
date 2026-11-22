/*
 * MAILTO.C - envoi de courrier (liens mailto:) de CAB.OVL (V5.68)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C).
 * Adresses : tobase64 0x11624, mailto 0x11768, smtp_reply 0x11c3a,
 * Send_Commande 0x11e58, Send_Message 0x11f58, Send_Char 0x120d6,
 * url_decode 0x12190, hex_byte 0x121d2, escape_string 0x12212.
 * La fonction en 0x12266 (boucle de redirection) est http_loop, dans HTTP_GET.C.
 *
 * L'adresse de l'expéditeur et le serveur SMTP ne viennent ni de CONFIG ni
 * de CAB : ce sont les variables "EMAIL" et "SMTP_SERVER" de la pile TCP
 * (lues par getvstr, 0x1636c). Le module n'écrit aucune adresse e-mail.
 *
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "CABOVL.H"
#include "CONFIG.H"
#include "LOG.H"
#include "TCP.H"

extern int my_form_alerte(char *title, char *fmt, ...);     /* 0x15320 */
extern char *stik_error_text(short code);              /* 0x15574, texte d'une erreur TCP */
extern int ReportText(char *url, char *file, short *handle, char *text);   /* 0x149c0 */

/* Délai d'envoi d'un octet ou d'une commande, en ticks de 200 Hz. */
#define SEND_TIMEOUT 0x4240L

/* Codes de réponse SMTP utilisés. */
#define SMTP_READY 220
#define SMTP_OK 250
#define SMTP_DATA 354

/* États du dialogue SMTP. */
#define ST_HELO 1       /* attend la salutation puis la réponse à helo */
#define ST_FROM 2       /* attend la réponse à mail from */
#define ST_RCPT 3       /* attend la réponse à rcpt to */
#define ST_DATA 4       /* attend la réponse à data */
#define ST_BODY 5       /* attend la réponse au message */
#define ST_DONE 6       /* message accepté */
#define ST_ERROR 7      /* échec */

/* Variables globales du module, en 0x250e6 (handle) et 0x250e8 (mail). */
static short smtp_handle;
static struct
{
    char *server;       /* 0x00, SMTP_SERVER */
    char *from;         /* 0x04, EMAIL */
    short state;        /* 0x08 */
    char *rcpt;         /* 0x0a, destinataire */
    char *subject;      /* 0x0e, "From the WEB" par défaut */
    char *enctype;      /* 0x12, type d'encodage du corps */
    char *message;      /* 0x16, corps */
} mail;

/* Tampon de escape_string, en 0x2568d. */
static char esc_buf[257];

static char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


/* Encode src en base64 dans dst (taille size). Renvoie 1, ou 0 si dst est trop petit.
   Défauts reproduits : pas de fin de ligne entre les blocs de 76 caractères, et
   l'offset d'écriture est remplacé par la taille du bloc au lieu de s'y ajouter. */
int tobase64(src, dst, size)
char *src;
char *dst;
short size;
{
    char line[0x50];
    unsigned char *p;
    unsigned char *end;
    long v;
    short pos;
    short groups;
    short off;
    short extra;

    pos = 0;
    groups = 0;
    p = (unsigned char *)src;
    end = p + strlen(src);
    off = 0;
    trace("> tobase64 <%s>\n", src);
    if (!(p < end))
    {
        trace("~ la fin est plus que le debut\n");
        return 0;
    }
    while (end > p)
    {
        memset(line, '=', 0x50);
        pos = 0;
        groups = 0;
        for (;;)
        {
            groups++;
            if (groups >= 20 || !(end > p))
                break;
            extra = 0;
            v = (long)*p++ << 8;
            if (end > p)
            {
                v += *p++;
                extra++;
            }
            v <<= 8;
            if (end > p)
            {
                v += *p++;
                extra++;
            }
            line[pos++] = b64_table[(v & 0xfc0000L) >> 18];
            line[pos++] = b64_table[(v & 0x3f000L) >> 12];
            if (extra >= 1)
                line[pos] = b64_table[(v & 0xfc0L) >> 6];
            else
                line[pos] = '=';
            pos++;
            if (extra == 2)
                line[pos] = b64_table[v & 0x3f];
            else
                line[pos] = '=';
            pos++;
        }
        line[pos] = 0;
        pos++;
        if (size > off + pos)
        {
            memcpy(dst + off, line, (long)pos);
            off = pos;
        }
        else
        {
            trace("~ buffer de reception <%d> trop petit pour <%d> \n", size, off + pos);
            return 0;
        }
    }
    return 1;
}


/* Valeur de l'octet écrit en deux chiffres hexadécimaux en p[0], p[1]. */
static char hex_byte(p)
char *p;
{
    unsigned char hi;
    unsigned char lo;

    if ((unsigned char)p[0] >= 0x41)
        hi = (p[0] & 0xdf) - 0x37;
    else
        hi = p[0] - 0x30;
    if ((unsigned char)p[1] >= 0x41)
        lo = (p[1] & 0xdf) - 0x37;
    else
        lo = p[1] - 0x30;
    return (char)((hi << 4) + lo);
}

/* Décode les %XX de s sur place. Renvoie la fin de la chaîne décodée.
   Aucun contrôle des chiffres hexadécimaux. */
static char *url_decode(s)
char *s;
{
    short i;
    short j;

    i = 0;
    j = 0;
    while (s[j])
    {
        s[i] = s[j];
        if (s[j] == '%')
        {
            s[i] = hex_byte(s + j + 1);
            j += 2;
        }
        i++;
        j++;
    }
    s[i] = 0;
    return s + i;
}

/* Copie s dans un tampon statique en faisant précéder "\" et l'espace d'un "\"
   et en doublant "%", pour un usage dans un format de trace. Nom non vérifié.
   Défaut : le résultat n'est pas terminé par un octet nul, sauf si s est courte
   ou si le tampon contenait déjà un nul. */
char *escape_string(s)
char *s;
{
    short i;
    short j;

    j = 0;
    esc_buf[0] = 0;
    for (i = 0; s[i] && j < 0xff; i++)
    {
        if (s[i] == '\\' || s[i] == ' ')
        {
            esc_buf[j] = '\\';
            j++;
        }
        else if (s[i] == '%')
        {
            esc_buf[j] = '%';
            j++;
        }
        esc_buf[j] = s[i];
        j++;
    }
    return esc_buf;
}


/* Alerte et trace communes aux erreurs d'envoi sur la connexion. */
static void send_error(rc, name)
short rc;
char *name;
{
    trace(name, mail.server, (*stik_tpl->get_err_text)(rc));
    (*cab->event)(-0x137L);
    my_form_alerte("Mailto", "[1][Server|%s|%s][ Ok ]", mail.server, stik_error_text(rc));
}

/* Envoie un octet. Renvoie 0 ou une erreur négative. */
int Send_Char(c)
int c;
{
    char b;
    short rc;
    long limit;

    b = (char)c;
    trace("> Send_Char <%d>\n", (unsigned char)b);
    limit = clock() + SEND_TIMEOUT;
    rc = -1;
    while (rc == -1)
    {
        rc = X_TCP_send(smtp_handle, &b, 1);
        if (limit < clock())
            rc = -0x10;
    }
    if (rc < 0)
        send_error(rc, "! X_TCP_send <%s> error <%s>\n");
    return rc;
}

/* Envoie une commande formatée. Renvoie le résultat de X_TCP_send. */
int Send_Commande(char *fmt, ...)
{
    char buf[0x1f6];
    char *p;
    va_list ap;
    short rc;
    long limit;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    limit = clock() + SEND_TIMEOUT;
    rc = -1;
    while (rc == -1)
    {
        rc = X_TCP_send(smtp_handle, buf, (short)strlen(buf));
        if (limit < clock())
            rc = -0x10;
    }
    p = buf + strlen(buf) - 1;
    while (p > buf && (*p == '\r' || *p == '\n'))
    {
        *p = 0;
        p--;
    }
    trace("> Send_Commande <%s> : \"%s\"\n", mail.server, buf);
    if (rc < 0)
        send_error(rc, "! TCP_send <%s> error <%s>\n");
    return rc;
}

/* Envoie le corps du message puis la ligne "." finale. Renvoie 0 ou une erreur.
   Défauts reproduits : sans décodage, le dernier octet du message n'est pas
   envoyé ; un point seul sur une ligne est suivi d'une espace au lieu d'être doublé ;
   p[-1] est lu au premier octet ; le résultat de la ligne finale est ignoré. */
int Send_Message(msg)
char *msg;
{
    char *p;
    char *end;
    short dot;
    short form;
    short rc;
    long limit;

    p = msg;
    end = msg + strlen(msg) - 1;
    dot = 0;
    form = 0;
    trace("> Send_Message <%s>\n", msg);
    if (strnicmp(mail.enctype, "application/x-www-form-urlencoded", 0x23) == 0)
    {
        end = url_decode(msg);
        form = 1;
    }
    while (end > p)
    {
        if (form == 1)
        {
            if (*p == '+')
                *p = ' ';
            else if (*p == '&')
            {
                rc = Send_Char('\r');
                if (rc < 0)
                    return rc;
                rc = Send_Char('\n');
                if (rc < 0)
                    return rc;
                p++;
                if (end < p)
                    return 0;
            }
        }
        if (*p == '.' && (p[1] == '\r' || p[1] == '\n')
            && (p[-1] == '\r' || p[-1] == '\n'))
            dot = 1;
        limit = clock() + SEND_TIMEOUT;
        rc = -1;
        while (rc == -1)
        {
            rc = X_TCP_send(smtp_handle, p, 1);
            if (limit < clock())
                rc = -0x10;
        }
        if (rc < 0)
        {
            send_error(rc, "! TCP_send <%s> error <%s>\n");
            return rc;
        }
        p++;
        if (dot == 1)
        {
            rc = Send_Char(' ');
            if (rc < 0)
                return rc;
            dot = 0;
        }
    }
    Send_Commande("\r\n.\r\n");
    return 0;
}


/* Alerte "erreur de courrier" commune aux réponses refusées. */
static void reply_error(line, fmt, arg)
char *line;
char *fmt;
char *arg;
{
    trace("! %s\n", line);
    if (arg)
        my_form_alerte(mail.server, fmt, arg, line);
    else
        my_form_alerte(mail.server, fmt, line);
}

/* Traite une réponse du serveur (code et texte) selon l'état du dialogue.
   Renvoie 1 pour continuer à attendre, 0 pour arrêter.
   Nom non vérifié. Défaut : une réponse refusée aux états 1 à 4 laisse l'état
   inchangé, mailto s'arrête quand même car 0 est renvoyé. */
static int smtp_reply(line, code)
char *line;
int code;
{
    char date[0x50];
    time_t t;

    switch (mail.state)
    {
    case ST_HELO:
        if (code == SMTP_OK)
        {
            if (Send_Commande("mail from:<%s>\r\n", mail.from) < 0)
                return 0;
            mail.state = ST_FROM;
            return 1;
        }
        if (code == SMTP_READY)
            return 1;
        reply_error(line, "[1][%s][ Ok ]", (char *)0);
        return 0;
    case ST_FROM:
        if (code == SMTP_OK)
        {
            if (Send_Commande("rcpt to:<%s>\r\n", mail.rcpt) < 0)
                return 0;
            mail.state = ST_RCPT;
            return 1;
        }
        reply_error(line, "[1][%s:|%s][ Ok ]", mail.from);
        return 0;
    case ST_RCPT:
        if (code == SMTP_OK)
        {
            if (Send_Commande("data\r\n") < 0)
                return 0;
            mail.state = ST_DATA;
            return 1;
        }
        reply_error(line, "[1][%s:|%s][ Ok ]", mail.rcpt);
        return 0;
    case ST_DATA:
        if (code == SMTP_DATA)
        {
            time(&t);
            strftime(date, 0x50, "%d %b %Y %X GMT", localtime(&t));
            if (Send_Commande("Date: %s\r\n", date) < 0)
                return 0;
            if (Send_Commande("From: %s\r\n", mail.from) < 0)
                return 0;
            if (Send_Commande("To: %s\r\n", mail.rcpt) < 0)
                return 0;
            if (Send_Commande("Subject: %s\r\n\r\n", mail.subject) < 0)
                return 0;
            if (Send_Message(mail.message) < 0)
                return 0;
            mail.state = ST_BODY;
            return 1;
        }
        reply_error(line, "[1][error in your mail:|%s][ Ok ]", (char *)0);
        return 0;
    case ST_BODY:
        if (code == SMTP_OK)
        {
            trace("~ Message sended!\n");
            mail.state = ST_DONE;
            return 0;
        }
        trace("! %s\n", line);
        my_form_alerte(mail.server, "[1][error in your mail:|%s][ Ok ]", line);
        mail.state = ST_ERROR;
        return 0;
    default:
        return 1;
    }
}

/* Heure du prochain appel des événements GEM, en ticks de 200 Hz.
   L'ordre multiplication par 1000 puis division par 200 est celui du
   binaire ; la correspondance des routines flottantes n'est pas vérifiée. */
static long next_event_time()
{
    return (long)((long double)clock()
                  + (long double)conf.event_interval * 1000.0L / 200.0L);
}

/* Envoie le message msg à rcpt par SMTP. rcpt peut contenir "?sujet".
   Renvoie 0 si le message est accepté, -1 sinon.
   Défauts reproduits : aucun retour de connexion fermée si helo échoue ou si
   la résolution du nom échoue. */
int mailto(rcpt, enctype, message)
char *rcpt;
char *enctype;
char *message;
{
    char buf[0x1f6];
    long ip;
    short result;
    short done;
    short count;
    short got;
    short avail;
    long deadline;
    long next_event;
    char *p;
    short report;

    done = 0;
    count = 3;
    mail.rcpt = rcpt;
    mail.subject = "From the WEB";
    mail.message = message;
    mail.enctype = enctype;
    mail.state = ST_HELO;
    p = strchr(mail.rcpt, '?');
    if (p)
    {
        *p = 0;
        p++;
        mail.subject = p;
    }
    trace("> mailto : rcpt     = <%s>\n", mail.rcpt);
    trace("> mailto : subject  = <%s>\n", mail.subject);
    trace("> mailto : enctype  = <%s>\n", mail.enctype);

    mail.from = X_getvstr("EMAIL");
    if (mail.from[1] == 0 && (mail.from[0] == '0' || mail.from[0] == '1'))
    {
        my_form_alerte("Mailto", "[1][Can not find|\"EMAIL=\"|variable in your system][ Ok ]");
        return -1;
    }
    mail.server = X_getvstr("SMTP_SERVER");
    if (mail.server[1] == 0 && (mail.server[0] == '0' || mail.server[0] == '1'))
    {
        my_form_alerte("Mailto", "[1][Can not find|\"SMTP_SERVER=\"|variable in your system][ Ok ]");
        return -1;
    }

    (*cab->progress)(4L, 0L);
    result = X_resolve(mail.server, (char **)0, 1, &ip);
    if (result < 0)
    {
        (*cab->event)(-0x258L);
        my_form_alerte("Mailto", "[1][resolve error on|%s|%s][ Ok ]",
                       mail.server, stik_error_text(result));
        return -1;
    }
    (*cab->progress)(1L, 0L);
    smtp_handle = X_TCP_open(ip, 25, 0, 501);
    if (smtp_handle < 0)
    {
        my_form_alerte("Mailto", "[1][Network error on|%s|%s][ Ok ]",
                       mail.server, stik_error_text(smtp_handle));
        return -1;
    }
    (*cab->progress)(5L, 0L);
    if (Send_Commande("helo [127.0.0.1]\r\n") < 0)
        return -1;

    trace("~ Wait SMTP server <%s> reponse....\n", mail.server);
    deadline = clock() + (long)conf.timeout * 200L;
    next_event = next_event_time();
    for (;;)
    {
        if (done == 0x1b)
            break;
        avail = X_CNbyte_count(smtp_handle);
        if (avail > 0)
        {
            got = X_CNget_block(smtp_handle, buf, avail);
            if (got > 0)
            {
                deadline = clock() + (long)conf.timeout * 200L;
                buf[got] = 0;
                p = buf + strlen(buf) - 1;
                while (p > buf && (*p == '\r' || *p == '\n'))
                {
                    *p = 0;
                    p--;
                }
                trace("~ receive %d bytes <%s>\n", got, buf);
                buf[3] = 0;
                count = 0;
                if (smtp_reply(buf + 4, atoi(buf)) != 1)
                {
                    done = 0x1b;
                    break;
                }
            }
            else if (got < 0)
            {
                my_form_alerte("Mailto", "[1][Network error on|%s|%s][ Ok ]",
                               mail.server, stik_error_text(got));
                break;
            }
        }
        else if (avail < 0)
        {
            my_form_alerte("Mailto", "[1][Network error on|%s|%s][ Ok ]",
                           mail.server, stik_error_text(avail));
            break;
        }

        if (deadline < clock())
        {
            trace("~ Timeout!\n");
            (*cab->event)(-0x24L);
            my_form_alerte("Mailto", "[1][Server|%s|%s][ Ok ]",
                           mail.server, stik_error_text(-0x10));
            done = 0x1b;
            mail.state = ST_ERROR;
        }
        if (clock() > next_event && avail == 0)
        {
            if ((*cab->gem_events)((long)conf.event_duration) == -1L)
            {
                trace("! aes_events : STOP icon or Esc key pressed\n");
                (*cab->event)(-0x13bL);
                done = 0x1b;
                mail.state = ST_ERROR;
            }
            next_event = next_event_time();
            if (count == 3)
                (*cab->progress)(10L, (deadline - clock()) / 200L);
            else if (count < 3)
                count++;
        }
    }

    if (Send_Commande("quit\r\n") < 0)
        X_TCP_close(smtp_handle, 0);
    else
        X_TCP_close(smtp_handle, 3);
    if (mail.state == ST_DONE)
    {
        report = -1;
        trace("~ mailto return 0L\n");
        ReportText("http://cab.ovl?mailto", "dummy.txt", &report, "Your mail is send!");
        return 0;
    }
    trace("~ mailto return an error\n");
    return -1;
}
