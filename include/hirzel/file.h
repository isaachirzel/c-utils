#ifndef HIRZEL_FILE_H
#define HIRZEL_FILE_H

#include <stdbool.h>

/**
 * @brief Reads a file into a string
 * 
 * The string is dynamically allocated and must be freed
 * 
 * @param	filepath	path to file
 * @param	options		options used in fopen: "r", "rb", etc..
 * @return	pointer to string
 */
extern char *hxfile_read(const char *filepath);
extern char *hxfile_read_raw(const char *filepath);

extern bool hxfile_write(const char *filepath, const char *text);
extern bool hxfile_write_raw(const char *filepath, const char *text);

extern bool hxfile_append(const char *filepath, const char *text);
extern bool hxfile_append_raw(const char *filepath, const char *data);

/**
 * @brief Reads line from file stream till newline
 * 
 * The memory for the stream is dynamically allocated and must be freed
 * 
 * @param	stream	file stream
 * @return	pointer to string
 */
extern char *hxfile_read_line(FILE* stream);

/**
 * @brief Reads lines from file in text mode
 * 
 * The buffer returned must be freed by hxfile_free_lines
 * 
 * @param	filepath	path to file to read
 * @return	buffer of null terminated lines array
 */
extern char **hxfile_read_lines(const char* filepath);

/**
 * @brief Frees lines gotten from hxfile_read_lines()
 * @param	lines	buffer to be freed
 */
extern void hxfile_free_lines(char** lines);

#endif

#ifdef HIRZEL_IMPLEMENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* read(const char* filepath, const char* options)
{
	if (!filepath || !options) return NULL;
	// open file stream
	FILE *stream = fopen(filepath, options);
	if (!stream) return NULL;
	// going to end of stream
	fseek(stream, 0, SEEK_END);
	// getting size of stream
	size_t size = ftell(stream);
	// rewinding stream
	fseek(stream, 0, SEEK_SET);
	// allocating string
	char* str = malloc(size + 1);
	// reading file
	size_t readc = fread(str, sizeof(char), size, stream);
	// null terminating string
	str[readc] = 0;
	// closing file
	fclose(stream);
	return str;
}


char *hxfile_read(const char *filepath)
{
	return read(filepath, "r");
}


char *hxfile_read_raw(const char *filepath)
{
	return read(filepath, "rb");
}


static bool write(const char *filepath, const char *text, const char *options)
{
	if (!filepath || !text || !options) return false;
	// open file stream
	FILE *stream = fopen(filepath, options);
	if (!stream) return false;
	// writing data
	fputs(text, stream);
	// closing file stream
	fclose(stream);
	return true;
}

bool hxfile_write(const char *filepath, const char *text)
{
	return write(filepath, text, "w");
}


bool hxfile_write_raw(const char *filepath, const char *text)
{
	return write(filepath, text, "wb");
}


bool hxfile_append(const char *filepath, const char *text)
{
	return write(filepath, text, "a");
}


bool hxfile_append_raw(const char *filepath, const char *data)
{
	return write(filepath, data, "ab");
}


char **hxfile_read_lines(const char *filepath)
{
	// read file
	char* text = hxfile_read(filepath);
	if (!text) return NULL;

	// iterator
	const char* pos = text;
	size_t linec = 0;

	// iterating to end
	while (*pos)
	{
		if (*pos++ == '\n') ++linec;
	}
	// if last line isn't empty count it as well
	if (*(pos - 1) != '\n' && pos > text) ++linec;

	// creating output buffer with space for null termination
	char **out = (char**)malloc((linec + 1) * sizeof(char*));

	// creating first token
	out[0] = strtok(text, "\n");

	// filling in output buffer
	for (size_t i = 1; i < linec; ++i)
	{
		out[i] = strtok(NULL, "\n");
	}
	// null terminating output
	out[linec] = NULL;

	return out;
}

char *hxfile_read_line(FILE *stream)
{
	if (!stream) return NULL;
	// checking if eof
	char c = getc(stream);
	// returning null on fail
	if (c == EOF) return NULL;
	// putting char back in stream
	ungetc(c, stream);
	// temporary buffer
	char buf[128];
	// size of output buffer
	size_t osize = 0;
	// output buffer pointer
	char *line = NULL;

	do
	{
		// reset flag byte
		buf[126] = 0;
		// read line into buf
		fgets(buf, 128, stream);
		// get len of read info
		size_t len = strlen(buf);
		// expand size of output buffer
		osize += (len + 1) * sizeof(char);
		// allocate space for output buffer
		char* tmp = malloc(osize);
		// if we have data in ouput buffer
		if (line)
		{
			// copy to over to new, bigger buffer
			strcpy(tmp, line);
			free(line);
		}
		else
		{
			// prepare new buffer for concatenation
			tmp[0] = 0;
		}
		// store output buffer in line
		line = tmp;
		// copy temp buffer into output buffer
		strcat(line, buf);
	}
	// continue if data was not a complete line
	while (buf[126] != 0 && buf[126] != '\n');

	// return output buffer
	return line;
}

void hxfile_free_lines(char **lines)
{
	free(*lines);
	free(lines);
}

#endif