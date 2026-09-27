#include "ch.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CH_DIFF_ARG_MAX 128

static void ch_join_path(const char *dir, const char *rel, char *out, size_t out_sz)
{
	const char *r = rel;

	if (!dir || !out || out_sz == 0)
		return;
	if (!r || r[0] == '\0') {
		snprintf(out, out_sz, "%s", dir);
		return;
	}
	while (r[0] == '.' && r[1] == '/')
		r += 2;
	snprintf(out, out_sz, "%s/%s", dir, r);
}

static bool ch_exclude_list_has(const ch_config_t *cfg, const char *pattern)
{
	for (int i = 0; i < cfg->exclude_count; i++) {
		if (strcmp(cfg->excludes[i], pattern) == 0)
			return true;
	}
	return false;
}

static int ch_diff_build_argv(const ch_config_t *cfg, const char *path_a,
			      const char *path_b, char *arg_storage[],
			      char *exclude_bufs[], int *argc_out)
{
	int argc = 0;

	arg_storage[argc++] = "diff";
	arg_storage[argc++] = "-ruN";

	if (!ch_exclude_list_has(cfg, ".git")) {
		static char git_ex[] = "--exclude=.git";

		arg_storage[argc++] = git_ex;
	}
	for (int i = 0; i < cfg->exclude_count; i++) {
		snprintf(exclude_bufs[i], CH_EXCLUDE_MAX, "--exclude=%s",
			 cfg->excludes[i]);
		arg_storage[argc++] = exclude_bufs[i];
	}
	arg_storage[argc++] = (char *)path_a;
	arg_storage[argc++] = (char *)path_b;
	arg_storage[argc++] = NULL;
	*argc_out = argc - 1;
	return 0;
}

static int ch_diff_run(char *const argv[], char **out)
{
	int pipefd[2];
	pid_t pid;
	int status;
	FILE *fp;
	int len;

	if (pipe(pipefd) != 0)
		return -1;
	pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}
	if (pid == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		(void)dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
		execvp("diff", argv);
		_exit(127);
	}
	close(pipefd[1]);
	fp = fdopen(pipefd[0], "r");
	if (!fp) {
		close(pipefd[0]);
		(void)waitpid(pid, &status, 0);
		return -1;
	}
	len = ch_read_stream(fp, out);
	fclose(fp);
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	status = WEXITSTATUS(status);
	if (status == 2)
		return -1;
	if (len < 0)
		return -1;
	return 0;
}

static int ch_diff_paths(const ch_config_t *cfg, const char *path_a,
			 const char *path_b, char **out)
{
	char *argv[CH_DIFF_ARG_MAX];
	char exclude_bufs[CH_MAX_EXCLUDES][CH_EXCLUDE_MAX];
	char *exclude_ptrs[CH_MAX_EXCLUDES];
	int argc = 0;

	for (int i = 0; i < CH_MAX_EXCLUDES; i++)
		exclude_ptrs[i] = exclude_bufs[i];
	if (ch_diff_build_argv(cfg, path_a, path_b, argv, exclude_ptrs, &argc) != 0)
		return -1;
	return ch_diff_run(argv, out);
}

int ch_diff_tree(const ch_config_t *cfg, char **out)
{
	return ch_diff_paths(cfg, cfg->diff_dir_a, cfg->diff_dir_b, out);
}

int ch_diff_file(const ch_config_t *cfg, const char *rel_path, char **out)
{
	char path_a[CH_PATH_MAX];
	char path_b[CH_PATH_MAX];

	ch_join_path(cfg->diff_dir_a, rel_path, path_a, sizeof(path_a));
	ch_join_path(cfg->diff_dir_b, rel_path, path_b, sizeof(path_b));
	return ch_diff_paths(cfg, path_a, path_b, out);
}

static void ch_diff_copy_rel(const char *p, char *out, size_t out_sz)
{
	size_t i = 0;

	while (p[i] != '\0' && p[i] != '\t' && p[i] != '\n' && i + 1 < out_sz) {
		out[i] = p[i];
		i++;
	}
	out[i] = '\0';
}

void ch_diff_display_path(const ch_config_t *cfg, const char *path, char *out,
			  size_t out_sz)
{
	const char *p = path;
	size_t len_a = strlen(cfg->diff_dir_a);
	size_t len_b = strlen(cfg->diff_dir_b);
	char rel[CH_PATH_MAX];

	if (!path || !out || out_sz == 0)
		return;
	if (strncmp(p, "+++ ", 4) == 0)
		p += 4;
	else if (strncmp(p, "--- ", 4) == 0)
		p += 4;
	if (strncmp(p, "/dev/null", 9) == 0) {
		out[0] = '\0';
		return;
	}
	while (*p == ' ')
		p++;
	if (strncmp(p, cfg->diff_dir_b, len_b) == 0 &&
	    (p[len_b] == '/' || p[len_b] == '\0')) {
		p += len_b;
		if (*p == '/')
			p++;
		ch_diff_copy_rel(p, out, out_sz);
		return;
	}
	if (strncmp(p, cfg->diff_dir_a, len_a) == 0 &&
	    (p[len_a] == '/' || p[len_a] == '\0')) {
		p += len_a;
		if (*p == '/')
			p++;
		ch_diff_copy_rel(p, out, out_sz);
		return;
	}
	ch_normalize_path(p, rel, sizeof(rel));
	ch_diff_copy_rel(rel, out, out_sz);
}
