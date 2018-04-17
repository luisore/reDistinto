/*
 * archivo.h
 *
 *  Created on: 31 mar. 2018
 *      Author: avinocur
 */

#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "commons/collections/list.h"

typedef struct {
	FILE* file;
	char* mode;
	bool open;
} t_textfile;

/*
 * Opens a text file in the selected mode.
 * filename: complete path to the file
 * modE:
 * "r" : read mode.
 * "w" : write mode. Will overwrite if file exists.
 * "a" : append mode. Will create the file if not existing.
 * "r+" : read+write mode. File must exist.
 * "w+" : read+write mode. Will create or overwrite the file.
 */
t_textfile* textfile_open(char *filename, char *mode);

/*
 * Closes the text file. A closed file cannot be reused.
 *
 * Return value zero signals that the file was properly closed.
 * EOF value signals that an error occurred while closing the file.
 */
int textfile_close(t_textfile *file);

bool textfile_end_reached(t_textfile* file);

char* textfile_readline(t_textfile* file, int max_line_length);

void textfile_writeline(t_textfile* file, char* line);

void textfile_destroy(t_textfile* archivo);

bool textfile_execute_by_line(t_textfile* file, int max_line_length, void (* to_exec)(char*));

#endif /* TEXTFILE_H_ */
