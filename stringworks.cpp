#include "stringworks.h"
#include <string.h>
#include <stdlib.h>
//Функция разбития строки на массив слов
char** strSplit(char* str, size_t *len, char c) {
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p, *t;
    p = str;
    while (*p != '\0' && *p != '\n') {
        if (*p == c)
            count++;
        p++;
    }
    char **arr = (char**) malloc(sizeof(char*) * count);
    if (arr == NULL)
        exit(1);
    p = str;
    while (*p != '\0' && *p != '\n'){
        if (*p == c){
            *(arr + i) = (char*) malloc( sizeof(char) * token_len );
            if (*(arr +i) == NULL)
                exit(1);
            token_len = 1;
            i++;
        }
        p++;
        token_len++;
    }
    *(arr +i) = (char*) malloc( sizeof(char) * token_len );
    if (*(arr +i) == NULL)
        exit(1);
    i = 0;
    p = str;
    t = *(arr +i);
    while (*p != '\0' && *p != '\n'){
        if (*p != c && *p != '\0' && *p != '\n'){
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = *(arr +i);
        }
        p++;
    }
    *t = '\0';
    *len = count;
    return arr;
}
