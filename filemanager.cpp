#include "filemanager.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Функция чтения строк из файла
char** readfile(FILE* fp, size_t *lines)
{
    char line[BUFF_SIZE];
    size_t llen;
    size_t counter = 0;
    size_t max_size = 1;
    char **data = (char **)calloc(max_size, sizeof(char *));
    if (data != NULL) {
        while (fgets(line, BUFF_SIZE,fp)) {
            if (counter >= max_size-1)
            {
                max_size *= 2;
                data = (char **)realloc(data,max_size * sizeof(char *));
                if (data == NULL) {
                    counter = 0;
                    break;
                }

            }
            llen = strlen(line);
            data[counter] = (char *)calloc(sizeof(char), llen+1);
            if (data[counter] == NULL){
                counter --;
                break;
            }
            strcpy(data[counter], line);
            counter++;
        }
    }
    *lines = counter;
    return data;
}

