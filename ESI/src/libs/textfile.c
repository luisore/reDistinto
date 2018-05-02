/*
 * archivo.c
 *
 *  Created on: 31 mar. 2018
 *      Author: avinocur
 */

#include <commons/string.h>
#include "textfile.h"


t_textfile* textfile_open(char *filename, char *mode){
	FILE* file = fopen(filename, mode);

	t_textfile* archivo = malloc(sizeof(t_textfile));
	archivo->file = file;
	archivo->mode = (char *)string_duplicate(mode);
	archivo->open = file != NULL;

	return archivo;
}

int textfile_close(t_textfile* file){
	int res = fclose(file->file);
	if(res){
		file->open = false;
	}
	return res;
}

bool textfile_is_open_read(t_textfile* file){
	return file->open && (
			string_equals_ignore_case(file->mode, "r") ||
			string_equals_ignore_case(file->mode, "r+") ||
			string_equals_ignore_case(file->mode, "w+")
	);
}

bool textfile_end_reached(t_textfile* file){
	return feof(file->file);
}

char* textfile_readline(t_textfile* file, int max_line_length){
	if(!textfile_is_open_read(file)) return NULL;

	char* line = malloc(sizeof(char) * max_line_length + 1);
	return fgets(line, max_line_length, file->file);
}

void textfile_writeline(t_textfile* file, char* line){
	fputs(line, file->file);
}

bool textfile_execute_by_line(t_textfile* file, int max_line_length, void (* to_exec)(char*)){
	if(!textfile_is_open_read(file)){
		return false;
	}

	while(!textfile_end_reached(file)){
		char* line = textfile_readline(file, max_line_length);
		if(line != NULL){
			to_exec(line);
			free(line);
		}
	}

	return true;
}

void textfile_destroy(t_textfile* file){
	if(file->open){
		textfile_close(file);
	}
	free(file->mode);
	free(file);
}

