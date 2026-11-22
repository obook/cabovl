/*
 * UTIL.C - fonctions utilitaires de CAB.OVL (V5.68, 19 janvier 2000)
 *
 * Source reconstitué à partir du binaire CAB.OVL (Pure C).
 * Le nom de module d'origine est inconnu : aucune chaîne __FILE__ ne le
 * donne dans cette zone du binaire. Ces fonctions se trouvent en
 * 0x15686 (parse_date) et 0x15e78 (skip_blanks, trim_token).
 * Non compilé avec Pure C, à vérifier.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LOG.H"


/* Saute les espaces et les tabulations en tête de s. */
char *skip_blanks(s)
char *s;
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

/* Comme skip_blanks, puis coupe s au premier caractère inférieur à 0x20. */
char *trim_token(s)
char *s;
{
    char *p;

    s = skip_blanks(s);
    for (p = s; (unsigned char)*p >= 0x20; p++)
        ;
    *p = 0;
    return s;
}

/* Janvier à novembre. Tout autre nom est lu comme décembre. */
static char *month_names[11] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov"
};

/* Valeur des n premiers caractères de s, n vaut 2 ou 4. buf a 5 octets. */
static int field(buf, s, n)
char *buf;
char *s;
int n;
{
    strncpy(buf, s, n);
    return atoi(buf);
}

/* Convertit une date HTTP en secondes depuis 1970.
   Trois formats, repérés par la présence d'un tiret puis d'une virgule :
     RFC 850   "Sunday, 06-Nov-94 08:49:37 GMT"
     RFC 1123  "Sun, 06 Nov 1994 08:49:37 GMT"
     asctime   "Sun Nov  6 08:49:37 1994"
   Renvoie 0xffffffff si mktime échoue.
   Avec le format à tiret, l'année est lue sur 2 chiffres : une date en
   "-Nov-2000" donne l'année 1920, et "-Jan-00" donne 1900.
   Même algorithme que parse_date de CABOVL.C (Tim, 1996, ATARI/ref/stikcovl),
   sans l'arrondi des secondes à 2 s de la version de Tim. */
unsigned long parse_date(s)
char *s;
{
    struct tm t;
    char buf[5];
    int i;
    int d;

    buf[0] = buf[1] = buf[2] = buf[3] = buf[4] = 0;
    trace("> parse_date\n");

    if (strchr(s, '-') != 0)
    {
        d = (int)strcspn(s, ",");
        t.tm_sec = field(buf, s + d + 18, 2);
        t.tm_min = field(buf, s + d + 15, 2);
        t.tm_hour = field(buf, s + d + 12, 2);
        t.tm_year = field(buf, s + d + 9, 2);
        t.tm_mday = field(buf, s + d + 2, 2);
    }
    else if (strchr(s, ',') != 0)
    {
        t.tm_sec = field(buf, s + 23, 2);
        t.tm_min = field(buf, s + 20, 2);
        t.tm_hour = field(buf, s + 17, 2);
        t.tm_mday = field(buf, s + 5, 2);
        t.tm_year = field(buf, s + 12, 4) - 1900;
    }
    else
    {
        t.tm_sec = field(buf, s + 17, 2);
        t.tm_min = field(buf, s + 14, 2);
        t.tm_hour = field(buf, s + 11, 2);
        t.tm_mday = field(buf, s + 8, 2);
        t.tm_year = field(buf, s + 20, 4) - 1900;
    }

    t.tm_mon = 11;
    for (i = 0; i < 11; i++)
    {
        if (strstr(s, month_names[i]) != 0)
        {
            t.tm_mon = i;
            break;
        }
    }
    return (unsigned long)mktime(&t);
}
