#ifndef APPLOGIC_H
#define APPLOGIC_H

#include <stdlib.h>
// Перечисление доступных функций, выделенных в сегрегированный файл логики
enum FuncType
{
    getData,                                    // функция чтения данных из csv файла
    calculateData,                              // функция для вычислений
    cleanData                                   // освобождение памяти
};

/*
 Структура, описывающая возможные варианты передаваемых аргументов для функций (один из вариантов
реализации передачи различных аргументов в точку входа). При необходимости вызова некоторой функции,
соответствующее поле структуры инициализируется некоторым значением.
 */
typedef struct
{
    char* filename;                             // имя файла (функция getData())
    char ***data;                               // данные (функция calculateData())
    char **headers;                             // заголовки таблицы (функция getData())
    char **solution;                            // результат вычислений (для освобождения памяти)
    size_t data_len;                            // число строк в структуре data
    size_t len;                                 // число строк в таблице
    size_t fields_num;                          // число столбцов в таблице
    char* region_name;                          // имя региона
    int  column_for_region;                     // стоблец региона
    int  column_to_calc;                        // столбец для вычисления
} FuncArgument;

typedef struct
{
    double value; // значение
    char*  label;  // подпись
} GraphValue;

/*
 Структура, описывающая возможные варианты возвращемого значения функций <...>
 */
typedef struct
{
    char ***data;                                     // данные (функция getData())
    char **headers;                                   // заголовки таблицы (функция getData())
    GraphValue **result_array;                        // результирующий массив с данными
    double result_min;                                // результат вычислений - минимум
    double result_max;                                // результат вычислений - максимум
    double result_median;                             // результат вычислений - медиана
    double result_rows;                               // результат вычислений - количество обработанных записей
    size_t data_len;                                  // общий размер данных для обработки
    size_t len;                                       // число строк в таблице
    size_t fields_num;                                // число столбцов в таблице

    int  column_for_region;                           // стоблец региона

    bool   isNonNumberInCalcField;                    // есть нечисловые значения в поле для вычислений
    bool   isRegionColumnNotFound;                    // не найден столбец региона
} FuncReturningValue;

// Объявление точки входа
FuncReturningValue* entryPoint(FuncType ft, FuncArgument* fa);

#endif // APPLOGIC_H
