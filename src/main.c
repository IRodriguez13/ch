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

static int process_one_file(ch_config_t *cfg, bool skip_empty_msg)
{
	char *diff_text = NULL;
	int add_count = 0;
	int rem_count = 0;
	int rc;

	if (cfg->mode == CH_MODE_PATCH) {
		if (ch_patch_extract(cfg, &diff_text) != 0)
			return 1;
	} else {
		if (ch_git_diff(cfg, &diff_text) != 0) {
			fprintf(stderr, "ch-adds: failed to run git diff\n");
			return 1;
		}
	}

	if (!ch_diff_has_relevant_lines(cfg, diff_text)) {
		if (!cfg->quiet && !skip_empty_msg) {
			char norm[CH_PATH_MAX];
			const char *dim = ch_color_enabled() ? "\033[2m" : "";
			const char *cyan = ch_color_enabled() ? "\033[36m" : "";
			const char *yellow = ch_color_enabled() ? "\033[33m" : "";
			const char *reset = ch_color_enabled() ? "\033[0m" : "";
			const char *what = cfg->show_removed ? "no changes" : "no additions";

			ch_normalize_path(cfg->file, norm, sizeof(norm));
			fprintf(stderr, "%sch-adds:%s %s in %s%s%s vs %s%s%s\n",
				dim, reset, what, cyan, norm, reset, yellow, cfg->ref_label,
				reset);
		}
		free(diff_text);
		return 0;
	}

	rc = ch_render_changes(cfg, diff_text, &add_count, &rem_count);
	free(diff_text);
	return rc == 0 ? 0 : 1;
}

static int process_all_files(ch_config_t *cfg)
{
	char *diff_text = NULL;
	int rc = 0;

	if (ch_git_diff_repo(cfg, &diff_text) != 0) {
		fprintf(stderr, "ch-adds: failed to run git diff\n");
		return 1;
	}
	if (!diff_text || !ch_diff_has_relevant_lines(cfg, diff_text)) {
		if (!cfg->quiet) {
			const char *dim = ch_color_enabled() ? "\033[2m" : "";
			const char *yellow = ch_color_enabled() ? "\033[33m" : "";
			const char *reset = ch_color_enabled() ? "\033[0m" : "";
			const char *what = cfg->show_removed ? "no changes" : "no additions";

			fprintf(stderr, "%sch-adds:%s %s in repository vs %s%s%s\n",
				dim, reset, what, yellow, cfg->ref_label, reset);
		}
		free(diff_text);
		return 0;
	}
	rc = ch_render_repo_diff(cfg, diff_text);
	free(diff_text);
	return rc == 0 ? 0 : 1;
}

static int process_file_list(ch_config_t *cfg)
{
	int rc = 0;
	bool any = false;

	for (int i = 0; i < cfg->file_count; i++) {
		char *diff_text = NULL;
		int adds = 0;
		int rems = 0;

		snprintf(cfg->file, sizeof(cfg->file), "%s", cfg->files[i]);
		if (cfg->mode == CH_MODE_PATCH) {
			if (ch_patch_extract(cfg, &diff_text) != 0)
				return 1;
		} else if (ch_git_diff(cfg, &diff_text) != 0) {
			fprintf(stderr, "ch-adds: failed to run git diff\n");
			return 1;
		}
		if (!ch_diff_has_relevant_lines(cfg, diff_text)) {
			free(diff_text);
			continue;
		}
		if (any && !cfg->quiet)
			printf("\n");
		if (ch_render_changes(cfg, diff_text, &adds, &rems) != 0)
			rc = 1;
		else
			any = true;
		free(diff_text);
	}
	return rc;
}

int main(int argc, char **argv)
{
	ch_config_t cfg;
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

	if (cfg.all_files)
		rc = process_all_files(&cfg);
	else if (cfg.file_count > 1)
		rc = process_file_list(&cfg);
	else
		rc = process_one_file(&cfg, false);
	return rc;
}
