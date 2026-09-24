#include "ch_adds.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

static void ch_config_init(ch_config_t *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	snprintf(cfg->ref, sizeof(cfg->ref), "HEAD");
	cfg->mode = CH_MODE_GIT;
	cfg->scope = CH_SCOPE_HEAD;
}

static bool ch_looks_like_ref(const char *arg)
{
	char alt[CH_PATH_MAX + 4];

	if (strcmp(arg, "HEAD") == 0)
		return true;
	if (strstr(arg, "...") != NULL || strstr(arg, "..") != NULL)
		return true;
	if (strstr(arg, ".patch") != NULL)
		return true;
	if (ch_file_exists(arg))
		return false;
	snprintf(alt, sizeof(alt), "./%s", arg);
	return !ch_file_exists(alt);
}

static bool ch_is_all_token(const char *arg)
{
	return strcmp(arg, ".") == 0 || strcmp(arg, "*") == 0;
}

static int ch_add_file(ch_config_t *cfg, const char *path)
{
	if (cfg->file_count >= CH_MAX_FILES) {
		fprintf(stderr, "ch-adds: too many files (max %d)\n", CH_MAX_FILES);
		return -1;
	}
	snprintf(cfg->files[cfg->file_count], CH_PATH_MAX, "%s", path);
	cfg->file_count++;
	return 0;
}

void ch_config_usage(FILE *out)
{
	fprintf(out,
		"Usage: ch-adds [file...] [reference] [options]\n"
		"       ch-adds [options]                 (all changed files in repo)\n"
		"\n"
		"Show only added lines (+) from a file vs a git ref or patch file.\n"
		"\n"
		"References (optional, default: HEAD):\n"
		"  HEAD                 working tree + staged vs last commit\n"
		"  <commit|tag>         git diff <ref> -- file\n"
		"  develop              git diff develop...HEAD (MR style, 3-dot)\n"
		"  develop..HEAD        explicit 2-dot range\n"
		"  develop...HEAD       explicit 3-dot range\n"
		"  changes.patch        extract + lines for that file from patch\n"
		"\n"
		"Options:\n"
		"  -r, --ref REF        reference (alternative to 2nd argument)\n"
		"  -p, --patch FILE     read additions from a .patch (ignore git)\n"
		"      --staged         only staged changes (git diff --cached)\n"
		"      --unstaged       only unstaged changes (git diff)\n"
		"  -2, --two-dot        branch name -> branch..HEAD\n"
		"  -x, --exact          git diff REF -- (single tree, no range)\n"
		"  -q, --quiet          only changed text, no header or line numbers\n"
		"  -0, --plain          with -q: omit leading +/- on each line\n"
		"  -d, --removed        also show removed lines (-), in diff order\n"
		"  -a, --all            all changed files in the repo (default with no file)\n"
		"  -h, --help           show this help\n"
		"  -V, --version        show version and license\n"
		"\n"
		"Examples:\n"
		"  ch-adds dojo/home/views.py\n"
		"  ch-adds k8s/pre/all/values.yaml develop\n"
		"  ch-adds dojo/foo.py --staged\n"
		"  ch-adds dojo/foo.py -p /tmp/fix.patch\n"
		"  ch-adds dojo/foo.py develop..HEAD\n"
		"  ch-adds -d                         # all changed files, + and -\n"
		"  ch-adds -a --staged              # all staged files\n");
}

static void ch_resolve_git_range(ch_config_t *cfg)
{
	if (cfg->scope == CH_SCOPE_STAGED) {
		snprintf(cfg->git_range, sizeof(cfg->git_range), "--cached");
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "staged (--cached)");
		return;
	}
	if (cfg->scope == CH_SCOPE_UNSTAGED) {
		cfg->git_range[0] = '\0';
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "unstaged (working tree)");
		return;
	}
	if (strcmp(cfg->ref, "HEAD") == 0) {
		snprintf(cfg->git_range, sizeof(cfg->git_range), "HEAD");
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "HEAD (working tree + staged)");
		return;
	}
	if (strstr(cfg->ref, "...") || strstr(cfg->ref, "..")) {
		snprintf(cfg->git_range, sizeof(cfg->git_range), "%s", cfg->ref);
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "%s", cfg->ref);
		return;
	}
	if (cfg->exact_ref) {
		snprintf(cfg->git_range, sizeof(cfg->git_range), "%s", cfg->ref);
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "%s (exact)", cfg->ref);
		return;
	}
	if (cfg->two_dot) {
		snprintf(cfg->git_range, sizeof(cfg->git_range), "%s..HEAD", cfg->ref);
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "%s..HEAD", cfg->ref);
		return;
	}
	snprintf(cfg->git_range, sizeof(cfg->git_range), "%s...HEAD", cfg->ref);
	snprintf(cfg->ref_label, sizeof(cfg->ref_label), "%s...HEAD", cfg->ref);
}

int ch_config_finalize(ch_config_t *cfg)
{
	char alt[CH_PATH_MAX + 4];

	if (cfg->mode == CH_MODE_PATCH) {
		snprintf(cfg->ref_label, sizeof(cfg->ref_label), "patch:%.200s", cfg->patch_path);
		return 0;
	}
	if (cfg->all_files) {
		ch_resolve_git_range(cfg);
		return 0;
	}
	if (cfg->file_count > 0)
		snprintf(cfg->file, sizeof(cfg->file), "%s", cfg->files[0]);
	if (!ch_file_exists(cfg->file)) {
		snprintf(alt, sizeof(alt), "./%s", cfg->file);
		if (!ch_file_exists(alt)) {
			fprintf(stderr, "ch-adds: file not found: %s\n", cfg->file);
			return 1;
		}
	}
	ch_resolve_git_range(cfg);
	return 0;
}

int ch_config_parse(ch_config_t *cfg, int argc, char **argv)
{
	static struct option long_opts[] = {
		{"ref", required_argument, NULL, 'r'},
		{"patch", required_argument, NULL, 'p'},
		{"staged", no_argument, NULL, 'S'},
		{"unstaged", no_argument, NULL, 'U'},
		{"two-dot", no_argument, NULL, '2'},
		{"exact", no_argument, NULL, 'x'},
		{"quiet", no_argument, NULL, 'q'},
		{"plain", no_argument, NULL, '0'},
		{"removed", no_argument, NULL, 'd'},
		{"all", no_argument, NULL, 'a'},
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'V'},
		{NULL, 0, NULL, 0}
	};
	int positional = 0;
	int opt;

	ch_config_init(cfg);
	while ((opt = getopt_long(argc, argv, "r:p:2xq0dhSaSUV", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'r':
			snprintf(cfg->ref, sizeof(cfg->ref), "%s", optarg);
			break;
		case 'p':
			snprintf(cfg->patch_path, sizeof(cfg->patch_path), "%s", optarg);
			cfg->mode = CH_MODE_PATCH;
			break;
		case 'S':
			cfg->scope = CH_SCOPE_STAGED;
			break;
		case 'U':
			cfg->scope = CH_SCOPE_UNSTAGED;
			break;
		case '2':
			cfg->two_dot = true;
			break;
		case 'x':
			cfg->exact_ref = true;
			break;
		case 'q':
			cfg->quiet = true;
			break;
		case '0':
			cfg->plain = true;
			break;
		case 'd':
			cfg->show_removed = true;
			break;
		case 'a':
			cfg->all_files = true;
			break;
		case 'h':
			cfg->show_help = true;
			return 0;
		case 'V':
			cfg->show_version = true;
			return 0;
		default:
			return 1;
		}
	}
	for (int i = optind; i < argc; i++)
		positional++;

	if (positional == 0) {
		if (!cfg->show_help && !cfg->show_version && cfg->mode != CH_MODE_PATCH)
			cfg->all_files = true;
	} else if (positional == 1) {
		if (ch_is_all_token(argv[optind]))
			cfg->all_files = true;
		else if (ch_looks_like_ref(argv[optind]) && cfg->mode != CH_MODE_PATCH) {
			snprintf(cfg->ref, sizeof(cfg->ref), "%s", argv[optind]);
			cfg->all_files = true;
		} else if (ch_add_file(cfg, argv[optind]) != 0)
			return 1;
	} else {
		int last = argc - 1;
		const char *ref_arg = NULL;

		if (ch_looks_like_ref(argv[last]) && strcmp(cfg->ref, "HEAD") == 0) {
			ref_arg = argv[last];
			last--;
		}
		for (int i = optind; i <= last; i++) {
			if (ch_is_all_token(argv[i])) {
				cfg->all_files = true;
				cfg->file_count = 0;
				break;
			}
			if (ch_add_file(cfg, argv[i]) != 0)
				return 1;
		}
		if (ref_arg)
			snprintf(cfg->ref, sizeof(cfg->ref), "%s", ref_arg);
	}
	if (cfg->all_files)
		cfg->file_count = 0;
	if (!cfg->all_files && cfg->file_count == 0 && !cfg->show_help && !cfg->show_version) {
		ch_config_usage(stderr);
		return 1;
	}
	if (cfg->show_help || cfg->show_version)
		return 0;
	if (cfg->mode != CH_MODE_PATCH && strlen(cfg->patch_path) == 0 &&
	    strstr(cfg->ref, ".patch") != NULL) {
		snprintf(cfg->patch_path, sizeof(cfg->patch_path), "%s", cfg->ref);
		cfg->mode = CH_MODE_PATCH;
	}
	if (cfg->mode == CH_MODE_PATCH && strlen(cfg->patch_path) == 0) {
		fprintf(stderr, "ch-adds: missing patch file (-p)\n");
		return 1;
	}
	if (cfg->scope != CH_SCOPE_HEAD && cfg->mode == CH_MODE_PATCH) {
		fprintf(stderr, "ch-adds: --staged/--unstaged only apply to git mode\n");
		return 1;
	}
	return 0;
}
