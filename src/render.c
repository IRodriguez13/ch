#include "ch_adds.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool line_is_addition(const char *line)
{
	return line[0] == '+' && line[1] != '+';
}

static int parse_new_line(const char *hunk, int *cur)
{
	const char *p = strstr(hunk, "+");

	if (!p)
		return 0;
	p++;
	while (*p && !isdigit((unsigned char)*p))
		p++;
	if (!isdigit((unsigned char)*p))
		return 0;
	*cur = atoi(p);
	return 1;
}

static void print_header(const ch_config_t *cfg, int count)
{
	const char *bold = ch_color_enabled() ? "\033[1m" : "";
	const char *cyan = ch_color_enabled() ? "\033[36m" : "";
	const char *dim = ch_color_enabled() ? "\033[2m" : "";
	const char *yellow = ch_color_enabled() ? "\033[33m" : "";
	const char *green = ch_color_enabled() ? "\033[32m" : "";
	const char *reset = ch_color_enabled() ? "\033[0m" : "";
	char norm[CH_PATH_MAX];

	ch_normalize_path(cfg->file, norm, sizeof(norm));
	printf("%s%s┌─%s %s%s%s\n", bold, cyan, reset, bold, norm, reset);
	printf("%s│%s ref: %s%s%s\n", dim, reset, yellow, cfg->ref_label, reset);
	printf("%s│%s %s+%d%s line(s)\n", dim, reset, green, count, reset);
	printf("%s%s└%s%s────────────────────────────────────────%s\n",
	       bold, cyan, reset, dim, reset);
}

static int count_additions(const char *diff_text)
{
	char *copy = strdup(diff_text ? diff_text : "");
	char *save = NULL;
	char *line;
	int count = 0;

	if (!copy)
		return 0;
	line = strtok_r(copy, "\n", &save);
	while (line) {
		if (line_is_addition(line))
			count++;
		line = strtok_r(NULL, "\n", &save);
	}
	free(copy);
	return count;
}

static void print_additions(const ch_config_t *cfg, const char *diff_text)
{
	char *copy = strdup(diff_text ? diff_text : "");
	char *save = NULL;
	char *line;
	int cur = 0;
	const char *dim = ch_color_enabled() ? "\033[2m" : "";
	const char *green = ch_color_enabled() ? "\033[32m" : "";
	const char *reset = ch_color_enabled() ? "\033[0m" : "";

	if (!copy)
		return;
	line = strtok_r(copy, "\n", &save);
	while (line) {
		if (strncmp(line, "@@", 2) == 0)
			parse_new_line(line, &cur);
		else if (line_is_addition(line)) {
			const char *text = line + 1;

			if (cfg->quiet) {
				if (cfg->plain)
					printf("%s\n", text);
				else
					printf("+%s\n", text);
			} else {
				printf("  %s%4d%s │ %s+%s%s\n", dim, cur, reset, green, text, reset);
			}
			cur++;
		}
		line = strtok_r(NULL, "\n", &save);
	}
	free(copy);
}

int ch_render_additions(const ch_config_t *cfg, const char *diff_text, int *count_out)
{
	int count = count_additions(diff_text);

	if (count_out)
		*count_out = count;
	if (count == 0)
		return 0;
	if (!cfg->quiet)
		print_header(cfg, count);
	print_additions(cfg, diff_text);
	return 0;
}
