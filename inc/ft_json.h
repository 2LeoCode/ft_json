#ifndef FT_JSON_H
# define FT_JSON_H

# include <stdint.h>
# include <stdbool.h>
# include <sys/types.h>

typedef enum e_value_type	t_value_type;
typedef enum e_number_type	t_number_type;
typedef struct s_value		t_value;
typedef struct s_member		t_member;
typedef struct s_object		t_object;
typedef struct s_number		t_number;
typedef struct s_array		t_array;

enum e_value_type {
	object,
	array,
	string,
	number,
	boolean,
	null_ptr
};

enum e_number_type {
	Int,
	Float
};

struct s_number {
	t_number_type	type;
	union {
		long	i;
		double	f;
	};
};

struct s_member {
	char	*string;
	t_value	*value;
};

struct s_object {
	size_t		size;
	t_member	member[];
};

struct s_array {
	size_t	size;
	t_value	*value[];
};

struct s_value {
	t_value_type	type;
	union {
		t_object		*object;
		t_array			*array;
		t_number		number;
		char			*string;
		bool			boolean;
		void			*null_ptr;
	};
};

t_value	*ft_json(const char *file_name);							
void	destroy_value(t_value *value);

#endif //FT_JSON_H
