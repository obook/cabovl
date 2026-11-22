/*
 * CONFIG.H - configuration de CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. La structure est lue et écrite
 * telle quelle dans le fichier de configuration (22 octets).
 * Les noms des champs viennent de la page de configuration
 * http://cab.ovl/setup.html (modèle dans CAB_OVL.RSC) et de regle_setup.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define CONF_VERSION 3L

/* Valeurs de cookie_mode, champ "i" de la page de configuration :
   1 toujours accepter, 2 toujours refuser, 3 stocker sans utiliser,
   4 toujours demander, 5 laisser CAB gérer. */
typedef struct
{
    long version;           /* 0x00, CONF_VERSION */
    short log_on;           /* 0x04, champ "a", 1 ouvre le journal cabovl.log */
    short log_all;          /* 0x06, champ "b", 1 active le masque de trace 0x100f */
    short timeout;          /* 0x08, champ "d", attente de réponse du serveur en secondes, 60 */
    float event_interval;   /* 0x0a, champ "g", appel des événements GEM en ms, 500 */
    short event_duration;   /* 0x0e, champ "h", durée des événements GEM, 5 */
    short cookie_mode;      /* 0x10, champ "i", 5 par défaut */
    short online;           /* 0x12, champ "o", 1 force le mode en ligne, 1 */
    short opt14;            /* 0x14, non initialisé par InitConf */
} CONFIG;

extern CONFIG conf;         /* 0x246aa */

int InitConf(char *file);   /* 0x105e0 */
int LoadConf(char *file);   /* 0x1063a */
int SaveConf(char *file);   /* 0x106ee */

#endif
