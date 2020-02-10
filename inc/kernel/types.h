/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * Types
 */

#ifndef __KERNEL_TYPES_H__
#define __KERNEL_TYPES_H__

#if defined(__i386__)

typedef unsigned long size_t;

typedef unsigned long long uint64_t;
typedef signed long long int64_t;

typedef unsigned long uint32_t;
typedef signed long int32_t;

typedef unsigned short uint16_t;
typedef signed short int16_t;

typedef unsigned char uint8_t;
typedef signed char int8_t;
#elif defined(__x86_64__) 

typedef unsigned long size_t;

typedef unsigned long uint64_t;
typedef signed long int64_t;

typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef unsigned short uint16_t;
typedef signed short int16_t;

typedef unsigned char uint8_t;
typedef signed char int8_t;

#endif

#define PAGE_SIZE 4096
#define PAGE_LOG_SIZE 12

#endif

