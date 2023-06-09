#include "applogic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "filemanager.h"
#include "memory.h"
#include "stringworks.h"

FuncReturningValue* getDataFromFile(const char *filename, int column_to_calc, char* region_name);
FuncReturningValue* solve(char ***data, size_t rows_num, size_t fields_num, int column_to_calc);
void clean(FuncArgument* args);

// Определение точки входа
FuncReturningValue* entryPoint(FuncType ft, FuncArgument* fa)
{
    FuncReturningValue* result;
    switch(ft)
    {
        case getData:
            result = getDataFromFile(fa->filename, fa->column_to_calc, fa->region_name);
            break;
        case calculateData:
            result = solve(fa->data, fa->len, fa->fields_num, fa->column_to_calc);
            break;
        case cleanData:
            clean(fa);
        default:
            result = NULL;
            break;
    }

    return result;
}

int findFieldRegion(char **headers, size_t fields)
{
    for(int i=0; i < fields; i++) {
        if (strcmp(headers[i], "region") == 0) {
            return i + 1;
        }
    }

    return -1;
}

int getDataLinesCount(size_t lines, char **rawData, size_t fields, int regionColumnNumber,
                      char* region_name, int column_to_calc, FuncReturningValue *frv)
{
    int dataLines = 0;
    for (size_t i = 0; i < lines - 1; i++)
    {
        char** splittedData = strSplit(rawData[i+1], &fields, ',');
        char* dataRegionName = splittedData[regionColumnNumber - 1];

        if (strcmp(dataRegionName, region_name) != 0)
            continue;

        char* dataCalcValue = splittedData[column_to_calc - 1];

        if (dataCalcValue[0] == '\0') {
            continue;
        }

        double value = atof(dataCalcValue);
        if (value == 0 && dataCalcValue[0] != '0') {
            frv->isNonNumberInCalcField = true;
        }

        dataLines++;
    }

    return dataLines;
}

FuncReturningValue* getEmptyResult(FuncReturningValue* frv)
{
    frv->data_len = 0;
    frv->len = 0;
    frv->fields_num = 0;
    frv->headers = NULL;
    frv->data = NULL;
    return frv;
}

FuncReturningValue* getDataResult(int dataLines, size_t lines, char **rawData,
                                    size_t fields, char* region_name, FuncReturningValue *frv,
                                    char **headers)
{
    char ***data = (char***)malloc(dataLines * sizeof(char**));
    int processedLine = 0;
    for (size_t i = 0; i < lines - 1; i++)
    {
        char** splittedData = strSplit(rawData[i+1], &fields, ',');
        char* dataRegionName = splittedData[1];

        if (strcmp(dataRegionName, region_name) != 0)
            continue;

        data[processedLine] =  splittedData;
        processedLine++;
    }

    clean2DArray(rawData, lines);

    //Заполняем струкутру
    frv->data_len = dataLines;
    frv->len = processedLine;
    frv->fields_num = fields;
    frv->headers = headers;
    frv->data = data;

    return frv;
}

FuncReturningValue* getDataFromFile(const char* filename, int column_to_calc, char* region_name)
{
    FuncReturningValue *frv = (FuncReturningValue *)malloc(sizeof(FuncReturningValue));
    frv->isRegionColumnNotFound = false;
    frv->isNonNumberInCalcField = false;

    FILE* fp = fopen(filename, "r");
    size_t lines, fields;
    char **rawData = readfile(fp, &lines), **headers = strSplit(rawData[0], &fields, ',');
    int regionColumnNumber = findFieldRegion(headers, fields);

    if (regionColumnNumber == -1) {
        frv->isRegionColumnNotFound = true;
        return getEmptyResult(frv);
    }

    frv->column_for_region = regionColumnNumber;

    int dataLines = getDataLinesCount(lines, rawData, fields, regionColumnNumber, region_name, column_to_calc, frv);

    if (dataLines == 0 || frv->isNonNumberInCalcField == true) {
        return getEmptyResult(frv);
    }

    return getDataResult(dataLines, lines, rawData, fields, region_name, frv, headers);
}

int cmpfunc(const void* a, const void* b) {
   if ((*(GraphValue**)a)->value > (*(GraphValue**)b)->value)
       return 1;

   if ((*(GraphValue**)a)->value < (*(GraphValue**)b)->value)
       return -1;

   return 0;
}

GraphValue* getGraphValue(char ***data, int row, int column_to_calc)
{
    char* textValue = data[row][column_to_calc - 1];
    double value = atof(textValue);
    if (value == 0 && textValue[0] != '0')
        return NULL;

    char* yearValue = data[row][0];

    GraphValue *element = (GraphValue *)malloc(sizeof(GraphValue));
    element->value = value;
    element->label = yearValue;

    return element;
}

FuncReturningValue* solveByElements(GraphValue **elementsArray, int idx)
{
    FuncReturningValue *frv = (FuncReturningValue *)malloc(sizeof(FuncReturningValue));
    GraphValue **resultArray = (GraphValue**) malloc(sizeof(GraphValue*) * idx);
    memcpy(resultArray, elementsArray, sizeof(GraphValue*) * idx);
    frv->result_array = resultArray;
    frv->result_rows = idx;

    qsort(elementsArray, idx, sizeof(GraphValue*), cmpfunc);

    frv->result_min = elementsArray[0]->value;
    frv->result_max = elementsArray[idx - 1]->value;

    double median;
    int medianIndex;
    if (idx % 2 == 1) {
        medianIndex = idx / 2;
        median = elementsArray[medianIndex]->value;
    } else {
        medianIndex = idx / 2;
        median = (elementsArray[medianIndex - 1]->value + elementsArray[medianIndex]->value) / 2;
    }

    frv->result_median = median;

    return frv;
}

FuncReturningValue* solve(char ***data, size_t rows_num, size_t fields_num,
                          int column_to_calc)
{
    GraphValue **elementsArray = (GraphValue**) malloc(sizeof(GraphValue*) * rows_num);

    int idx = 0;
    for(int row=0; row < rows_num; row++) {
        GraphValue* element = getGraphValue(data, row, column_to_calc);
        if (element != NULL)
            elementsArray[idx++] = element;
    }

    if (idx > 0) {
        return solveByElements(elementsArray, idx);
    }

    FuncReturningValue *frv = (FuncReturningValue *)malloc(sizeof(FuncReturningValue));
    frv->result_min = 0;
    frv->result_max = 0;
    frv->result_median = 0;
    frv->result_rows = 0;
    return frv;
}

void clean(FuncArgument* args)
{
    if (args->data != NULL)
    {
        clean3DArray(args->data, args->data_len, args->fields_num);
        args->data = NULL;
    }
    if (args->filename != NULL)
    {
        free(args->filename);
        args->filename = NULL;
    }
    if (args->headers != NULL)
    {
        clean2DArray(args->headers, args->fields_num);
        args->headers = NULL;
    }
    if (args->solution != NULL)
    {
        clean2DArray(args->solution, args->fields_num);
        args->solution = NULL;
    }
}


