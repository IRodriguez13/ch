/*
 * ch — version output
 * Copyright (C) 2026 Iván Ezequiel Rodriguez
 * License: GPLv3+
 */

#include "version.h"

#include <stdio.h>

void ch_print_version(void)
{
	printf("ch %s\n", CH_VERSION);
	printf("Copyright (C) %s %s\n", CH_COPYRIGHT_YEAR, CH_AUTHOR);
	printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
	printf("This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law.\n");
	printf("\n");
	printf("Source: %s\n", CH_SOURCE_URL);
	printf("\n");
	printf("Escrito por %s.\n", CH_AUTHOR);
}
