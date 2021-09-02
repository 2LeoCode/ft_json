#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <ft_json_int.h>

#define FAIL_IF(expr); {\
	if (expr) {\
		close(fd);\
		return (NULL);\
	}\
}

static t_token	**tokenize_file(int fd, size_t *token_cnt) {
	struct stat	fd_stats;
	const int	ret = fstat(fd, &fd_stats);
	t_token		**token = NULL;
	char		*file_data;

	FAIL_IF (ret == -1);
	file_data = mmap(NULL, fd_stats.st_size, PROT_READ,
		MAP_PRIVATE, fd, 0)
	FAIL_IF (!file_data);
	token = json_tokenizer(file_data, fd_stats.st_size, token_cnt);
	munmap(file_data, fd_stats.st_size);
	close(fd);
	return (token);
}

#define PRINT_TMP_BLOCK(c, cnt) {\
	char	t[cnt];\
\
	memset(t, c, cnt);\
	write(1, t, cnt);\
}

static void	print_value(t_value *value) {
	static unsigned	tab_cnt = 0;

	switch (value->type) {
		case object:
			write(1, "{\n", 2);
			++tab_cnt;
			for (unsigned i = 0; i < value->object->size; ++i) {
				PRINT_TMP_BLOCK('\t', tab_cnt);
				write(1, "\"", 1);
				write(1, value->object->member[i].string,
					strlen(value->object->member[i].string));
				write(1, "\" : ", 3);
				print_value(value->object->member[i].value);
				if (i != value->object->size - 1) write(1, ",", 1);
				write(1, "\n", 1);
			}
			--tab_cnt;
			PRINT_TMP_BLOCK('\t', tab_cnt);
			write(1, "}", 1);
			break ;
		case array:
			write(1, "[\n", 1);
			++tab_cnt;
			for (unsigned i = 0; i < value->object->size; ++i) {
				PRINT_TMP_BLOCK('\t', tab_cnt);
				print_value(value->object->member[i].value);
				if (i != value->object->size - 1) write(1, ",", 1);
				write(1, "\n", 1);
			}
			--tab_cnt;
			PRINT_TMP_BLOCK('\t', tab_cnt);
			write(1, "]", 1);
			break ;
		case string:
			write(1, "\"", 1);
			write(1, value->string, strlen(value->string));
			write(1, "\"", 1);
			break ;
		case number:
			PUTNBR(1, value->number.i);
			break ;
		case boolean:
			if (value->boolean) write(1, "true", 4);
			else write(1, "false", 5);
			break ;
		case null_ptr:
			write(1, "null", 4);
	}
	if (!tab_cnt) write(1, "\n", 1);
}

t_value	*ft_json(const char *file_name) {
	t_value	*json_data = NULL;
	t_token	**token = NULL;
	size_t	token_cnt;
	int		fd = open(file_name, O_RDONLY);

	if (fd == -1 || !(token = tokenize_file(fd, &token_cnt))) return (NULL);
	for (size_t i = 0; i < token_cnt; i++)
		write(1, token[i]->data, token[i]->size - 1), write(1, "\n", 1);
	json_data = json_parser(*token, token_cnt);
	for (size_t i = 0; i < token_cnt; ++i) free(token[i]), token[i] = NULL;
	free(token), token = NULL;
	print_value(json_data);
	return (json_data);
}
