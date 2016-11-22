#ifndef FUNDATION_dep_h__
#define FUNDATION_dep_h__

#include <stdio.h>
#ifdef LOG_V
#undef LOG_V
#endif
#ifdef LOG_I
#undef LOG_I
#endif
#ifdef LOG_W
#undef LOG_W
#endif
#ifdef LOG_E
#undef LOG_E
#endif
#ifdef LOG_F
#undef LOG_F
#endif
#define LOG_V(fmt,...) do {printf("I:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_I(fmt,...) do {printf("I:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_W(fmt,...) do {printf("W:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_E(fmt,...) do {printf("E:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);
#define LOG_F(fmt,...) do {printf("F:");printf(fmt, ##__VA_ARGS__);printf("\n");} while (0);

#if defined(WIN32) || defined(WIN64)
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif // FUNDATION_dep_h__
