/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Numeric to alpha conversions
 */

#ifndef __LOADER_UTL_CONV_H__
#define __LOADER_UTL_CONV_H__

#include "loader/types.h"

char* conv_decimal(uint32_t val,const char* buffer,size_t len,size_t* plen);
char* conv_hexa(uint32_t val,const char* buffer,size_t len,size_t* plen);

const char* conv_int8_d(int8_t val,const char* buffer,size_t len);
const char* conv_int16_d(int16_t val,const char* buffer,size_t len);
const char* conv_int32_d(int32_t val,const char* buffer,size_t len);
const char* conv_uint8_d(uint8_t val,const char* buffer,size_t len);
const char* conv_uint16_d(uint16_t val,const char* buffer,size_t len);
const char* conv_uint32_d(uint32_t val,const char* buffer,size_t len);

const char* conv_int8_h(int8_t val,const char* buffer,size_t len);
const char* conv_int16_h(int16_t val,const char* buffer,size_t len);
const char* conv_int32_h(int32_t val,const char* buffer,size_t len);
const char* conv_uint8_h(uint8_t val,const char* buffer,size_t len);
const char* conv_uint16_h(uint16_t val,const char* buffer,size_t len);
const char* conv_uint32_h(uint32_t val,const char* buffer,size_t len);

const char* conv_uuid(uint8_t* val,const char* buffer,size_t len);

#endif

