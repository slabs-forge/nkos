/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * String functions
 */

#ifndef __LOADER_UTL_STRING_H__
#define __LOADER_UTL_STRING_H__

#include "loader/types.h"

char* strcpy(char* dest,const char* src);
size_t strlen(const char* s);
char* strchr(const char*s, int c);
char* strdup(const char*s);
char* strcat(char* dest, const char* src);

int strcmp(const char* s1,const char* s2);
int strncmp(const char* s1,const char* s2,size_t n);

void* memset(void*, int c, size_t n);
void* memmove(void* dest, const void *src, size_t n);

#endif

