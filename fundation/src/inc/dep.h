#ifndef FUNDATION_dep_h__
#define FUNDATION_dep_h__

#include <stdio.h>
#define LOG_V(fmt,...) do {printf("I:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_I(fmt,...) do {printf("I:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_W(fmt,...) do {printf("W:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_E(fmt,...) do {printf("E:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_F(fmt,...) do {printf("F:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);

#endif // FUNDATION_dep_h__
