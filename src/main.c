/*
 * ch-adds — show only added lines from a file diff
 * Copyright (C) 2026 Iván Ezequiel Rodriguez
 * License: GPLv3+
 */

#include "ch_adds.h"
#include "version.h"

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

static bool diff_has_relevant_lines(const ch_config_t *cfg, const char *diff_text)
{
	const char *p = diff_text;

	if (!p)
		return false;
	for (p = diff_text; *p; p++) {
		if (*p != '\n')
			continue;
		p++;
		if (line_is_addition(p))
			return true;
		if (cfg->show_removed && line_is_removal(p))
			return true;
	}
	if (line_is_addition(diff_text))
		return true;
	if (cfg->show_removed && line_is_removal(diff_text))
		return true;
	return false;
}

int main(int argc, char **argv)
{
	ch_config_t cfg;
	char *diff_text = NULL;
	int add_count = 0;
	int rem_count = 0;
	int rc;

	if (ch_config_parse(&cfg, argc, argv) != 0)
		return 1;
	if (cfg.show_help) {
		ch_config_usage(stdout);
		return 0;
	}
	if (cfg.show_version) {
		ch_print_version();
		return 0;
	}
	if (ch_config_finalize(&cfg) != 0)
		return 1;

	if (cfg.mode == CH_MODE_PATCH) {
		if (ch_patch_extract(&cfg, &diff_text) != 0)
			return 1;
	} else {
		if (ch_git_diff(&cfg, &diff_text) != 0) {
			fprintf(stderr, "ch-adds: failed to run git diff\n");
			return 1;
		}
	}

	if (!diff_has_relevant_lines(&cfg, diff_text)) {
		if (!cfg.quiet) {
			char norm[CH_PATH_MAX];
			const char *dim = ch_color_enabled() ? "\033[2m" : "";
			const char *cyan = ch_color_enabled() ? "\033[36m" : "";
			const char *yellow = ch_color_enabled() ? "\033[33m" : "";
			const char *reset = ch_color_enabled() ? "\033[0m" : "";
			const char *what = cfg.show_removed ? "no changes" : "no additions";

			ch_normalize_path(cfg.file, norm, sizeof(norm));
			fprintf(stderr, "%sch-adds:%s %s in %s%s%s vs %s%s%s\n",
				dim, reset, what, cyan, norm, reset, yellow, cfg.ref_label,
				reset);
		}
		free(diff_text);
		return 0;
	}

	rc = ch_render_changes(&cfg, diff_text, &add_count, &rem_count);
	free(diff_text);
	return rc == 0 ? 0 : 1;
}
