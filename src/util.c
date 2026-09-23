#include "ch_adds.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ch_normalize_path(const char *in, char *out, size_t out_sz)
{
	const char *p = in;

	if (!in || !out || out_sz == 0)
		return;

	if (strncmp(p, "./", 2) == 0)
		p += 2;
	if (strncmp(p, "a/", 2) == 0)
		p += 2;
	if (strncmp(p, "b/", 2) == 0)
		p += 2;

	snprintf(out, out_sz, "%s", p);
}

bool ch_file_exists(const char *path)
{
	return path && access(path, F_OK) == 0;
}

bool ch_color_enabled(void)
{
	const char *no_color = getenv("NO_COLOR");

	if (no_color && no_color[0] != '\0')
		return false;
	return isatty(STDOUT_FILENO);
}

static int read_all_stream(FILE *fp, char **out)
{
	size_t cap = 8192;
	size_t len = 0;
	char *buf = malloc(cap);

	if (!buf)
		return -1;

	buf[0] = '\0';
	while (1) {
		if (len + 4096 >= cap) {
			char *nbuf = realloc(buf, cap * 2);

			if (!nbuf) {
				free(buf);
				return -1;
			}
			buf = nbuf;
			cap *= 2;
		}
		if (!fgets(buf + len, (int)(cap - len), fp))
			break;
		len += strlen(buf + len);
	}
	*out = buf;
	return (int)len;
}

int ch_read_file(const char *path, char **out)
{
	FILE *fp = fopen(path, "r");

	if (!fp)
		return -1;
	return read_all_stream(fp, out);
}

int ch_popen_read(const char *cmd, char **out)
{
	FILE *fp = popen(cmd, "r");

	if (!fp)
		return -1;
	return read_all_stream(fp, out);
}
