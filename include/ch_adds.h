#ifndef CH_ADDS_H
#define CH_ADDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CH_PATH_MAX 4096
#define CH_REF_MAX 256
#define CH_MAX_FILES 512

typedef enum {
	CH_MODE_GIT,
	CH_MODE_PATCH
} ch_mode_t;

typedef enum {
	CH_SCOPE_HEAD,
	CH_SCOPE_STAGED,
	CH_SCOPE_UNSTAGED
} ch_scope_t;

typedef struct {
	char file[CH_PATH_MAX];
	char ref[CH_REF_MAX];
	char patch_path[CH_PATH_MAX];
	char ref_label[CH_REF_MAX + 64];
	char git_range[CH_REF_MAX + 64];
	ch_mode_t mode;
	ch_scope_t scope;
	bool quiet;
	bool plain;
	bool two_dot;
	bool exact_ref;
	bool show_help;
	bool show_version;
	bool show_removed;
	bool all_files;
	int file_count;
	char files[CH_MAX_FILES][CH_PATH_MAX];
} ch_config_t;

int ch_config_parse(ch_config_t *cfg, int argc, char **argv);
int ch_config_finalize(ch_config_t *cfg);
void ch_config_usage(FILE *out);

int ch_git_diff(const ch_config_t *cfg, char **out);
int ch_git_diff_repo(const ch_config_t *cfg, char **out);
int ch_patch_extract(const ch_config_t *cfg, char **out);

int ch_render_changes(const ch_config_t *cfg, const char *diff_text,
		      int *add_count_out, int *rem_count_out);
int ch_render_repo_diff(const ch_config_t *cfg, const char *diff_text);
bool ch_diff_has_relevant_lines(const ch_config_t *cfg, const char *diff_text);

void ch_normalize_path(const char *in, char *out, size_t out_sz);
bool ch_file_exists(const char *path);
bool ch_color_enabled(void);
int ch_read_file(const char *path, char **out);
int ch_popen_read(const char *cmd, char **out);

#endif
