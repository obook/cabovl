/*
 * COOKIE.C - gestion des cookies de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C, convention
 * d'appel par registres). Les noms de fonctions viennent des chaînes de
 * trace du binaire. Les noms de variables sont des choix de reconstitution.
 * Le tout n'est pas compilé avec Pure C, à vérifier.
 *
 * Reconstituées : AddCookie, SendCookie, SplitCookie, ClearCookie,
 * CheckCookieDate, ProcessLine.
 *
 * Dans le binaire, fopen, fclose et remove passent par des enveloppes de
 * trace qui reçoivent __FILE__ et __LINE__. Ici ce sont les appels standard.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CONFIG.H"
#include "CABOVL.H"
#include "REQUEST.H"
#include "LOG.H"

/* Le mode des cookies est le champ cookie_mode de la configuration (0x246ba). */
#define ck_mode (conf.cookie_mode)


/* Résultat de l'analyse, adresses du segment data de CAB.OVL entre parenthèses. */
char *ck_version;           /* 0x246c0 */
char *ck_name;              /* 0x246c4 */
char *ck_value;             /* 0x246c8 */
char *ck_expires;           /* 0x246cc */
char *ck_path;              /* 0x246d0 */
char *ck_domain;            /* 0x246d4 */
char ck_pathbuf[0x700];     /* 0x246d8, chemin par défaut */
int ck_secure;              /* 0x24dd8 */
char ck_line[0x100];        /* 0x24dda, cookie mis en forme */

char *ck_default_expires = "0";     /* pointeur 0x20766 */

char *ck_current;                   /* 0x24eda, cookie courant, 0 si aucun */

/* Tampons de lecture du fichier de cookies, 0x80 octets chacun. */
char ck_in[0x80];                   /* 0x24ad8, ligne lue */
char ck_copy[0x80];                 /* 0x24b58, copie de la ligne lue */
char ck_f1[0x80];                   /* 0x24bd8, champ 1 */
char ck_f2[0x80];                   /* 0x24c58, champ 2 */
char ck_f3[0x80];                   /* 0x24cd8, champ 3, date d'expiration */
char ck_f4[0x80];                   /* 0x24d58, champ 4, reste de la ligne */

extern char ck_filename[];          /* 0x25489, fichier de cookies, défini ailleurs */
extern char ck_tmpname[];           /* 0x25306, fichier temporaire, défini ailleurs */


extern unsigned long parse_date(char *s);   /* 0x15686, 0xffffffff si erreur */
extern int ask_cookie_accepte(char *host, char *domain, char *path,
                              char *name, char *value, char *expires);  /* 0x14d58, 0xf = accepté */

/* Fonctions du module et du journal de CAB.OVL. */
extern char *skip_blanks(char *s);  /* 0x15e78, saute espaces et tabulations */
extern char *trim_token(char *s);   /* 0x15e8a, idem, coupe au premier caractère < 0x20 */

/* Remplace CR et LF par 0 dans s. s peut être nul. */
void chomp(s)
char *s;
{
    if (s == 0)
        return;
    for (; *s != 0; s++)
        if (*s == '\r' || *s == '\n')
            *s = 0;
}

/* Lit un en-tête "Set-Cookie" dans cookie (modifié sur place).
   Renvoie 1 si le cookie est exploitable, 0 sinon. */
int SplitCookie(url, cookie)
REQUEST *url;
char *cookie;
{
    int has_path = 0;
    int has_domain = 0;
    char *tok;
    char *eq;
    char *p;

    trace("> SplitCookie\n");

    ck_version = 0;
    ck_name = 0;
    ck_value = 0;
    ck_secure = 0;
    ck_path = 0;
    ck_domain = 0;
    ck_expires = 0;

    for (tok = strtok(cookie, ";"); tok != 0; tok = strtok(0, ";"))
    {
        tok = trim_token(tok);
        trace("~ SplitCookie: token =");
        trace_str("~", tok);
        trace_str("~", ">\n");

        if (strnicmp(tok, "version=", 8) == 0)
            ck_version = skip_blanks(tok + 8);
        else if (strnicmp(tok, "domain=", 7) == 0)
        {
            ck_domain = skip_blanks(tok + 7);
            has_domain = 1;
        }
        else if (strnicmp(tok, "path=", 5) == 0)
        {
            ck_path = skip_blanks(tok + 5);
            has_path = 1;
        }
        else if (strnicmp(tok, "expires=", 8) == 0)
            ck_expires = skip_blanks(tok + 8);
        else
        {
            /* Dernier "=" du jeton : nom=valeur. */
            eq = strrchr(tok, '=');
            if (eq != 0)
            {
                ck_name = tok;
                ck_value = skip_blanks(eq + 1);
                *eq = 0;
            }
            else if (strnicmp(tok, "secure", 6) == 0)
                ck_secure = 1;
        }
    }

    chomp(ck_name);
    chomp(ck_value);
    chomp(ck_path);
    chomp(ck_expires);
    chomp(ck_domain);

    if (ck_name == 0)
    {
        trace("? SplitCookie: incomplete cookie data, no name found\n");
        return 0;
    }
    if (ck_value == 0)
    {
        trace("? SplitCookie: incomplete cookie data, no value found\n");
        return 0;
    }

    if (ck_path == 0)
    {
        /* Chemin du document sans la requête ni le nom de fichier. */
        ck_pathbuf[0] = 0;
        strncat(ck_pathbuf, url->path, 0x400);
        p = strchr(ck_pathbuf, '?');
        if (p != 0)
            *p = 0;
        p = strrchr(ck_pathbuf, '/');
        if (p != 0)
            p[1] = 0;
        ck_path = ck_pathbuf;
        trace("~ SplitCookie: incomplete cookie data, path set as document path.\n");
    }

    if (ck_expires == 0)
    {
        ck_expires = ck_default_expires;
        trace("~ SplitCookie: incomplete cookie data, no expires found\n");
    }

    if (ck_domain == 0)
    {
        /* Domaine par défaut : serveur, à partir de son premier point. */
        p = strchr(url->host, '.');
        if (p == 0)
        {
            trace("! SplitCookie: incomplete cookie data, not domain available\n");
            return 0;
        }
        ck_domain = p;
        trace("~ SplitCookie: incomplete cookie data, domain set as server\n");
    }

    trace("~ SplitCookie : name <");
    trace_str("~", ck_name);
    trace_str("~", ">\n");
    trace("~ SplitCookie : value <");
    trace_str("~", ck_value);
    trace_str("~", ">\n");
    trace("~ SplitCookie : domain <");
    trace_str("~", ck_domain);
    trace_str("~", ">\n");
    trace("~ SplitCookie : path <");
    trace_str("~", ck_path);
    trace_str("~", ">\n");
    trace("~ SplitCookie : expires <");
    trace_str("~", ck_expires);
    trace_str("~", ">\n");

    /* Mise en forme : [$version=V\r\n]nom=valeur[; $path=P][; $domain=D] */
    ck_line[0] = 0;
    if (ck_version != 0)
    {
        strncat(ck_line, "$version=", 0x100);
        strncat(ck_line, ck_version, 0x100);
        strncat(ck_line, "\r\n", 0x100);
    }
    strncat(ck_line, ck_name, 0x100);
    strncat(ck_line, "=", 0x100);
    strncat(ck_line, ck_value, 0x100);
    if (has_path == 1)
    {
        strncat(ck_line, "; ", 0x100);
        strncat(ck_line, "$path=", 0x100);
        strncat(ck_line, ck_path, 0x100);
    }
    if (has_domain == 1)
    {
        strncat(ck_line, "; ", 0x100);
        strncat(ck_line, "$domain=", 0x100);
        strncat(ck_line, ck_domain, 0x100);
    }
    return 1;
}

/* Efface le cookie courant. */
void ClearCookie()
{
    trace("> ClearCookie\n");
    ck_current = 0;
    ck_line[0] = 0;
}

/* Découpe une ligne du fichier de cookies en quatre champs.
   Les trois premiers sont séparés par une tabulation, le quatrième est
   le reste de la ligne. Chaque champ est copié sur n caractères au plus. */
void ProcessLine(line, f1, n, f2, f3, f4)
char *line;
char *f1;
long n;
char *f2;
char *f3;
char *f4;
{
    char *tok;

    trace("> ProcessLine <%s>\n", line);

    f4[0] = 0;
    f3[0] = 0;
    f2[0] = 0;
    f1[0] = 0;

    tok = strtok(line, "\t");
    if (tok == 0)
        return;
    f1[0] = 0;
    strncat(f1, tok, (size_t)n);

    tok = strtok(0, "\t");
    if (tok == 0)
        return;
    f2[0] = 0;
    strncat(f2, tok, (size_t)n);

    tok = strtok(0, "\t");
    if (tok == 0)
        return;
    f3[0] = 0;
    strncat(f3, tok, (size_t)n);

    tok = strtok(0, "");
    if (tok == 0)
        return;
    f4[0] = 0;
    strncat(f4, tok, (size_t)n);
}

/* Réécrit le fichier de cookies sans les cookies périmés.
   Une date "-" ou une date passée supprime la ligne. Une ligne sans date
   est conservée. Renvoie toujours 0. */
int CheckCookieDate()
{
    FILE *in;
    FILE *out;
    unsigned long now;
    unsigned long expires;

    trace("> CheckCookieDate\n");
    if (ck_mode == 5)
        return 0;

    time((time_t *)&now);

    in = fopen(ck_filename, "r");
    if (in == 0)
    {
        trace("? Cookie file <%s> error\n", ck_filename);
        return 0;
    }
    out = fopen(ck_tmpname, "w");
    if (out == 0)
    {
        trace("? Unable to open temporary file <%s>\n", ck_tmpname);
        fclose(in);
        return 0;
    }

    while (fgets(ck_in, 0x7f, in) != 0)
    {
        ck_copy[0] = 0;
        strncat(ck_copy, ck_in, 0xff);
        ProcessLine(ck_in, ck_f1, 0x80L, ck_f2, ck_f3, ck_f4);

        if (ck_f3[0] != 0)
        {
            if (strcmp(ck_f3, "-") == 0)
            {
                trace("~ CheckCookieDate <%s> no expires : WILL DELETE\n", ck_f1);
                continue;
            }
            expires = parse_date(ck_f3);
            if (expires < now && expires != 0xffffffffUL)
            {
                trace("~ CheckCookieDate <%s> (%ld) < (%ld) WILL DELETE\n",
                      ck_f3, expires, now);
                continue;
            }
        }
        /* L'original appelle fprintf avec la ligne comme format. */
        fputs(ck_copy, out);
    }

    fclose(in);
    remove(ck_filename);
    fclose(out);
    rename(ck_tmpname, ck_filename);
    return 0;
}

/* Enregistre le cookie lu par SplitCookie.
   Selon ck_mode et la version de CAB : refus, question à l'utilisateur,
   transmission à CAB ou ajout au fichier de cookies.
   Renvoie 1 seulement si le cookie est écrit dans le fichier. */
int AddCookie(url, cookie)
REQUEST *url;
char *cookie;
{
    FILE *f;

    trace("> AddCookie\n");
    if (ck_mode == 2)
    {
        trace("~ Cannot set cookie because you refuse cookies.\n");
        return 0;
    }
    if (SplitCookie(url, cookie) != 1)
    {
        trace("! SplitCookie error, can not set cookie.\n");
        return 0;
    }
    if (ck_mode == 4
        && ask_cookie_accepte(url->host, ck_domain, ck_path, ck_name,
                              ck_value, ck_expires) != 0xf)
        return 0;

    if (cab_version >= 0x30323730L && ck_mode == 5)
    {
        /* CAB gère lui-même ses cookies : on lui transmet celui-ci. */
        trace("~ browser->cookie(");
        trace_str("~", url->url);
        trace_str("~", ", ");
        trace_str("~", ck_name);
        trace_str("~", ", ");
        trace_str("~", ck_value);
        trace_str("~", ", ");
        trace_str("~", ck_path);
        trace_str("~", ", ");
        trace_str("~", ck_domain);
        trace_str("~", ", ");
        trace_str("~", ck_expires);
        trace_str("~", ", ");
        trace_str("~", ck_secure > 0 ? "1" : "0");
        trace_str("~", ");\n");
        (*cab->cookie)(url->url, ck_name, ck_value, ck_expires,
                       ck_path, ck_domain, ck_secure);
        ck_current = ck_line;
        return 0;
    }

    f = fopen(ck_filename, "a+");
    if (f == 0)
    {
        trace("~ Error open write cookie file <%s>\n", ck_filename);
        return 0;
    }
    /* Une ligne par cookie : domaine, chemin, expiration, cookie. */
    fwrite(ck_domain, 1, strlen(ck_domain), f);
    fputs("\t", f);
    fwrite(ck_path, 1, strlen(ck_path), f);
    fputs("\t", f);
    fwrite(ck_expires, 1, strlen(ck_expires), f);
    fputs("\t", f);
    fwrite(ck_line, 1, strlen(ck_line), f);
    fputs("\n", f);
    fclose(f);
    return 1;
}

/* Ajoute à out (max caractères) l'en-tête "Cookie:" pour le serveur host
   et le chemin req_path. Seul le premier cookie correspondant du fichier
   est envoyé. Renvoie 1 si un cookie a été ajouté, 0 sinon. */
int SendCookie(host, req_path, max, out)
char *host;
char *req_path;
int max;
char *out;
{
    FILE *f;
    char *p;
    int count = 0;

    trace("> SendCookie\n");
    if ((unsigned long)cab_version < 0x99999999UL && ck_mode == 2)
        return 0;

    if (ck_mode == 5)
    {
        /* Le cookie vient de CAB, pas du fichier. */
        if (ck_current == 0)
        {
            trace("~ cookie_str is NULL, no cookie string given by CAB.\n");
            ck_line[0] = 0;
            return 0;
        }
        strncat(out, "Cookie: ", (size_t)max);
        strncat(out, ck_current, (size_t)max);
        strncat(out, "\r\n", (size_t)max);
        trace("- set cookie <");
        trace_str("-", ck_current);
        trace_str("-", ">\n");
        ck_current = 0;
        ck_line[0] = 0;
        return 1;
    }

    ck_current = 0;
    ck_line[0] = 0;
    f = fopen(ck_filename, "r");
    if (f == 0)
    {
        trace("~ Cookie file <%s> not found\n", ck_filename);
        return 0;
    }

    while (fgets(ck_in, 0x7f, f) != 0)
    {
        if (ck_in[0] == ';' || ck_in[0] == '#')
            continue;
        p = strchr(ck_in, '\n');
        if (p != 0)
            *p = 0;
        p = strchr(ck_in, '\r');
        if (p != 0)
            *p = 0;
        ProcessLine(ck_in, ck_f1, 0x80L, ck_f2, ck_f3, ck_f4);
        if (ck_f1[0] == 0)
            continue;

        trace("~ CheckCookie domain <%s> on <%s>\n", host, ck_f1);
        if (strstr(host, ck_f1) == 0 && stricmp(host, ck_f1) != 0)
        {
            trace("~ CheckCookie domain do not match\n");
            continue;
        }
        if (ck_f2[0] == 0)
        {
            trace("~ CheckCookie empty path\n");
            continue;
        }
        trace("~ CheckCookie path <");
        trace_str("~", req_path);
        trace_str("~", "> on <");
        trace_str("~", ck_f2);
        trace_str("~", ">\n");
        if (strstr(req_path, ck_f2) == 0 && stricmp(req_path, ck_f2) != 0)
        {
            trace("~ CheckCookie path do not match\n");
            continue;
        }
        if (ck_f4[0] == 0)
        {
            trace("~ CheckCookie variable empty\n");
            continue;
        }

        if (count == 0)
            strncat(out, "Cookie: ", (size_t)max);
        else
            strncat(out, "; ", (size_t)max);
        strncat(out, ck_f4, (size_t)max);
        fclose(f);
        strncat(out, "\r\n", (size_t)max);
        count++;
        return 1;       /* l'original sort ici, count reste donc à 0 avant */
    }

    fclose(f);
    if (count > 0)
    {
        strncat(out, "\r\n", (size_t)max);
        return 1;
    }
    return 0;
}
