#ifndef _BAROMETRIC_H
#define _BAROMETRIC_H

typedef unsigned char       et_uchar;
typedef unsigned char		et_uint8;
typedef char                et_char;
typedef char                et_int8;

typedef unsigned short		et_ushort;
typedef unsigned short		et_uint16;
typedef short               et_short;
typedef short               et_int16;

typedef unsigned int		et_uint;
typedef unsigned int		et_uint32;
typedef int                 et_int;
typedef int                 et_int32;

typedef unsigned long		et_ulong32;
typedef long                et_long32;

typedef unsigned long long	et_uint64;
typedef long long           et_int64;

typedef float               et_float;
typedef double              et_double;

typedef et_uint32           et_size_t;

#define OSS 0

et_long32 read_pressure(void);
et_long32 read_temp(void);
et_long32 calculate(void);

#endif
