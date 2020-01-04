#ifndef TALIDEON_CARDS__cards_h
#define TALIDEON_CARDS__cards_h
/*-
 * Copyright (c) Keith Gaughan, 2007.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common.h"

/*
 * The cards library does credit and debit card number checks. While it can't
 * validate the existence of a card number, it can check if a provided card
 * number is actually well-formed (yes, there's a difference between
 * well-formedness and validity...)
 *
 * The library validates against a wide variety of debit and credit cards,
 * and includes many cards that are not usually validated correctly. For
 * instance, it correctly includes the Laser, Switch and Solo cards under
 * Maestro. 
 *
 * For an explaination of why these constants have these values, see cards.c.
 */

/* American Express */
#define CARDPAT_AMEX            (1 << 0)
/* China Union Pay */
#define CARDPAT_CUP             (1 << 1)
/* Diner's Club International and Carte Blanche */
#define CARDPAT_DINERS          ((1 << 2) | (1 << 3))
/* Discovery Card (and China Union Pay) */
#define CARDPAT_DISC            (CARDPAT_CUP | (1 << 4))
/* JCB */
#define CARDPAT_JCB             ((1 << 5) | (1 << 6))
/* Laser (but Maestro outside of Ireland) */
#define CARDPAT_LASER           (1 << 7)
/* Maestro (not including cards that are now Maestro) */
#define CARDPAT_MAESTRO_CLASSIC (1 << 8)
/* MasterCard */
#define CARDPAT_MC              (1 << 9)
/* Solo (but Maestro outside of the UK) */
#define CARDPAT_SOLO            (1 << 10)
/* Switch (but Maestro outside of the UK) */
#define CARDPAT_SWITCH          (1 << 11)
/* Maestro UK debit cards (formerly Solo and Switch) */
#define CARDPAT_MAESTRO_UK      (CARDPAT_SOLO | CARDPAT_SWITCH)
/* All Maestro cards (including Laser, Solo and Switch) */
#define CARDPAT_MAESTRO         (CARDPAT_LASER | CARDPAT_MAESTRO_UK | CARDPAT_MAESTRO_CLASSIC)
/* Visa Electron (automatically included in Visa) */
#define CARDPAT_ELECTRON        (1 << 12)
/* Visa */
#define CARDPAT_VISA            (1 << 13)
/* All */
#define CARDPAT_ALL             (CARDPAT_AMEX | CARDPAT_DINERS | CARDPAT_DISC | CARDPAT_JCB | CARDPAT_MAESTRO | CARDPAT_MC | CARDPAT_VISA)

BEGIN_C_DECLS

/**
 * Checks the Luhn-10 checksum of a number.
 *
 * @param  scrubbed_number  Number to check (and contains only digits).
 *
 * @return 0 on failure, or a non-zero value on success.
 */
extern int luhn10(const char* scrubbed_number);

/**
 * Checks a scrubbed number is in the list of valid cards and is well-formed
 * (including if it passes a Luhn-10 checksum).
 *
 * @param  scrubbed_number  Number to check (and contains only digits).
 * @param  valid_types      Bitmask of card types to accept as valid.
 *
 * @return 0 if unvalidated, otherwise the bit representing the card will be set.
 */
extern unsigned long card_number_is_well_formed(const char* scrubbed_number, unsigned long valid_types);

END_C_DECLS

#endif /* !TALIDEON_CARDS__cards_h */
