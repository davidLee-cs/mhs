
#ifndef LIB_MHSSTD_H_
#define LIB_MHSSTD_H_


/*
 * mystd.h
 *
 *  Created on: 2024. 7. 18.
 *      Author: Derac SON
 */

#include "Typedef.h"
//#include <stdarg.h>
//

#if 0
int16_t my_atoi(const char_t *str);
char_t* my_strtok(char_t* str, const char_t* delimiters);
uint16_t my_strlen(const char_t *str);
int16_t my_strncmp(const char_t *s1, const char_t *s2, uint16_t n);
#endif
int16_t LToStr(int32_t lval, char8_t *str, int16_t str_size);
void StrZeroFill( char8_t *str, uint16_t iLength);

#endif /* LIB_MHSSTD_H_ */
