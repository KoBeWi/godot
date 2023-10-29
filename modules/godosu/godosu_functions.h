#ifndef GODOSU_FUNCTIONS_H
#define GODOSU_FUNCTIONS_H

VALUE gd_print(VALUE self, VALUE string) {
	String print_string = StringValueCStr(string);
	print_line(print_string);
	return OK;
}

#endif