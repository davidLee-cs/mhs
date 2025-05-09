#ifndef TYPEDEF_H_
#define TYPEDEFH_



#include <stdbool.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE (0x01)
#endif

#ifndef FALSE
#define FALSE (0x00)
#endif

typedef float         float32_t;
typedef long double   float64_t;

typedef char          char8_t;
typedef unsigned char uchar8_t;

typedef          int   int16_t;
typedef unsigned int   uint16_t;
typedef          long  int32_t;
typedef unsigned long  uint32_t;
typedef signed long long  int64_t;
typedef unsigned long long  uint64_t;

typedef bool bool_t;





#endif
