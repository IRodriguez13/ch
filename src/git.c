#include "ch_adds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_git_diff_cmd(const char *range, const char *file, char **out)
{
	char cmd[CH_PATH_MAX * 2 + 128];
	int n;

	if (range && range[0] != '\0')
		n = snprintf(cmd, sizeof(cmd), "git diff -U0 %s -- %s 2>/dev/null", range, file);
	else
		n = snprintf(cmd, sizeof(cmd), "git diff -U0 -- %s 2>/dev/null", file);

	if (n < 0 || (size_t)n >= sizeof(cmd))
		return -1;
	if (ch_popen_read(cmd, out) < 0)
		return -1;
	if ((*out)[0] == '\0') {
		char alt[CH_PATH_MAX + 4];

		snprintf(alt, sizeof(alt), "./%s", file);
		if (range && range[0] != '\0')
			snprintf(cmd, sizeof(cmd), "git diff -U0 %s -- %s 2>/dev/null", range, alt);
		else
			snprintf(cmd, sizeof(cmd), "git diff -U0 -- %s 2>/dev/null", alt);
		free(*out);
		*out = NULL;
		return ch_popen_read(cmd, out);
	}
	return 0;
}

int ch_git_diff(const ch_config_t *cfg, char **out)
{
	*out = NULL;
	return run_git_diff_cmd(cfg->git_range, cfg->file, out);
}
