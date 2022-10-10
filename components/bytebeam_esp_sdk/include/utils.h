#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

extern char *utils_read_file(char *filename)
{
    FILE *file;

    file = fopen(filename, "r");

    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // dynamically allocate a char array to store the file contents
    char *buff = malloc(sizeof(char) * (file_length + 1));

    int temp_c;
    int loop_var = 0;

    while ((temp_c = fgetc(file)) != EOF)
    {
        buff[loop_var] = temp_c;
        loop_var++;
    }

    buff[loop_var] = '\0';

    fclose(file);

    return buff;
}

#endif