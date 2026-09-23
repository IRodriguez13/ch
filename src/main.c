#include "ch_adds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool diff_has_additions(const char *diff_text)
{
	const char *p = diff_text;

	if (!p)
		return false;
	while ((p = strchr(p, '\n')) != NULL) {
		p++;
		if (p[0] == '+' && p[1] != '+')
			return true;
	}
	return diff_text[0] == '+' && diff_text[1] != '+';
}

int main(int argc, char **argv)
{
	ch_config_t cfg;
	char *diff_text = NULL;
	int count = 0;
	int rc;

	if (ch_config_parse(&cfg, argc, argv) != 0)
		return 1;
	if (cfg.show_help) {
		ch_config_usage(stdout);
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

	if (!diff_text || !diff_has_additions(diff_text)) {
		if (!cfg.quiet) {
			char norm[CH_PATH_MAX];
			const char *dim = ch_color_enabled() ? "\033[2m" : "";
			const char *cyan = ch_color_enabled() ? "\033[36m" : "";
			const char *yellow = ch_color_enabled() ? "\033[33m" : "";
			const char *reset = ch_color_enabled() ? "\033[0m" : "";

			ch_normalize_path(cfg.file, norm, sizeof(norm));
			fprintf(stderr, "%sch-adds:%s no additions in %s%s%s vs %s%s%s\n",
				dim, reset, cyan, norm, reset, yellow, cfg.ref_label, reset);
		}
		free(diff_text);
		return 0;
	}

	rc = ch_render_additions(&cfg, diff_text, &count);
	free(diff_text);
	return rc == 0 ? 0 : 1;
}
