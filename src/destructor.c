#include <ft_json_int.h>
#include <stdlib.h>

static void	destroy_array(t_array *arr) {
	while (--arr->size) {
		destroy_value(arr->value[arr->size]);
		arr->value[arr->size] = NULL;
	}
	free(arr);
}

static void	destroy_object(t_object *obj) {
	while (--obj->size) {
		free(obj->member[obj->size].string);
		obj->member[obj->size].string = NULL;
		destroy_value(obj->member[obj->size].value);
		obj->member[obj->size].value = NULL;
	}
	free(obj);
}

void	destroy_value(t_value *value) {
	switch (value->type) {
		case object:
			destroy_object(value->object);
			break ;
		case array:
			destroy_array(value->array);
			break ;
		case string:
			free(value->string), value->string = NULL;
		default: NULL;
	}
	free(value);
}
