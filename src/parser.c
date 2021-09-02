#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <ft_json_int.h>

#define INVALID_SYNTAX ((size_t)-1)

#define EXPECTED_TOKEN(str) ({\
	write(2, "Expected token `", 16);\
	write(2, str, strlen(str));\
	__TOKEN_
#define UNEXPECTED_TOKEN(str) ({\
	write(2, "Unexpected token `", 16);\
	write(2, str, strlen(str));\
	__TOKEN_
#define __TOKEN_AFTER(tok) \
	write(2, "\' after `", 9);\
	write(2, tok->data, tok->size - 1);\
	write(2, "\'\n", 2);\
	write(2, "at line ", 8);\
	PUTNBR(2, tok->line);\
	INVALID_SYNTAX;\
})
#define __TOKEN_BEFORE(tok) \
	write(2, "\' before `", 9);\
	write(2, tok->data, tok->size - 1);\
	write(2, "\'\n", 2);\
	write(2, "at line ", 8);\
	PUTNBR(2, tok->line);\
	INVALID_SYNTAX;\
})

static size_t	check_object_size(t_token *token, size_t cnt) {
	int				array_cnt = 0;
	int				object_cnt = 0;
	size_t			size = 0;
	bool			done = false;
	t_value_type	last = -1;

	if (cnt == 1) return (EXPECTED_TOKEN("}")AFTER(token));
	++token, ++cnt;
	while (cnt >= 4 && !done) {
		if (*token->data != '\"')
			return (EXPECTED_TOKEN("<string>")AFTER(token - 1));
		++size, ++token, --cnt;
		if (*token->data != ':')
			return (EXPECTED_TOKEN(":")AFTER(token - 1));
		++token, --cnt;
		switch (*token->data) {
			case '[': case '{':
				array_cnt = (*token->data == '[');
				object_cnt = !array_cnt;
				last = array_cnt * array;
				while (array_cnt || object_cnt) {
					--token;
					if (!--cnt) return (EXPECTED_TOKEN("]")AFTER(token - 1))
					if (*token->data == '{') ++object_cnt, last = object;
					else if (*token->data == '}') {
						if (last == array) return (EXPECTED_TOKEN("]")
								BEFORE(token));
						--object_cnt;
					}
					else if (*token->data == '[') ++array_cnt, last = array;
					else if (*token->data == ']') {
						if (last == object) return (EXPECTED_TOKEN("}")
								BEFORE(token));
						--array_cnt;
					}
				}
				break ;
			case '}': case ',':
				return (EXPECTED_TOKEN("<value>")BEFORE(token));
			case ']':
				return (UNEXPECTED_TOKEN("]")AFTER(token - 1));
		}
		++token;
		if (!--cnt) return (EXPECTED_TOKEN("}")AFTER(token - 1));
		if (*token->data == ',') ++token, --cnt;
		else if (*token->data == '}') done = true;
	}
	if (!done) return (EXPECTED_TOKEN(
			*(token - 1)->data == ',' ? "<value>" : "}")AFTER(token - 1));
	return (size);
}

static int	parse_object(t_value *value, t_token **token_p, size_t *cnt_p) {
	const size_t	size = check_object_size(*token_p, *cnt_p);
	size_t			i = 0;
	
	if (size == INVALID_SYNTAX
	|| !(value->object = malloc(sizeof(t_object) + sizeof(t_member) * size)))
		return (-1);
	value->type = object;
	value->size = size;
	for (unsigned i = 0; i < size; ++i) {
		value->object->member[i].string = NULL;
		value->object->member[i].value = NULL;
	}
	++*token_p, --*cnt_p;
	while (--*cnt_p && *((*token_p)++)->data != '}') {
		value->object->member[i].string =
			strndup((*token_p)->data + 1, (*token_p)->size - 3);
		*token_p += 2, *cnt_p -= 2;
		if (!value->object->member[i].string) return (-1);
		if (!(value->object->member[i].value = parse_value(token_p, cnt_p))) {
			free(value->object->member[i].string);
			value->object->member[i].string = NULL;
			return (-1);
		}
		--*cnt_p, --*token_p;
	}
	return (0);
}

static size_t	check_array_size(t_token *token, size_t cnt) {
	int				array_cnt = 1;
	int				object_cnt = 0;
	size_t			size = 0;
	t_value_type	last = -1;

	if (cnt == 1) return (EXPECTED_TOKEN("]")AFTER(token));
	++token, --cnt;
	while (cnt && (array_cnt || object_cnt)) {
		switch (*token->data) {
			case '[':
				size += (!object_cnt && array_cnt == 1);
				++array_cnt, last = array;
				break ;
			case '{':
				size += (!object_cnt && array_cnt == 1);
				++object_cnt, last = object;
				break ;
			case '}':
				if (last == array || --object_cnt < 0)
					return (UNEXPECTED_TOKEN("}")AFTER(token - 1));
				break ;
			case ',':
				if (object_cnt || array_cnt > 1)
					break ;
				if (*(token - 1)->data == ',' || *(token - 1)->data == '[')
					return (UNEXPECTED_TOKEN(",")AFTER(token - 1));
			case ']':
				if (last == object)
					return (UNEXPECTED_TOKEN("]")AFTER(token - 1));
				--array_cnt;
				break ;
			default:
				if (object_cnt || array_cnt > 1) break ;
				if (*(token - 1)->data == ',' || *(token - 1)->data == '[')
					++size;
				else return (UNEXPECTED_TOKEN(token->data)
					AFTER(token - 1));
		}
		++token, --cnt;
	}
	if (array_cnt || object_cnt)
		return (EXPECTED_TOKEN(last == array ? "]" : "}")AFTER(token - 1));
	else if (*(token - 1)->data != "]")
		return (EXPECTED_TOKEN(*(token - 1)->data == ',' ? "<value>" : "]")
			AFTER(token - 1));
	return (size);
}

static t_array	*alloc_array(size_t size) {
	t_array	*ptr = malloc(sizeof(t_array) + sizeof(t_value *) * size);

	if (ptr)
		for (unsigned i = 0; i < size; ++i)
			if (!(ptr->value[i] = malloc(sizeof(t_value)))) {
				while (--i >= 0) free(ptr->value[i]), ptr->value[i] = NULL;
				free(ptr), ptr = NULL;
				break ;
			}
	return (ptr);
}

static int	parse_array(t_value *value, t_token **token_p, size_t *cnt_p) {
	const size_t	size = check_array_size(*token_p, *cnt_p);
	size_t			i = 0;

	if (size == INVALID_SYNTAX || !(value->array = alloc_array(size)))
		return (-1);
	value->type = array;
	value->array->size = size;
	while (--*cnt_p && *((*token_p)++)->data != ']') {
		if (!(value->array->value[i] = parse_value(token_p, cnt_p)))
			return (-1);
		++i;
	}
	return (0);
}

static int	parse_string(t_value *value, t_token **token_p, size_t *cnt_p) {
	value->type = string;
	if (!(value->string = strndup((*token_p)->data + 1, (*token_p)->size - 3)))
		return (-1);
	++*token_p, --*cnt_p;
	return (0);
}

static void	parse_number(t_value *value, t_token **token_p, size_t *cnt_p) {
	unsigned i = 0;

	value->type = number;
	if ((*token_p)->data[i] == '-') ++i;
	while (IS_DIGIT((*token_p)->data[i])) ++i;
	if ((*token_p)->data[i] == '.' || (*token_p)->data[i] == 'e'
	|| (*token_p)->data[i] == 'E')
		value->number.type = Float, value->number.f = atof((*token_p)->data);
	else value->number.type = Int, value->number.i = atoi((*token_p)->data);
	++*token_p, --*cnt_p;
}

static void	parse_special(t_value *value, t_token **token_p, size_t *cnt_p) {
	if (!strcmp(*token_p, "null")) {
		value->type = null_ptr;
		value->null_ptr = NULL;
	}
	else {
		value->type = boolean;
		value->boolean = !ft_strcmp(*token_p, "false");
	}
	++*token_p, --*cnt_p;
}

#define FAIL_IF(expr); if (expr) destroy_value(value), value = NULL;

static t_value	*parse_value(t_token **token_p, size_t *cnt_p) {
	t_value	*value = malloc(sizeof(t_value));

	if (!value) return (NULL);
	switch (**token_p) {
		case '{':
			FAIL_IF(parse_object(value, token_p, cnt_p));
			break ;
		case '[':
			FAIL_IF(parse_array(value, token_p, cnt_p));
			break ;
		case '\"':
			FAIL_IF(parse_string(value, token_p, cnt_p));
			break ;
		default:
			if (IS_DIGIT(**token_p)) parse_number(value, token_p, cnt_p);
			else if (!strcmp((*token_p)->data, "null")
			|| !strcmp((*token_p)->data, "true")
			|| !strcmp((*token_p)->data, "false"))
				parse_special(value, token_p, cnt_p);
	}
	return (value);
}

t_value	*json_parser(t_token *token, size_t cnt) {
	t_value	*value = parse_value(&token, &cnt));

	if (cnt) destroy_value(value), value = NULL;
	return (value);
}
