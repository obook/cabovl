/*
 * REPORT.C - pages d'erreur et de rapport servies à CAB (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions de
 * 0x146ee à 0x14c9a. Le nom du module vient de la chaîne __FILE__
 * "I:\STIKCOVL\REPORT.C". ReportRequestText et ReportText sont les noms des
 * chaînes de trace, les autres noms sont des noms de reconstitution.
 * Le HTML des pages est écrit en dur dans le binaire. Seuls les messages
 * viennent de CAB_OVL.RSC, arbre 6 (les lignes 1 et 2 de l'arbre ne sont
 * pas lues). Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CABOVL.H"
#include "REQUEST.H"
#include "LOG.H"
#include "GUI.H"
#include "REPORT.H"

extern long Fcreate(char *name, int attr);          /* GEMDOS */
extern int Fclose(int handle);
extern long Fdelete(char *name);
extern long Fwrite(int handle, long count, void *buf);

extern char *strlwr(char *s);                       /* 0x1af2c, bibliothèque */

extern char ovl_ver_str[];                          /* OVL.C */
extern char ck_tmpname[];

static char err_url[0x400];                         /* 0x2af3e */
static char date_buf[50];                           /* 0x2af06 */
static time_t now_buf;                              /* 0x2af3a */

/* Compteur et noms de page d'erreur, 0x21ebe, 0x21ec0 et 0x21ec9. */
static char name_cnt[2] = { 'A', 'C' };
static char name_cur[9] = "12345678";
static char name_tmp[9] = "12345678";

/* Texte de la ligne code de l'arbre 6 (0x14b88). */
static char *error_text(code)
int code;
{
    return ((TEDINFO *)tree_report[code].ob_spec)->te_ptext;
}

/* Écrit s dans le fichier h (0x14ba0). Renvoie le retour de Fwrite. */
static long put(s, h)
char *s;
short h;
{
    return Fwrite(h, (long)strlen(s), s);
}

/* Fabrique un nom de 8 caractères dans tmp et le copie dans dest (0x14bc4).
   Les 6 premiers caractères viennent de l'heure, calculée une seule fois :
   année, mois, jour, heure, minutes. Les 2 derniers viennent d'un compteur
   qui avance à chaque appel. */
static void make_name(dest, tmp)
char *dest;
char *tmp;
{
    time_t t;
    struct tm *tm;

    if (name_cnt[0] == 'A' && name_cnt[1] == 'C')
    {
        t = time(0);
        tm = localtime(&t);
        tmp[0] = (char)('A' + tm->tm_year % 10);
        tmp[1] = (char)('A' + tm->tm_mon);
        tmp[2] = (char)('A' + tm->tm_mday);
        if ((unsigned char)tmp[2] > 'Z')
            tmp[2] -= 0x2a;
        tmp[3] = (char)('A' + tm->tm_hour);
        tmp[4] = (char)('A' + tm->tm_min / 10);
        tmp[5] = (char)('A' + tm->tm_min % 10);
    }
    name_cnt[1]++;
    if (name_cnt[1] > 'Z')
    {
        name_cnt[1] = 'C';
        name_cnt[0]++;
        if (name_cnt[0] > 'Z')
            name_cnt[0] = 'A';
    }
    tmp[6] = name_cnt[0];
    tmp[7] = name_cnt[1];
    tmp[8] = 0;
    dest[0] = 0;
    strncat(dest, tmp, 8);
}

/* Prépare la date de la page dans date_buf. */
static void make_date()
{
    time(&now_buf);
    strftime(date_buf, 50, "%a, %d %b %Y %X", localtime(&now_buf));
}

/* Écrit la fin de la page : message, signature et date. Renvoie la somme
   des retours de Fwrite (le total est une somme de short, sans test d'erreur). */
static short put_footer(h, text)
short h;
char *text;
{
    short n = 0;

    n += (short)put("<HR><H1><B>\n\n", h);
    n += (short)put(text, h);
    n += (short)put("\n\n</B></H1>", h);
    n += (short)put("<HR><I>\nCreated by cab.ovl V", h);
    n += (short)put(ovl_ver_str, h);
    n += (short)put(" ", h);
    n += (short)put(date_buf, h);
    n += (short)put("\n</I></BODY></HTML>", h);
    return n;
}

/* Page d'erreur d'une requête : appelle ReportText avec le message de la
   ligne code de l'arbre 6. fh est le handle du fichier à fermer et effacer
   s'il est ouvert.
   Défaut de l'original : il fabrique l'adresse http://cab.ovl/error?... dans
   err_url comme file_error, puis passe req->url à ReportText et ignore err_url. */
void report_error(req, file, code, fh)
REQUEST *req;
char *file;
int code;
short *fh;
{
    make_name(name_cur, name_tmp);
    err_url[0] = 0;
    strncat(err_url, "http://cab.ovl/error?", 0x400);
    strncat(err_url, strlwr(name_tmp), 0x400);
    ReportText(req->url, file, fh, error_text(code));
}

/* Remplace le fichier de la requête par une page de rapport contenant text.
   Renvoie la longueur écrite, ou -1 si le fichier ne se crée pas. */
int ReportRequestText(req, text)
REQUEST *req;
char *text;
{
    short h = -1;
    short n = 0;

    trace("> ReportRequestText <%s> <%d> <%s>\n", req->file, (int)req->file_handle, text);

    if (req->file_handle > 0)
    {
        trace("~ ReportRequest : Fclose <%s>\n", req->file);
        Fclose(req->file_handle);
        if (req->cached != 1)
        {
            trace("~ ReportRequest : delete <%s>\n", req->file);
            Fdelete(req->file);
        }
        req->file_handle = -1;
    }

    h = (short)Fcreate(req->file, 0);
    if (h < 0)
    {
        trace("? Couldn't create <%s> <%d>\n", req->file, (int)h);
        return -1;
    }

    make_date();
    n += (short)put("<HTML><HEAD><TITLE>\n\nCAB.OVL Report\n\n</TITLE></HEAD><BODY>\n\n", h);
    n += (short)put("Server: ", h);
    n += (short)put(req->server, h);
    n += (short)put("<BR>\n", h);
    n += (short)put("URL: <A HREF=\"", h);
    n += (short)put(req->url, h);
    n += (short)put("\">", h);
    n += (short)put(req->path, h);
    n += (short)put("</A><BR><BR>\n\n", h);
    n += put_footer(h, text);
    Fclose(h);
    return n;
}

/* Page d'erreur d'un fichier (création, écriture, fermeture) : op est la
   ligne du message dans l'arbre 6. err est le handle à fermer. */
void file_error(file, op, err)
char *file;
int op;
short *err;
{
    make_name(name_cur, name_tmp);
    err_url[0] = 0;
    strncat(err_url, "http://cab.ovl/error?", 0x400);
    strncat(err_url, strlwr(name_tmp), 0x400);
    ReportText(err_url, file, err, error_text(op));
}

/* Écrit une page de rapport pour url. CAB propose un nom de fichier, sinon
   file, sinon cabovl.tmp. Renvoie la longueur écrite, ou -1 si la création échoue.
   Si *fh > 0, ferme et efface file d'abord, quel que soit le nom choisi ensuite. */
int ReportText(url, file, fh, text)
char *url;
char *file;
short *fh;
char *text;
{
    char *name;
    short h;
    short n = 0;

    trace("> ReportText <%s> <%d> <%s>\n", file, (int)*fh, text);

    if (*fh > 0)
    {
        Fclose(*fh);
        Fdelete(file);
        *fh = -1;
    }

    if ((*cab->new_filename)(url, &name) != 0)
    {
        trace("! browser can not give a new filename, use default <%s>\n", file);
        name = file;
    }
    if (name[0] == 0)
        name = ck_tmpname;

    h = (short)Fcreate(name, 0);
    if (h < 0)
    {
        trace("? Couldn't create <%s> <%d>\n", name, (int)h);
        return -1;
    }

    make_date();
    n += (short)put("<HTML><HEAD><TITLE>\n\nCAB.OVL Report\n\n</TITLE></HEAD><BODY>\n\n", h);
    n += (short)put("URL: <A HREF=\"", h);
    n += (short)put(url, h);
    n += (short)put("\">", h);
    n += (short)put(url, h);
    n += (short)put("</A><BR><BR>\n\n", h);
    n += put_footer(h, text);
    Fclose(h);
    return n;
}
