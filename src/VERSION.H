/*
 * version.h
 * Num‚ro et date de version du module.
 *
 * CAB_OVL_VERSION n'utilise pas les deux derniers chiffres
 * (CAB ne les lit pas dans le User-Agent).
 *
 * Utilis‚ par : COOKIE.C, MAIN.C, SETUP.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/*
 SVP ne pas utiliser les deux derniers chiffres pour CAB_OVL_VERSION
 cab user_agent n'en tient pas compte
*/


#define	CAB_OVL_VERSION	0x00056800L
#define	CAB_OVL_DATE		0x20000119L		/* 0x19991231L pour 31/12/1999 */

/* Version de CAB qui prend en charge totalement les cookies : */

/* #define CAB_GERE_COOKIE	0x30323730L */
#define CAB_GERE_COOKIE	0x99999999L