/*
 * GUI.C - interface GEM de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions de
 * 0x14c9c à 0x15685. Noms issus des chaînes de trace : rsc_load,
 * ask_cookie_accepte, ask_authentification, my_form_alerte. Les autres sont
 * des noms de reconstitution.
 * Les dialogues passent par la bibliothèque windform 3.0 de Jacques Delavoix
 * (0x1e230 et suivantes, non reconstituée, noms de ses fonctions dans
 * ATARI/ref/windform/WFORM300/WINDFORM.300/LIB/WINDFORM.H). Les liaisons AES
 * et VDI sont déclarées extern.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "CABOVL.H"
#include "LOG.H"
#include "GUI.H"

/* Liaisons AES et VDI. */
extern short rsrc_gaddr(short type, short index, void *addr);   /* 0x1c0ea, AES 112 */
extern short rsrc_free(void);                                   /* 0x1c0e0, AES 111 */
extern short appl_exit(void);                                   /* 0x1beb2, AES 19 */
extern short graf_mouse(short form, void *addr);                /* 0x1c01e, AES 78 */
extern short evnt_multi(short flags, short bclicks, short bmask, short bstate,
                        short m1flags, short m1x, short m1y, short m1w, short m1h,
                        short m2flags, short m2x, short m2y, short m2w, short m2h,
                        short *msgbuf, short tlocount, short thicount,
                        short *mox, short *moy, short *mobutton, short *mokstate,
                        short *mkreturn, short *mbreturn);      /* 0x1bec6, AES 25 */
extern short wind_get(short handle, short field,
                      short *w1, short *w2, short *w3, short *w4);  /* 0x1c1f0, AES 104 */
extern short wind_set(short handle, short field,
                      short w1, short w2, short w3, short w4);  /* 0x1c238, AES 105 */
extern short wind_find(short x, short y);                       /* 0x1c18e, AES 106 */
extern short form_dial(short flag, short x1, short y1, short w1, short h1,
                       short x2, short y2, short w2, short h2); /* 0x1bf6a, AES 51 */
extern short v_clsvwk(short handle);                            /* 0x1c2e6, VDI 101 */

/* Bibliothèque windform 3.0 de Jacques Delavoix (ATARI/ref/windform/WFORM300,
   WINDFORM.H), non reconstituée. Un formulaire fait 22 octets. */
typedef struct
{
    void *adr_form;         /* 0x00, arbre du formulaire */
    short w_handle;         /* 0x04, handle de la fenêtre, témoin d'ouverture */
    short w_x;              /* 0x06 */
    short w_y;
    short w_w;
    short w_h;
    short offset;           /* 0x0e */
    short edit_objc;        /* 0x10, objet éditable courant */
    short edit_pos;         /* 0x12 */
    char reduced;           /* 0x14 */
    char iconified;         /* 0x15 */
} WFORM;

extern void init_gem(void);                                     /* 0x1e33c */
extern short load_rsc(char *rsc_name, short wind_fail, short index_icon);   /* 0x1e42c, 0 si échec */
extern void init_windform(WFORM *f, short form_index, short pos_x, short pos_y);   /* 0x1e4e2 */
extern void open_dialog(WFORM *f, char *w_title, short edit_index, short back);    /* 0x1e66c */
extern short windform_do(WFORM *f, short event);                /* 0x1e9aa, bouton, 0 si aucun */
extern void wf_change(WFORM *f, short objet, short state, short flag);  /* 0x1e7ac, remplace objc_change */
extern void close_dialog(WFORM *f);                             /* 0x1e772 */

extern short fw_attrib;                         /* 0x243ee, attributs de fenêtre, 0x4009 avant chaque ouverture */
extern short aes_handle;                        /* 0x2cb7a */
extern short vdi_handle;                        /* 0x2cb7c */
extern short x_desk, y_desk, w_desk, h_desk;    /* 0x2cb80, bureau */

typedef struct
{
    char *unshift;
    char *shift;
    char *capslock;
} KEYTAB;

extern KEYTAB *Keytbl(void *unshift, void *shift, void *capslock);  /* XBIOS 16 */

extern char auth_cred[];                         /* 0x24ee2, "login:mot de passe", 0x81 octets */

OBJECT *tree_report;                            /* 0x2b356, arbre 6 */
static OBJECT *tree_msg;                        /* 0x2b352, arbre 1 */
static OBJECT *tree_cookie;                     /* 0x2b35a, arbre 0 */
static OBJECT *tree_alert;                      /* 0x2b35e, arbre 5 */
static OBJECT *tree_auth;                       /* 0x2b362, arbre 7 */

static WFORM forms[3];                          /* 0x2b366, cookie, authentification, alerte */
static char alert_buf[0x100];                   /* 0x2b3a8 */
static char alert_title[0x1a];                  /* 0x2b4a8 */
static short form_rect[4];                      /* 0x2b4c2, position des formulaires */

static OBJECT *cur_obj;                         /* 0x2b4ca, ligne courante de rsc_next_line */
static short cur_index;                         /* 0x2b4ce */
static short cur_tree = -1;                     /* 0x2209e */
static short line_started;                      /* 0x220a0 */
static KEYTAB *keytab;                          /* 0x220a2 */

/* Tampons d'événements de run_dialog. Adresses 0x2cb88 à 0x2cbae, proches des
   variables de la bibliothèque : l'appartenance à GUI.C n'est pas vérifiée. */
static short msgbuf[8];                         /* 0x2cb88 */
static short ev_mx;                             /* 0x2cb98 */
static short ev_my;                             /* 0x2cb9a */
static short ev_button;                         /* 0x2cb9c */
static short ev_key;                            /* 0x2cb9e */
static short ev_kstate;                         /* 0x2cba0 */
static short ev_clicks;                         /* 0x2cba2 */
static short top_win;                           /* 0x2cbae */

/* Texte (te_ptext) de l'objet i d'un arbre. */
#define OBJ_TEXT(tree, i) (((TEDINFO *)(tree)[i].ob_spec)->te_ptext)

/* Charge le fichier de ressources, initialise la bibliothèque de fenêtres
   et retrouve les arbres. Renvoie 1 si réussi, 0 sinon. */
int rsc_load(file)
char *file;
{
    trace("> rsc_load <%s>\n", file);
    init_gem();
    if (load_rsc(file, 0, 4) == 0)
        return 0;

    init_windform(&forms[0], 0, 0, 0);
    tree_cookie = (OBJECT *)forms[0].adr_form;
    init_windform(&forms[1], 7, 0, 0);
    tree_auth = (OBJECT *)forms[1].adr_form;
    init_windform(&forms[2], 5, 0, 0);
    tree_alert = (OBJECT *)forms[2].adr_form;
    rsrc_gaddr(0, 1, &tree_msg);
    rsrc_gaddr(0, 6, &tree_report);
    return 1;
}

/* Ferme les stations VDI, libère les ressources et quitte l'AES. */
void gem_cleanup()
{
    v_clsvwk(vdi_handle);
    v_clsvwk(aes_handle);
    rsrc_free();
    appl_exit();
}

/* Fin de s sur n caractères au plus, "" si s est nul (0x14e9a). */
static char *tail_of(s, n)
char *s;
int n;
{
    int len;

    if (s == 0)
        return "";
    len = (int)strlen(s);
    if (n < len)
        return s + (len - n);
    return s;
}

/* Copie dans dest les n derniers caractères de src (0x14ecc). dest doit
   avoir n + 1 octets. */
static void set_text(dest, n, src)
char *dest;
int n;
char *src;
{
    dest[0] = 0;
    strncat(dest, tail_of(src, n), (size_t)n);
}

/* Dialogue du cookie, arbre 0 (0x15128). Appelé avec 0x2000 pour ouvrir
   la fenêtre, puis avec le code d'événement. Renvoie le bouton, 0 si aucun. */
static int dlg_cookie(ev)
int ev;
{
    short b;

    if (ev == 0x2000)
    {
        fw_attrib = 0x4009;
        open_dialog(&forms[0], " CAB.OVL: Accept cookie ? ", 0, 1);
        return 0;
    }
    b = windform_do(&forms[0], (short)ev);
    if (b == 0)
        return 0;
    wf_change(&forms[0], b, 0, 0);
    close_dialog(&forms[0]);
    return b;
}

/* Dialogue d'authentification, arbre 7 (0x15186). Le bouton 7 valide :
   le texte "login:mot de passe" est rangé dans auth_cred. */
static int dlg_auth(ev)
int ev;
{
    short b;

    if (ev == 0x2000)
    {
        fw_attrib = 0x4009;
        open_dialog(&forms[1], " CAB.OVL: Access Restricted ", 2, 1);
        return 0;
    }
    b = windform_do(&forms[1], (short)ev);
    if (b == 0)
        return 0;
    wf_change(&forms[1], b, 0, 0);
    if (b == 7)
    {
        auth_cred[0] = 0;
        strncat(auth_cred, OBJ_TEXT(tree_auth, 2), 0x80);
        strncat(auth_cred, ":", 0x80);
        strncat(auth_cred, OBJ_TEXT(tree_auth, 4), 0x80);
    }
    close_dialog(&forms[1]);
    return b;
}

/* Dialogue d'alerte, arbre 5 (0x1523c). Renvoie le numéro du bouton, de 1 à 5
   (les boutons sont les objets 5 à 9). */
static int dlg_alert(ev)
int ev;
{
    short b;

    if (ev == 0x2000)
    {
        fw_attrib = 0x4009;
        open_dialog(&forms[2], alert_title, 0, 1);
        return 0;
    }
    b = windform_do(&forms[2], (short)ev);
    if (b == 0)
        return 0;
    wf_change(&forms[2], b, 0, 0);
    close_dialog(&forms[2]);
    return b - 4;
}

/* Décode une touche dans out (5 octets) : état des touches spéciales, code
   de balayage, code ASCII, puis la touche sans et avec Majuscule en
   majuscule (0x1560e). */
static void key_decode(out, key, kstate)
char *out;
short key;
short kstate;
{
    if (keytab == 0)
        keytab = Keytbl((void *)-1L, (void *)-1L, (void *)-1L);
    out[0] = (char)kstate;
    out[1] = (char)(key >> 8);
    out[2] = (char)key;
    out[3] = (char)toupper((unsigned char)keytab->unshift[(unsigned char)out[1]]);
    out[4] = (char)toupper((unsigned char)keytab->shift[(unsigned char)out[1]]);
}

/* Ctrl-U ferme le formulaire dont la fenêtre est win (0x155a6). */
static void key_close_form(win, key, kstate)
short win;
short key;
short kstate;
{
    char k[5];

    key_decode(k, key, kstate);
    if ((k[0] & 4) == 0 || (unsigned char)k[4] != 0x55)
        return;
    if (win == forms[0].w_handle)
        close_dialog(&forms[0]);
    else if (win == forms[1].w_handle)
        close_dialog(&forms[1]);
    else if (win == forms[2].w_handle)
        close_dialog(&forms[2]);
}

/* Affiche un dialogue et attend un bouton (0x14ef4). which : 0 cookie,
   7 authentification, 5 alerte, autre valeur sans effet. Renvoie le bouton. */
static int run_dialog(which)
int which;
{
    short r = 0;
    short ev;
    short d1, d2, d3;

    form_rect[0] = 10 + x_desk;
    form_rect[1] = 10 + y_desk;
    form_rect[2] = w_desk / 2;
    form_rect[3] = h_desk / 2;

    if (which == 0)
        dlg_cookie(0x2000);
    else if (which == 7)
        dlg_auth(0x2000);
    else if (which == 5)
        dlg_alert(0x2000);
    else
        return 0;

    graf_mouse(0, 0);
    printf("\007");

    do
    {
        ev = evnt_multi(0x13, 0x102, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        msgbuf, 10, 0,
                        &ev_mx, &ev_my, &ev_button, &ev_kstate, &ev_key, &ev_clicks);
        (*cab->messages)(msgbuf);
        wind_get(0, 10, &top_win, &d1, &d2, &d3);

        /* Message 0x15 (fenêtre passée au premier plan) : on remonte la
           première fenêtre de dialogue ouverte. */
        if ((ev & 0x10) && msgbuf[0] == 0x15)
        {
            if (forms[0].w_handle > 0)
                wind_set(forms[0].w_handle, 10, 0, 0, 0, 0);
            else if (forms[1].w_handle > 0)
                wind_set(forms[1].w_handle, 10, 0, 0, 0, 0);
            else if (forms[2].w_handle > 0)
                wind_set(forms[2].w_handle, 10, 0, 0, 0, 0);
        }

        if (ev & 1)
            key_close_form(top_win, ev_key, ev_kstate);
        else if (ev & 2)
            msgbuf[3] = wind_find(ev_mx, ev_my);    /* réutilise le champ handle du message */

        if (msgbuf[3] > 0)
        {
            if (msgbuf[3] == forms[0].w_handle)
                r = dlg_cookie(ev);
            else if (msgbuf[3] == forms[1].w_handle)
                r = dlg_auth(ev);
            else if (msgbuf[3] == forms[2].w_handle)
                r = dlg_alert(ev);
        }
    } while (r == 0);

    close_dialog(&forms[0]);
    close_dialog(&forms[1]);
    close_dialog(&forms[2]);
    form_dial(3, 0, 0, 0, 0, x_desk, y_desk, w_desk, h_desk);
    return r;
}

/* Demande si un cookie est accepté. Les valeurs sont recopiées, tronquées à
   leurs dernières lettres, dans les champs du dialogue. Un pointeur nul
   laisse le champ vide. Renvoie le numéro d'objet du bouton, 0xf pour
   accepter (voir COOKIE.C). */
int ask_cookie_accepte(host, domain, path, name, value, expires)
char *host;
char *domain;
char *path;
char *name;
char *value;
char *expires;
{
    trace("> ask_cookie_accepte\n");

    OBJ_TEXT(tree_cookie, 2)[0] = 0;
    OBJ_TEXT(tree_cookie, 7)[0] = 0;
    OBJ_TEXT(tree_cookie, 9)[0] = 0;
    OBJ_TEXT(tree_cookie, 11)[0] = 0;
    OBJ_TEXT(tree_cookie, 13)[0] = 0;
    OBJ_TEXT(tree_cookie, 5)[0] = 0;

    if (host != 0)
        set_text(OBJ_TEXT(tree_cookie, 2), 0x3c, host);
    if (domain != 0)
        set_text(OBJ_TEXT(tree_cookie, 5), 0x32, domain);
    if (path != 0)
        set_text(OBJ_TEXT(tree_cookie, 7), 0x32, path);
    if (name != 0)
        set_text(OBJ_TEXT(tree_cookie, 9), 0x32, name);
    if (value != 0)
        set_text(OBJ_TEXT(tree_cookie, 11), 0x32, value);
    if (expires != 0)
        set_text(OBJ_TEXT(tree_cookie, 13), 0x32, expires);
    return run_dialog(0);
}

/* Demande un login et un mot de passe. Renvoie dans user et pass les
   tampons des champs, que l'utilisateur remplit. Renvoie le bouton, 7 pour
   valider. host n'est pas utilisé (l'original le passe à trace sans
   format correspondant). */
int ask_authentification(host, user, pass)
char *host;
char **user;
char **pass;
{
    trace("> ask_authentification\n");

    OBJ_TEXT(tree_auth, 2)[0] = 0;
    OBJ_TEXT(tree_auth, 4)[0] = 0;
    *user = OBJ_TEXT(tree_auth, 2);
    *pass = OBJ_TEXT(tree_auth, 4);
    return run_dialog(7);
}

/* Lit les lignes G_FTEXT (type 0x15) d'un arbre l'une après l'autre.
   Renvoie l'indice de l'objet et place son texte dans *line. Renvoie -1
   et *line = 0 après la dernière ligne, l'arbre suivant se relit alors
   depuis le début. Le premier appel sur un arbre le charge. */
int rsc_next_line(line, tree)
char **line;
int tree;
{
    if (tree != cur_tree)
    {
        rsrc_gaddr(0, (short)tree, &cur_obj);
        cur_index = 0;
        cur_tree = (short)tree;
        line_started = 0;
    }

    for (;;)
    {
        cur_obj++;
        cur_index++;
        if ((cur_obj->ob_type & 0xff) == 0x15)
        {
            if (line_started == 1)
                break;
            /* Le dernier enfant de la racine a ob_next à 0. */
            if (cur_obj->ob_next == 0)
                line_started = 1;
            *line = ((TEDINFO *)cur_obj->ob_spec)->te_ptext;
            return cur_index;
        }
        if ((cur_obj->ob_flags & 0x20) && line_started != 0)
            break;
    }
    *line = 0;
    cur_tree = -1;
    return -1;
}

/* Place les lignes du texte d'alerte (séparées par '|') dans les objets 1
   à 4 de l'arbre 5, 60 caractères au plus, et vide celles qui restent
   (0x15422). */
static void alert_set_lines(s)
char *s;
{
    int done = 0;
    int i = 0;
    int len;
    char *p;

    len = (int)strlen(s);
    do
    {
        p = strchr(s, '|');
        if (p == 0)
        {
            p = s + len - 1;
            done = 1;
        }
        else
            *p = 0;
        if (i < 4)
        {
            set_text(OBJ_TEXT(tree_alert, i + 1), 0x3c, s);
            i++;
        }
        s = p + 1;
    } while (s != 0 && !done);

    while (i < 4)
    {
        set_text(OBJ_TEXT(tree_alert, i + 1), 10, "");
        i++;
    }
}

/* Place les boutons (séparés par '|') dans les objets 5 à 9 de l'arbre 5,
   10 caractères au plus. Les boutons inutilisés sont cachés (0x154b6).
   Le texte du bouton est lu à te_ptmplt : hypothèse, non vérifiée. */
static void alert_set_buttons(s)
char *s;
{
    int done = 0;
    int i = 0;
    int len;
    char *p;

    len = (int)strlen(s);
    do
    {
        p = strchr(s, '|');
        if (p == 0)
        {
            p = s + len - 1;
            done = 1;
        }
        else
            *p = 0;
        if (i < 5)
        {
            set_text(((TEDINFO *)tree_alert[i + 5].ob_spec)->te_ptmplt, 10, s);
            tree_alert[i + 5].ob_flags &= ~0x80;
            i++;
        }
        s = p + 1;
    } while (s != 0 && !done);

    while (i < 5)
    {
        tree_alert[i + 5].ob_flags |= 0x80;
        i++;
    }
}

/* Alerte au format de form_alert, "[1][ texte | texte ][ OK ]", affichée
   dans une fenêtre. Le titre est précédé de " CAB.OVL: ". Renvoie le
   numéro du bouton. Le numéro d'icône est ignoré. */
int my_form_alerte(char *title, char *fmt, ...)
{
    va_list ap;
    int in_bracket = 0;
    int i;
    int len;
    int n = 0;
    char *tok;

    trace("> my_form_alerte <");
    trace_str(">", title);
    trace_str(">", ">\n");

    alert_buf[0] = 0;
    va_start(ap, fmt);
    vsprintf(alert_buf, fmt, ap);
    va_end(ap);

    /* Les crochets de l'alerte deviennent des séparateurs \001. */
    len = (int)strlen(alert_buf);
    for (i = 0; i < len; i++)
    {
        if (alert_buf[i] == '[' && in_bracket == 0)
        {
            alert_buf[i] = 1;
            in_bracket = 1;
        }
        else if (alert_buf[i] == ']' && in_bracket == 1)
        {
            alert_buf[i] = 1;
            in_bracket = 0;
        }
    }

    for (tok = strtok(alert_buf, "\001"); tok != 0; tok = strtok(0, "\001"))
    {
        if (n == 1)
            alert_set_lines(tok);
        else if (n == 2)
            alert_set_buttons(tok);
        n++;
    }

    alert_title[0] = 0;
    strncat(alert_title, " CAB.OVL: ", 0x19);
    strncat(alert_title, title, 0x19);
    return run_dialog(5);
}

/* Chaîne libre de la ressource (rsrc_gaddr R_FRSTR). */
char *get_rsc_string(index)
int index;
{
    char **p;

    rsrc_gaddr(15, (short)index, &p);
    return *p;
}

/* Texte d'une erreur de la pile TCP, arbre 1. Le code est lu en valeur
   absolue, la ligne est code + 1, au plus 35. */
char *stik_error_text(code)
int code;
{
    int n;

    n = code < 0 ? -code : code;
    n++;
    if (n > 35)
        n = 35;
    return OBJ_TEXT(tree_msg, n);
}
