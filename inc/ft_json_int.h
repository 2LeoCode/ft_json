#ifndef FT_JSON_INT_H
# define FT_JSON_INT_H

# include "ft_json.h"

# define PUTNBR(fd, n) _Generic((n),\
	off_t: putnbr_offt,\
	unsigned: putnbr_unsigned\
)((fd), (n))
# define IS_DIGIT(c) (c >= '0' && c <= '9')
# define IS_JSON_WS(c) (c == ' ' || c == '\n' || c == '\r' || c == '\t')

extern const char	*g_special[3];
extern const char	g_single_token[6];

typedef struct	s_token {
	off_t	line;
	size_t	size;
	char	data[];
}	t_token;

int	putnbr_unsigned(int fd, unsigned n);
int	putnbr_offt(int fd, off_t n);
int	putnbr_llong(int fd, long long n);
t_token	**json_tokenizer(char *file_data, off_t file_size, size_t *token_cnt);
t_value	*json_parser(t_token *token, size_t cnt);

#endif //FT_JSON_INT_H
