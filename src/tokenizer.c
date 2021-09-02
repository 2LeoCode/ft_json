#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <ft_json_int.h>

# define TOKEN_LEN(tok) ({\
	size_t	len = 0;\
	size_t	limit = file_size - i;\
\
	if (IS_SINGLE_TOKEN(*(tok))) len = 1;\
	else if (IS_DIGIT(*(tok)) || *(tok) == '.' || *(tok) == '-') {\
		if (*(tok) == '-') ++len;\
		while (len < limit && IS_DIGIT((tok)[len])) ++len;\
		if (len != limit && (tok)[len] == '.') ++len;\
		while (len < limit && IS_DIGIT((tok)[len])) ++len;\
		if (len != limit && ((tok)[len] == 'e' || (tok)[len] == 'E')\
		&& ((tok)[++len] == '+' || (tok)[len] == '-')) ++len;\
		while (len < limit && IS_DIGIT((tok)[len])) ++len;\
	}\
	else if (*(tok) == '\"') {\
		while (++len < limit && (tok)[len] != '\"') {\
			if ((tok)[len] == '\\' && ++len != limit && (tok)[len] == 'u')\
				len += 4;\
		}\
		++len;\
	}\
	else if (!TOKEN_CMP("true", tok) || !TOKEN_CMP("null", tok)) len = 4;\
	else if (!TOKEN_CMP("false", tok)) len = 5;\
	len;\
})

# define GET_TOKEN(str) ({\
	const size_t	len = TOKEN_LEN(str);\
	t_token			*tok = malloc(sizeof(t_token) + (len + 1));\
\
	if (!tok) goto error;\
	tok->size = len + 1;\
	{\
		void *dummy = memcpy(tok->data, str, len);\
		(void)dummy;\
	}\
	tok->data[len] = 0;__GET_TOKEN_LINE__
#define __GET_TOKEN_LINE__(line) \
	tok->line = line;\
	error: tok;\
})

# define TOKEN_CMP(ref, token) ({\
	size_t	len = strlen(ref);\
	bool	ret = false;\
\
	if ((size_t)(file_size - i) < len) ret = true;\
	else {\
		for (size_t k = 0; k < len; ++k) if ((token)[k] != (ref)[k]) ret = true;\
		if (len < (size_t)(file_size - i) && !IS_JSON_WS((token)[len])\
		&& !IS_SINGLE_TOKEN((token)[len])) ret = true;\
	}\
	ret;\
})

# define INVAL_TOKEN_LEN(str) ({\
	size_t	len = 0;\
	size_t	limit = file_size - i;\
\
	while (len < limit && !IS_JSON_WS((str)[len])) len++;\
	len;\
})

# define PRINT_INVALID_TOKEN(str); write(2, str, INVAL_TOKEN_LEN(str));

# define UNEXPECTED_TOKEN(str) {\
	write(2, "Unexpected token `", 18);\
	PRINT_INVALID_TOKEN(str);__UNEXPECTED_TOKEN_LINE__
# define __UNEXPECTED_TOKEN_LINE__(line); \
	write(2, "\'\nat line ", 10);\
	PUTNBR(2, line);\
	write(2, "\n", 1);\
}

# define IS_SINGLE_TOKEN(c) ({\
	bool	ret = false;\
\
	for (int k = 0; k < 6; k++) if (c == g_single_token[k]) ret = true;\
	ret;\
})

# define SKIP_TOKEN(str); {\
	if ((str)[i] == 't' || (str)[i] == 'n') i += 4;\
	else if ((str)[i] == 'f') i += 5;\
	else if (IS_DIGIT((str)[i]) || (str)[i] == '.' || (str)[i] == '-') {\
		if ((str)[i] == '-') ++i;\
		while (i < file_size && IS_DIGIT((str)[i])) ++i;\
		if (i < file_size && (str)[i] == '.') ++i;\
		while (i < file_size && IS_DIGIT((str)[i])) ++i;\
		if (i < file_size && ((str)[i] == 'e' || (str)[i] == 'E')\
		&& ((str)[++i] == '-' || (str)[i] == '+')) ++i;\
		while (i < file_size && IS_DIGIT((str)[i])) ++i;\
	}\
	else if ((str)[i] == '\"') {\
		while (++i < file_size && (str)[i] != '\"')\
			if ((str)[i] == '\\' && ++i != file_size && (str)[i] == 'u') i += 4;\
		++i;\
	}\
	else if (IS_SINGLE_TOKEN((str)[i])) ++i;\
}

# define IS_JSON_NUMBER(str) ({\
	bool			ret = true;\
	unsigned		j = 0;\
	const unsigned	limit = file_size - i;\
\
	if (!IS_DIGIT(*(str)) && *(str) != '.' && *(str) != '-') ret = false;\
	else {\
		if (*(str) == '-') ++j;\
		while (j < limit && IS_DIGIT((str)[j])) ++j;\
		if (j < limit && (str)[j] == '.') ++j;\
		while (j < limit && IS_DIGIT((str)[j])) ++j;\
		if (j < limit && ((str)[j] == 'e' || (str)[j] == 'E')) {\
			if (++j < limit && ((str)[j] == '+' || (str)[j] == '-')) ++j;\
			if (j == limit || !IS_DIGIT((str)[j])) ret = false;\
		}\
		while (j < limit && IS_DIGIT((str)[j])) ++j;\
		if (j < limit && !IS_JSON_WS((str)[j]) && !IS_SINGLE_TOKEN((str)[j])) ret = false;\
	}\
	ret;\
})

# define IS_JSON_STRING(str) ({\
	bool			ret = true;\
	unsigned		j = 0;\
	const unsigned	limit = file_size - i;\
\
	if ((str)[j] != '\"') ret = false;\
	else {\
		while (++j < limit && (str)[j] != '\"')\
			if ((str)[j] == '\\') {\
				if (++j == limit || !strchr("\"\\/bfnrtu", (str)[j])) ret = false;\
				if ((str)[j] == 'u') j += 4;\
			}\
		if (j >= limit) ret = false;\
	}\
	ret;\
})

# define IS_JSON_SPECIAL(str) ({\
	bool	ret = false;\
\
	for (int l = 0; l < 3; ++l) if (!TOKEN_CMP(g_special[l], str)) ret = true;\
	ret;\
})

# define IS_VALID_TOKEN(str) (IS_SINGLE_TOKEN(*(str)) || IS_JSON_STRING((str))\
|| IS_JSON_NUMBER((str)) || IS_JSON_SPECIAL((str)))

#define FAIL_IF(expr); {\
	if (expr) {\
		UNEXPECTED_TOKEN(file_data + i)(line);\
		return (NULL);\
	}\
}

static void	fill_tokens(t_token ***token_addr, char *file_data, off_t file_size) {
	off_t		line = 1;
	unsigned	i = 0;
	t_token		**it = *token_addr;

	while (42) {
		while (i < file_size && IS_JSON_WS(file_data[i]))
			if (file_data[i++] == '\n') ++line;
		if (i == file_size) break ;
		if (!(*it++ = GET_TOKEN(file_data + i)(line))) {
			for (t_token **it1 = *token_addr; *it1; ++it1)
				free(*it1), *it1 = NULL;
			free(*token_addr), *token_addr = NULL;
			break ;
		}
		SKIP_TOKEN(file_data);
	}
}

t_token	**json_tokenizer(char *file_data, off_t file_size, size_t *token_cnt) {
	off_t		line = 1;
	unsigned	i = 0;
	t_token		**token;

	*token_cnt = 0;
	while (42) {
		while (i < file_size && IS_JSON_WS(file_data[i]))
			if (file_data[i++] == '\n') ++line;
		if (i == file_size) break ;
		FAIL_IF(!IS_VALID_TOKEN(file_data + i));
		++*token_cnt;
		SKIP_TOKEN(file_data);
	}
	if (!(token = malloc(sizeof(t_token *) * *token_cnt))) return (NULL);
	fill_tokens(&token, file_data, file_size);
	return (token);
}
