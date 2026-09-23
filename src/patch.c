#include "ch_adds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int append_line(char **buf, size_t *len, size_t *cap, const char *line)
{
	size_t llen = strlen(line) + 1;

	while (*len + llen + 1 >= *cap) {
		size_t ncap = *cap ? *cap * 2 : 4096;
		char *nbuf = realloc(*buf, ncap);

		if (!nbuf)
			return -1;
		*buf = nbuf;
		*cap = ncap;
	}
	memcpy(*buf + *len, line, strlen(line));
	*len += strlen(line);
	(*buf)[(*len)++] = '\n';
	(*buf)[*len] = '\0';
	return 0;
}

int ch_patch_extract(const ch_config_t *cfg, char **out)
{
	char target[CH_PATH_MAX];
	char line[CH_PATH_MAX + 256];
	char *result = NULL;
	size_t result_len = 0;
	size_t result_cap = 0;
	bool active = false;
	FILE *fp;

	ch_normalize_path(cfg->file, target, sizeof(target));

	if (strcmp(cfg->patch_path, "-") == 0)
		fp = stdin;
	else {
		if (!ch_file_exists(cfg->patch_path)) {
			fprintf(stderr, "ch-adds: patch not found: %s\n", cfg->patch_path);
			return -1;
		}
		fp = fopen(cfg->patch_path, "r");
		if (!fp)
			return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		size_t n = strlen(line);

		while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
			line[--n] = '\0';

		if (strncmp(line, "diff --git ", 11) == 0) {
			const char *bpath = strstr(line, " b/");

			active = false;
			if (bpath) {
				bpath += 3;
				if (strcmp(bpath, target) == 0)
					active = true;
			}
			continue;
		}
		if (active && append_line(&result, &result_len, &result_cap, line) != 0) {
			if (fp != stdin)
				fclose(fp);
			free(result);
			return -1;
		}
	}

	if (fp != stdin)
		fclose(fp);

	*out = result ? result : strdup("");
	return 0;
}
