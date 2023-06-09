#include "memory.h"

void clean2DArray(char **arr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        free(arr[i]);
    }
    free(arr);
}

void clean3DArray(char ***arr, size_t sizeX, size_t sizeY)
{
    for (size_t i = 0; i < sizeX; i++)
    {
        clean2DArray(arr[i], sizeY);
    }
    free(arr);
}

