#include "ch_adds.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool line_is_addition(const char *line)
{
	return line[0] == '+' && line[1] != '+';
}

static bool line_is_removal(const char *line)
{
	return line[0] == '-' && line[1] != '-';
}

static bool line_is_context(const char *line)
{
	return line[0] == ' ' && line[1] != '\0';
}

static int parse_hunk_lines(const char *hunk, int *old_line, int *new_line)
{
	const char *p = hunk;

	if (strncmp(p, "@@", 2) != 0)
		return 0;
	p += 2;
	while (*p == ' ')
		p++;
	if (*p != '-')
		return 0;
	p++;
	*old_line = atoi(p);
	while (*p && *p != ' ')
		p++;
	while (*p == ' ')
		p++;
	if (*p != '+')
		return 0;
	p++;
	*new_line = atoi(p);
	return 1;
}

static void count_changes(const char *diff_text, bool count_removals, int *adds, int *rems)
{
	char *copy = strdup(diff_text ? diff_text : "");
	char *save = NULL;
	char *line;

	*adds = 0;
	*rems = 0;
	if (!copy)
		return;
	line = strtok_r(copy, "\n", &save);
	while (line) {
		if (line_is_addition(line))
			(*adds)++;
		else if (count_removals && line_is_removal(line))
			(*rems)++;
		line = strtok_r(NULL, "\n", &save);
	}
	free(copy);
}

static void print_header(const ch_config_t *cfg, int adds, int rems)
{
	const char *bold = ch_color_enabled() ? "\033[1m" : "";
	const char *cyan = ch_color_enabled() ? "\033[36m" : "";
	const char *dim = ch_color_enabled() ? "\033[2m" : "";
	const char *yellow = ch_color_enabled() ? "\033[33m" : "";
	const char *green = ch_color_enabled() ? "\033[32m" : "";
	const char *red = ch_color_enabled() ? "\033[31m" : "";
	const char *reset = ch_color_enabled() ? "\033[0m" : "";
	char norm[CH_PATH_MAX];

	ch_normalize_path(cfg->file, norm, sizeof(norm));
	printf("%s%s┌─%s %s%s%s\n", bold, cyan, reset, bold, norm, reset);
	printf("%s│%s ref: %s%s%s\n", dim, reset, yellow, cfg->ref_label, reset);
	if (cfg->show_removed && rems > 0) {
		if (adds > 0) {
			printf("%s│%s %s+%d%s  %s-%d%s change(s)\n",
			       dim, reset, green, adds, reset, red, rems, reset);
		} else {
			printf("%s│%s %s-%d%s line(s)\n", dim, reset, red, rems, reset);
		}
	} else {
		printf("%s│%s %s+%d%s line(s)\n", dim, reset, green, adds, reset);
	}
	printf("%s%s└%s%s────────────────────────────────────────%s\n",
	       bold, cyan, reset, dim, reset);
}

static void print_change_line(const ch_config_t *cfg, int line_no, char sign,
			      const char *color, const char *text)
{
	const char *dim = ch_color_enabled() ? "\033[2m" : "";
	const char *reset = ch_color_enabled() ? "\033[0m" : "";

	if (cfg->quiet) {
		if (cfg->plain)
			printf("%s\n", text);
		else
			printf("%c%s\n", sign, text);
		return;
	}
	printf("  %s%4d%s │ %s%c%s%s\n", dim, line_no, reset, color, sign, text, reset);
}

static void print_changes(const ch_config_t *cfg, const char *diff_text)
{
	char *copy = strdup(diff_text ? diff_text : "");
	char *save = NULL;
	char *line;
	int old_cur = 0;
	int new_cur = 0;
	const char *green = ch_color_enabled() ? "\033[32m" : "";
	const char *red = ch_color_enabled() ? "\033[31m" : "";

	if (!copy)
		return;
	line = strtok_r(copy, "\n", &save);
	while (line) {
		if (strncmp(line, "@@", 2) == 0)
			parse_hunk_lines(line, &old_cur, &new_cur);
		else if (line_is_context(line)) {
			old_cur++;
			new_cur++;
		} else if (line_is_removal(line)) {
			if (cfg->show_removed)
				print_change_line(cfg, old_cur, '-', red, line + 1);
			old_cur++;
		} else if (line_is_addition(line)) {
			print_change_line(cfg, new_cur, '+', green, line + 1);
			new_cur++;
		}
		line = strtok_r(NULL, "\n", &save);
	}
	free(copy);
}

int ch_render_changes(const ch_config_t *cfg, const char *diff_text,
		      int *add_count_out, int *rem_count_out)
{
	int adds = 0;
	int rems = 0;

	count_changes(diff_text, cfg->show_removed, &adds, &rems);
	if (add_count_out)
		*add_count_out = adds;
	if (rem_count_out)
		*rem_count_out = rems;
	if (adds == 0 && rems == 0)
		return 0;
	if (!cfg->quiet)
		print_header(cfg, adds, rems);
	print_changes(cfg, diff_text);
	return 0;
}
