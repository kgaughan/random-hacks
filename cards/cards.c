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

#include "cards.h"

struct CardPattern {
	/* Each bit that's set indicates a valid length. */
	unsigned long lengths;
	/* List of prefixes. */
	char* prefixes[];
};

static const struct CardPattern cp_amex = {
	1 << 15, {
		"34",
		"37",
		NULL
	}
};

static const struct CardPattern cp_cup = {
	(1 << 16) | (1 << 17) | (1 << 18) | (1 << 19), {
		"622126",
		"622127",
		"622128",
		"622129",
		"62213",
		"62214",
		"62215",
		"62216",
		"62217",
		"62218",
		"62219",
		"6222",
		"6223",
		"6224",
		"6225",
		"6226",
		"6227",
		"6228",
		"62290",
		"62291",
		"622920",
		"622921",
		"622922",
		"622923",
		"622924",
		"622925",
		NULL
	}
};

static const struct CardPattern cp_cb = {
	1 << 14, {
		"300",
		"301",
		"302",
		"303",
		"304",
		"305",
		NULL
	}
};

static const struct CardPattern cp_dc = {
	1 << 14, {
		"36",
		NULL
	}
};

static const struct CardPattern cp_disc = {
	1 << 16, {
		"6011",
		"65",
		NULL
	}
};

static const struct CardPattern cp_jcb1 = {
	1 << 16, {
		"35",
		NULL
	}
};

static const struct CardPattern cp_jcb2 = {
	1 << 15, {
		"1800",
		"2131",
		NULL
	}
};

static const struct CardPattern cp_laser = {
	(1 << 16) | (1 << 17) | (1 << 18) | (1 << 19), {
		"6304",
		"6706",
		"6771",
		"6709",
		NULL
	}
};

static const struct CardPattern cp_maestro = {
	(1 << 16) | (1 << 18), {
		"5020",
		"5038",
		"6304",
		"6759",
		NULL
	}
};

static const struct CardPattern cp_mc = {
	1 << 16, {
		"51",
		"52",
		"53",
		"54",
		"55",
		NULL
	}
};

static const struct CardPattern cp_solo = {
	(1 << 16) | (1 << 18) | (1 << 19), {
		"6334",
		"6767",
		NULL
	}
};

static const struct CardPattern cp_switch = {
	(1 << 16) | (1 << 18) | (1 << 19), {
		"4903",
		"4905",
		"4911",
		"4936",
		"564182",
		"633110",
		"6333",
		"6759",
		NULL
	}
};

static const struct CardPattern cp_visa = {
	(1 << 13) | (1 << 16), {
		"4",
		NULL
	}
};

static const struct CardPattern cp_electron = {
	1 << 16, {
		/* Bit of an overlap with regular VISA there, but there's method.. */
		"417500",
		"4917",
		"4913",
		"4508",
		"4844",
		NULL
	}
};

static const struct CardPattern * const patterns[] = {
	&cp_amex,     /*  0: American Express */
	&cp_cup,      /*  1: China Union Pay */
	&cp_cb,       /*  2: Carte Blanche */
	&cp_dc,       /*  3: Diner's Card International */
	&cp_disc,     /*  4: Discover Card */
	&cp_jcb1,     /*  5: JCB */
	&cp_jcb2,     /*  6: JCB */
	&cp_laser,    /*  7: Laser */
	&cp_maestro,  /*  8: Maestro */
	&cp_mc,       /*  9: MasterCard */
	&cp_solo,     /* 10: Solo */
	&cp_switch,   /* 11: Switch */
	&cp_electron, /* 12: Electron */
	&cp_visa      /* 13: Visa */
};

int
luhn10(const char* scrubbed_number) {
	/* Lookup table to simplify the logic. */
	static const int odd[]  = { 0, 2, 4, 6, 8, 1, 3, 5, 7, 9 };

	int sum;
	int len;
	const char* pch;
	int alt;

	/* The number must be at least two (including the check) digits long. */
	len = strlen(scrubbed_number);
	if (len < 2) {
		return 0;
	}

	alt = 0;
	sum = 0;
	pch = scrubbed_number + len - 1;
	while (pch >= scrubbed_number) {
		if (*pch < '0' || *pch > '9') {
			/* Somebody's trying to tamper with us! */
			return 0;
		}
		sum += alt ? odd[*pch - '0'] : *pch - '0';
		if (sum >= 10) {
			sum -= 10;
		}
		alt = !alt;
		pch--;
	}
	return sum == 0;
}

static int
is_prefixed_by(const char* s, const char* prefix) {
	while (*prefix != '\0' && *s == *prefix) {
		s++;
		prefix++;
	}
	return *prefix == '\0';
}

unsigned long
card_number_is_well_formed(const char* scrubbed_number, unsigned long valid_types) {
	unsigned i;
	unsigned len;
	char* const* pprefix;

	if (!luhn10(scrubbed_number)) {
		return 0;
	}

	len = strlen(scrubbed_number);
	for (i = 0; i < ARRAY_SIZE(patterns); i++) {
		/* Is in the list of valid types and is the right length. */
		if ((valid_types & (1 << i)) != 0 && (patterns[i]->lengths & (1 << len)) != 0) {
			for (pprefix = patterns[i]->prefixes; *pprefix != NULL; pprefix++) {
				if (is_prefixed_by(scrubbed_number, *pprefix)) {
					return 1 << i;
				}
			}
		}
	}
	return 0;
}

#ifdef TEST_MAIN
#include <stdio.h>

static const char* good_checksums[] = {
	"00"
};

static const char* good_cards[] = {
	"4005559876540"
};

static const char* bad_checksums[] = {
	"",
	"0",
	"4005559876541"
};

static const char* malformed_cards[] = {
	"00",
	"400555987654",
	"40055598765400"
};

#define RUN_TEST(data, test) (run_test(#test, ARRAY_SIZE(data), (data), (test_ ## test)))

static int
run_test(const char* name, size_t n, const char* tests[], int (fn)(const char*)) {
	size_t i;
	int passed;
	int failed;

	passed = 0;
	printf("Executing '%s' [", name);
	for (i = 0; i < n; i++) {
		if (fn(tests[i])) {
			putchar('.');
			passed++;
		} else {
			putchar('x');
		}
	}
	failed = n - passed;
	printf("]\n%d executed, %d passed, %d failed.\n\n", n, passed, failed);
	return failed;
}

static int
test_luhn10(const char* number) {
	return luhn10(number) != 0;
}

static int
test_well_formed(const char* number) {
	return card_number_is_well_formed(number, CARDPAT_ALL) != 0;
}

static int
test_bad_luhn10(const char* number) {
	return luhn10(number) == 0;
}

static int
test_malformed(const char* number) {
	return card_number_is_well_formed(number, CARDPAT_ALL) == 0;
}

int
main(void) {
	int failed;
	printf("Executing tests...\n\n");
	failed  = RUN_TEST(good_checksums, luhn10);
	failed += RUN_TEST(good_cards, well_formed);
	failed += RUN_TEST(bad_checksums, bad_luhn10);
	failed += RUN_TEST(malformed_cards, malformed);
	if (failed > 0) {
		printf("FAILURE: %d failed.\n", failed);
		return 1;
	}
	printf("Success!\n");
	return 0;
}
#endif /* TEST_MAIN */
