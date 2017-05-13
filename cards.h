/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the structure definition for cards.
*
*/

#ifndef CARDS_H
#define CARDS_H
#define DECK_PHYSICAL_SIZE      60
/* These are the four possible suits (symbols) associated with any playing card */
#define SPADES		"Spades"
#define HEARTS		"Hearts"
#define CLUBS		"Clubs"
#define DIAMONDS	"Diamonds"
#define PAYOOS	        "Payoos"

typedef enum card_const {SPADES_CONST, HEARTS_CONST, CLUBS_CONST, DIAMONDS_CONST, PAYOO_CONST} card_const;

/* These are the two possible colors associated with any playing card, these may only be used for display purposes as the color of a card is implied by its suit */

/* This is the structure used to store a card in memory. A card is composed of a number from 1 to 10 , a suit which can take any of the four values defined above as constants and a color, which can take any of the two values defined by the above constants, as long as it matches the suit. */
typedef struct card{
	unsigned int number;
        card_const type;
} card;

#endif
