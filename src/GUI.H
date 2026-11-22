/*
 * GUI.H - interface GEM de CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. Types GEM minimaux et
 * prototypes de GUI.C. Les arbres du fichier CAB_OVL.RSC sont numérotés
 * ainsi dans le code : 0 dialogue de cookie, 1 textes d'erreur de la pile
 * TCP, 2 et 3 page de configuration, 5 alerte, 6 textes des pages d'erreur,
 * 7 dialogue d'authentification.
 */

#ifndef GUI_H
#define GUI_H

/* Objet GEM, 24 octets sur 68000. */
typedef struct
{
    short ob_next;
    short ob_head;
    short ob_tail;
    unsigned short ob_type;
    unsigned short ob_flags;
    unsigned short ob_state;
    void *ob_spec;
    short ob_x;
    short ob_y;
    short ob_w;
    short ob_h;
} OBJECT;

typedef struct
{
    char *te_ptext;
    char *te_ptmplt;
    char *te_pvalid;
} TEDINFO;

extern OBJECT *tree_report;         /* 0x2b356, arbre 6 */

int rsc_load(char *file);                       /* 0x14c9c, 1 si réussi */
void gem_cleanup(void);                         /* 0x14d30 */
int ask_cookie_accepte(char *host, char *domain, char *path,
                       char *name, char *value, char *expires);     /* 0x14d58 */
int ask_authentification(char *host, char **user, char **pass);     /* 0x14e4c */
int rsc_next_line(char **line, int tree);       /* 0x1529c, indice de ligne, -1 à la fin */
int my_form_alerte(char *title, char *fmt, ...);    /* 0x15320 */
char *get_rsc_string(int index);                /* 0x1555e */
char *stik_error_text(int code);                /* 0x15574 */

#endif
