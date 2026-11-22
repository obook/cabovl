/*
 * SENDICON.C - icônes gopher de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C), fonctions en
 * 0x13e88 (SendIcon) et 0x14034 (GetIcon). Les noms viennent des chaînes
 * de trace. Les données des icônes sont dans ICONS.C.
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdio.h>
#include <string.h>
#include "LOG.H"


extern char icon_back[], icon_binary[], icon_blank[], icon_image[], icon_index[];
extern char icon_menu[], icon_movie[], icon_sound[], icon_text[], icon_unknown[];
extern short icon_back_len, icon_binary_len, icon_blank_len, icon_image_len;
extern short icon_index_len, icon_menu_len, icon_movie_len, icon_sound_len;
extern short icon_text_len, icon_unknown_len;

/* Écrit size octets de data dans le fichier. Renvoie size, ou -1. */
int GetIcon(file, data, size)
char *file;
char *data;
short size;
{
    FILE *f;

    trace("> GetIcon : Send %s\n", file);
    f = fopen(file, "w");
    if (f == 0)
    {
        trace("! GetIcon : Can not fopen <%s>\n", file);
        return -1;
    }
    fwrite(data, 1, (size_t)size, f);
    fclose(f);
    return size;
}

/* Écrit dans file l'icône demandée par l'adresse name. Renvoie la taille
   écrite, ou -1 si l'adresse n'est pas une icône connue. */
int SendIcon(name, file)
char *name;
char *file;
{
    trace("> SendIcon : Scan for <%s>\n", name);

    if (strcmp(name, "/internal-gopher-back") == 0)
        return GetIcon(file, icon_back, icon_back_len);
    if (strcmp(name, "/internal-gopher-binary") == 0)
        return GetIcon(file, icon_binary, icon_binary_len);
    if (strcmp(name, "/internal-gopher-blank") == 0)
        return GetIcon(file, icon_blank, icon_blank_len);
    if (strcmp(name, "/internal-gopher-image") == 0)
        return GetIcon(file, icon_image, icon_image_len);
    if (strcmp(name, "/internal-gopher-index") == 0)
        return GetIcon(file, icon_index, icon_index_len);
    if (strcmp(name, "/internal-gopher-menu") == 0)
        return GetIcon(file, icon_menu, icon_menu_len);
    if (strcmp(name, "/internal-gopher-movie") == 0)
        return GetIcon(file, icon_movie, icon_movie_len);
    if (strcmp(name, "/internal-gopher-sound") == 0)
        return GetIcon(file, icon_sound, icon_sound_len);
    if (strcmp(name, "/internal-gopher-text") == 0)
        return GetIcon(file, icon_text, icon_text_len);
    if (strcmp(name, "/internal-gopher-unknown") == 0)
        return GetIcon(file, icon_unknown, icon_unknown_len);
    return -1;
}
