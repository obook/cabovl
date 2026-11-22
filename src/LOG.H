/*
 * LOG.H - journal de CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. Le premier caractère d'un message
 * donne son niveau. Un message n'est écrit que si le bit du niveau est
 * levé dans le masque. Les niveaux, du bit 0 au bit 13 :
 *   - ~ ! ? + : # * r w $ & > /
 * Masque 0x0d : bannières et erreurs. Masque 0x100f : trace complète.
 */

#ifndef LOG_H
#define LOG_H

/* Drapeaux de log_open : le quartet bas vaut 1 pour effacer le fichier, 4
   pour limiter sa taille. Le quartet 0x0f0 vaut 0x20 pour rouvrir le fichier
   à chaque message. Le bit 0x800 ajoute la date à chaque ligne. */

int trace(char *msg, ...);                  /* 0x1dcec */
int trace_str(char *level, char *s);        /* 0x1ddde, suite d'une ligne */
int trace_err(char *msg, ...);              /* 0x1de4a */
int log_open(char *file, char *tag, long size, long flags, long level); /* 0x1db76 */
int log_close(void);                        /* 0x1dc8e */
int log_welcome(void);                      /* 0x1e200 */
int log_end(void);                          /* 0x1e230 */
int log_set_mask(long mask);                /* 0x1e27e */
int log_is_open(void);                      /* 0x1e288 */

#endif
