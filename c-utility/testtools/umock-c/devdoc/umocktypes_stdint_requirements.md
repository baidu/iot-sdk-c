
# umocktypes_stdint requirements

# Overview

umocktypes_stdint is a module that exposes out of the box functionality for types defined in stdint in C99.
Currently the supported types are:
- integer types having certain exact widths

# Exposed API

```c
extern int umocktypes_c_register_types(void);

#define UMOCKTYPES_STDINT_HANDLERS(type, function_postfix) \
    extern char* C2(umocktypes_stringify_,function_postfix)(const type* value); \
    extern int C2(umocktypes_are_equal_, function_postfix)(const type* left, const type* right); \
    extern int C2(umocktypes_copy_, function_postfix)(type* destination, const type* source); \
    extern void C2(umocktypes_free_, function_postfix)(type* value);

UMOCKTYPES_STDINT_HANDLERS(uint8_t, uint8_t)
UMOCKTYPES_STDINT_HANDLERS(int8_t, int8_t)
UMOCKTYPES_STDINT_HANDLERS(uint16_t, uint16_t)
UMOCKTYPES_STDINT_HANDLERS(int16_t, int16_t)
UMOCKTYPES_STDINT_HANDLERS(uint32_t, uint32_t)
UMOCKTYPES_STDINT_HANDLERS(int32_t, int32_t)
UMOCKTYPES_STDINT_HANDLERS(uint64_t, uint64_t)
UMOCKTYPES_STDINT_HANDLERS(int64_t, int64_t)
UMOCKTYPES_STDINT_HANDLERS(uint_fast8_t, uint_fast8_t)
UMOCKTYPES_STDINT_HANDLERS(int_fast8_t, int_fast8_t)
UMOCKTYPES_STDINT_HANDLERS(uint_fast16_t, uint_fast16_t)
UMOCKTYPES_STDINT_HANDLERS(int_fast16_t, int_fast16_t)
UMOCKTYPES_STDINT_HANDLERS(uint_fast32_t, uint_fast32_t)
UMOCKTYPES_STDINT_HANDLERS(int_fast32_t, int_fast32_t)
UMOCKTYPES_STDINT_HANDLERS(uint_fast64_t, uint_fast64_t)
UMOCKTYPES_STDINT_HANDLERS(int_fast64_t, int_fast64_t)

```

## umocktypes_stdint_register_types

```c
extern int umocktypes_stdint_register_types(void);
```

**SRS_UMOCKTYPES_STDINT_01_001: [** umocktypes_stdint_register_types shall register support for all the types in the module. **]**

**SRS_UMOCKTYPES_STDINT_01_002: [** On success, umocktypes_stdint_register_types shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_003: [** If registering any of the types fails, umocktypes_stdint_register_types shall fail and return a non-zero value. **]**

## umocktypes_stringify_uint8_t

```c
extern char* umocktypes_stringify_uint8_t(const uint8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_004: [** umocktypes_stringify_uint8_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_005: [** If value is NULL, umocktypes_stringify_uint8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_006: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_007: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint8_t shall return NULL. **]**

## umocktypes_are_equal_uint8_t

```c
extern int umocktypes_are_equal_uint8_t(const uint8_t* left, const uint8_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_008: [** umocktypes_are_equal_uint8_t shall compare the 2 uint8_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_009: [** If any of the arguments is NULL, umocktypes_are_equal_uint8_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_010: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint8_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_011: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint8_t shall return 0. **]**

## umocktypes_copy_uint8_t

```c
extern int umocktypes_copy_uint8_t(uint8_t* destination, const uint8_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_012: [** umocktypes_copy_uint8_t shall copy the uint8_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_013: [** On success umocktypes_copy_uint8_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_014: [** If source or destination are NULL, umocktypes_copy_uint8_t shall return a non-zero value. **]**

## umocktypes_free_uint8_t

```c
extern void umocktypes_free_uint8_t(uint8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_015: [** umocktypes_free_uint8_t shall do nothing. **]**

## umocktypes_stringify_int8_t

```c
extern char* umocktypes_stringify_int8_t(const int8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_016: [** umocktypes_stringify_int8_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_017: [** If value is NULL, umocktypes_stringify_int8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_018: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_019: [** If any other error occurs when creating the string representation, umocktypes_stringify_int8_t shall return NULL. **]**

## umocktypes_are_equal_int8_t

```c
extern int umocktypes_are_equal_int8_t(const int8_t* left, const int8_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_020: [** umocktypes_are_equal_int8_t shall compare the 2 int8_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_021: [** If any of the arguments is NULL, umocktypes_are_equal_int8_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_022: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int8_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_023: [** If the values pointed to by left and right are different, umocktypes_are_equal_int8_t shall return 0. **]**

## umocktypes_copy_int8_t

```c
extern int umocktypes_copy_int8_t(int8_t* destination, const int8_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_024: [** umocktypes_copy_int8_t shall copy the int8_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_025: [** On success umocktypes_copy_int8_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_026: [** If source or destination are NULL, umocktypes_copy_int8_t shall return a non-zero value. **]**

## umocktypes_free_int8_t

```c
extern void umocktypes_free_int8_t(int8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_027: [** umocktypes_free_int8_t shall do nothing. **]**

## umocktypes_stringify_uint16_t

```c
extern char* umocktypes_stringify_uint16_t(const uint16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_028: [** umocktypes_stringify_uint16_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_029: [** If value is NULL, umocktypes_stringify_uint16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_030: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_031: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint16_t shall return NULL. **]**

## umocktypes_are_equal_uint16_t

```c
extern int umocktypes_are_equal_uint16_t(const uint16_t* left, const uint16_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_032: [** umocktypes_are_equal_uint16_t shall compare the 2 uint16_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_033: [** If any of the arguments is NULL, umocktypes_are_equal_uint16_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_034: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint16_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_035: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint16_t shall return 0. **]**

## umocktypes_copy_uint16_t

```c
extern int umocktypes_copy_uint16_t(uint16_t* destination, const uint16_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_036: [** umocktypes_copy_uint16_t shall copy the uint16_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_037: [** On success umocktypes_copy_uint16_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_038: [** If source or destination are NULL, umocktypes_copy_uint16_t shall return a non-zero value. **]**

## umocktypes_free_uint16_t

```c
extern void umocktypes_free_uint16_t(uint16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_039: [** umocktypes_free_uint16_t shall do nothing. **]**

## umocktypes_stringify_int16_t

```c
extern char* umocktypes_stringify_int16_t(const int16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_040: [** umocktypes_stringify_int16_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_041: [** If value is NULL, umocktypes_stringify_int16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_042: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_043: [** If any other error occurs when creating the string representation, umocktypes_stringify_int16_t shall return NULL. **]**

## umocktypes_are_equal_int16_t

```c
extern int umocktypes_are_equal_int16_t(const int16_t* left, const int16_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_044: [** umocktypes_are_equal_int16_t shall compare the 2 int16_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_045: [** If any of the arguments is NULL, umocktypes_are_equal_int16_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_046: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int16_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_047: [** If the values pointed to by left and right are different, umocktypes_are_equal_int16_t shall return 0. **]**

## umocktypes_copy_int16_t

```c
extern int umocktypes_copy_int16_t(int16_t* destination, const int16_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_048: [** umocktypes_copy_int16_t shall copy the int16_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_049: [** On success umocktypes_copy_int16_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_050: [** If source or destination are NULL, umocktypes_copy_int16_t shall return a non-zero value. **]**

## umocktypes_free_int16_t

```c
extern void umocktypes_free_int16_t(int16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_051: [** umocktypes_free_int16_t shall do nothing. **]**

## umocktypes_stringify_uint32_t

```c
extern char* umocktypes_stringify_uint32_t(const uint32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_052: [** umocktypes_stringify_uint32_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_053: [** If value is NULL, umocktypes_stringify_uint32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_054: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_055: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint32_t shall return NULL. **]**

## umocktypes_are_equal_uint32_t

```c
extern int umocktypes_are_equal_uint32_t(const uint32_t* left, const uint32_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_056: [** umocktypes_are_equal_uint32_t shall compare the 2 uint32_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_057: [** If any of the arguments is NULL, umocktypes_are_equal_uint32_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_058: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint32_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_059: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint32_t shall return 0. **]**

## umocktypes_copy_uint32_t

```c
extern int umocktypes_copy_uint32_t(uint32_t* destination, const uint32_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_060: [** umocktypes_copy_uint32_t shall copy the uint32_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_061: [** On success umocktypes_copy_uint32_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_062: [** If source or destination are NULL, umocktypes_copy_uint32_t shall return a non-zero value. **]**

## umocktypes_free_uint32_t

```c
extern void umocktypes_free_uint32_t(uint32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_063: [** umocktypes_free_uint32_t shall do nothing. **]**

## umocktypes_stringify_int32_t

```c
extern char* umocktypes_stringify_int32_t(const int32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_064: [** umocktypes_stringify_int32_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_065: [** If value is NULL, umocktypes_stringify_int32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_066: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_067: [** If any other error occurs when creating the string representation, umocktypes_stringify_int32_t shall return NULL. **]**

## umocktypes_are_equal_int32_t

```c
extern int umocktypes_are_equal_int32_t(const int32_t* left, const int32_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_068: [** umocktypes_are_equal_int32_t shall compare the 2 int32_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_069: [** If any of the arguments is NULL, umocktypes_are_equal_int32_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_070: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int32_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_071: [** If the values pointed to by left and right are different, umocktypes_are_equal_int32_t shall return 0. **]**

## umocktypes_copy_int32_t

```c
extern int umocktypes_copy_int32_t(int32_t* destination, const int32_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_072: [** umocktypes_copy_int32_t shall copy the int32_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_073: [** On success umocktypes_copy_int32_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_074: [** If source or destination are NULL, umocktypes_copy_int32_t shall return a non-zero value. **]**

## umocktypes_free_int32_t

```c
extern void umocktypes_free_int32_t(int32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_075: [** umocktypes_free_int32_t shall do nothing. **]**

## umocktypes_stringify_uint64_t

```c
extern char* umocktypes_stringify_uint64_t(const uint64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_076: [** umocktypes_stringify_uint64_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_077: [** If value is NULL, umocktypes_stringify_uint64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_078: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_079: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint64_t shall return NULL. **]**

## umocktypes_are_equal_uint64_t

```c
extern int umocktypes_are_equal_uint64_t(const uint64_t* left, const uint64_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_080: [** umocktypes_are_equal_uint64_t shall compare the 2 uint64_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_081: [** If any of the arguments is NULL, umocktypes_are_equal_uint64_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_082: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint64_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_083: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint64_t shall return 0. **]**

## umocktypes_copy_uint64_t

```c
extern int umocktypes_copy_uint64_t(uint64_t* destination, const uint64_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_084: [** umocktypes_copy_uint64_t shall copy the uint64_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_085: [** On success umocktypes_copy_uint64_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_086: [** If source or destination are NULL, umocktypes_copy_uint64_t shall return a non-zero value. **]**

## umocktypes_free_uint64_t

```c
extern void umocktypes_free_uint64_t(uint64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_087: [** umocktypes_free_uint64_t shall do nothing. **]**

## umocktypes_stringify_int64_t

```c
extern char* umocktypes_stringify_int64_t(const int64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_088: [** umocktypes_stringify_int64_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_089: [** If value is NULL, umocktypes_stringify_int64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_090: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_091: [** If any other error occurs when creating the string representation, umocktypes_stringify_int64_t shall return NULL. **]**

## umocktypes_are_equal_int64_t

```c
extern int umocktypes_are_equal_int64_t(const int64_t* left, const int64_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_092: [** umocktypes_are_equal_int64_t shall compare the 2 int64_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_093: [** If any of the arguments is NULL, umocktypes_are_equal_int64_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_094: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int64_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_095: [** If the values pointed to by left and right are different, umocktypes_are_equal_int64_t shall return 0. **]**

## umocktypes_copy_int64_t

```c
extern int umocktypes_copy_int64_t(int64_t* destination, const int64_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_096: [** umocktypes_copy_int64_t shall copy the int64_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_097: [** On success umocktypes_copy_int64_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_098: [** If source or destination are NULL, umocktypes_copy_int64_t shall return a non-zero value. **]**

## umocktypes_free_int64_t

```c
extern void umocktypes_free_int64_t(int64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_099: [** umocktypes_free_int64_t shall do nothing. **]**

## umocktypes_stringify_uint_fast8_t

```c
extern char* umocktypes_stringify_uint_fast8_t(const uint_fast8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_100: [** umocktypes_stringify_uint_fast8_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_101: [** If value is NULL, umocktypes_stringify_uint_fast8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_102: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint_fast8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_103: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint_fast8_t shall return NULL. **]**

## umocktypes_are_equal_uint_fast8_t

```c
extern int umocktypes_are_equal_uint_fast8_t(const uint_fast8_t* left, const uint_fast8_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_104: [** umocktypes_are_equal_uint_fast8_t shall compare the 2 uint_fast8_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_105: [** If any of the arguments is NULL, umocktypes_are_equal_uint_fast8_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_106: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint_fast8_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_107: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint_fast8_t shall return 0. **]**

## umocktypes_copy_uint_fast8_t

```c
extern int umocktypes_copy_uint_fast8_t(uint_fast8_t* destination, const uint_fast8_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_108: [** umocktypes_copy_uint_fast8_t shall copy the uint_fast8_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_109: [** On success umocktypes_copy_uint_fast8_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_110: [** If source or destination are NULL, umocktypes_copy_uint_fast8_t shall return a non-zero value. **]**

## umocktypes_free_uint_fast8_t

```c
extern void umocktypes_free_uint_fast8_t(uint_fast8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_111: [** umocktypes_free_uint_fast8_t shall do nothing. **]**

## umocktypes_stringify_uint_fast8_t

```c
extern char* umocktypes_stringify_int_fast8_t(const int_fast8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_112: [** umocktypes_stringify_int_fast8_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_113: [** If value is NULL, umocktypes_stringify_int_fast8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_114: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int_fast8_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_115: [** If any other error occurs when creating the string representation, umocktypes_stringify_int_fast8_t shall return NULL. **]**

## umocktypes_are_equal_int_fast8_t

```c
extern int umocktypes_are_equal_int_fast8_t(const int_fast8_t* left, const int_fast8_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_116: [** umocktypes_are_equal_int_fast8_t shall compare the 2 int_fast8_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_117: [** If any of the arguments is NULL, umocktypes_are_equal_int_fast8_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_118: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int_fast8_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_119: [** If the values pointed to by left and right are different, umocktypes_are_equal_int_fast8_t shall return 0. **]**

## umocktypes_copy_int_fast8_t

```c
extern int umocktypes_copy_int_fast8_t(int_fast8_t* destination, const int_fast8_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_120: [** umocktypes_copy_int_fast8_t shall copy the int_fast8_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_121: [** On success umocktypes_copy_int_fast8_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_122: [** If source or destination are NULL, umocktypes_copy_int_fast8_t shall return a non-zero value. **]**

## umocktypes_free_int_fast8_t

```c
extern void umocktypes_free_int_fast8_t(int_fast8_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_123: [** umocktypes_free_int_fast8_t shall do nothing. **]**

## umocktypes_stringify_uint_fast16_t

```c
extern char* umocktypes_stringify_uint_fast16_t(const uint_fast16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_124: [** umocktypes_stringify_uint_fast16_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_125: [** If value is NULL, umocktypes_stringify_uint_fast16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_126: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint_fast16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_127: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint_fast16_t shall return NULL. **]**

## umocktypes_are_equal_uint_fast16_t

```c
extern int umocktypes_are_equal_uint_fast16_t(const uint_fast16_t* left, const uint_fast16_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_128: [** umocktypes_are_equal_uint_fast16_t shall compare the 2 uint_fast16_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_129: [** If any of the arguments is NULL, umocktypes_are_equal_uint_fast16_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_130: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint_fast16_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_131: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint_fast16_t shall return 0. **]**

## umocktypes_copy_uint_fast16_t

```c
extern int umocktypes_copy_uint_fast16_t(uint_fast16_t* destination, const uint_fast16_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_132: [** umocktypes_copy_uint_fast16_t shall copy the uint_fast16_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_133: [** On success umocktypes_copy_uint_fast16_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_134: [** If source or destination are NULL, umocktypes_copy_uint_fast16_t shall return a non-zero value. **]**

## umocktypes_free_uint_fast16_t

```c
extern void umocktypes_free_uint_fast16_t(uint_fast16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_135: [** umocktypes_free_uint_fast16_t shall do nothing. **]**

## umocktypes_stringify_int_fast16_t

```c
extern char* umocktypes_stringify_int_fast16_t(const int_fast16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_136: [** umocktypes_stringify_int_fast16_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_137: [** If value is NULL, umocktypes_stringify_int_fast16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_138: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int_fast16_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_139: [** If any other error occurs when creating the string representation, umocktypes_stringify_int_fast16_t shall return NULL. **]**

## umocktypes_are_equal_int_fast16_t

```c
extern int umocktypes_are_equal_int_fast16_t(const int_fast16_t* left, const int_fast16_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_140: [** umocktypes_are_equal_int_fast16_t shall compare the 2 int_fast16_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_141: [** If any of the arguments is NULL, umocktypes_are_equal_int_fast16_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_142: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int_fast16_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_143: [** If the values pointed to by left and right are different, umocktypes_are_equal_int_fast16_t shall return 0. **]**

## umocktypes_copy_int_fast16_t

```c
extern int umocktypes_copy_int_fast16_t(int_fast16_t* destination, const int_fast16_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_144: [** umocktypes_copy_int_fast16_t shall copy the int_fast16_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_145: [** On success umocktypes_copy_int_fast16_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_146: [** If source or destination are NULL, umocktypes_copy_int_fast16_t shall return a non-zero value. **]**

## umocktypes_free_int_fast16_t

```c
extern void umocktypes_free_int_fast16_t(int_fast16_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_147: [** umocktypes_free_int_fast16_t shall do nothing. **]**

## umocktypes_stringify_uint_fast32_t

```c
extern char* umocktypes_stringify_uint_fast32_t(const uint_fast32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_148: [** umocktypes_stringify_uint_fast32_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_149: [** If value is NULL, umocktypes_stringify_uint_fast32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_150: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint_fast32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_151: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint_fast32_t shall return NULL. **]**

## umocktypes_are_equal_uint_fast32_t

```c
extern int umocktypes_are_equal_uint_fast32_t(const uint_fast32_t* left, const uint_fast32_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_152: [** umocktypes_are_equal_uint_fast32_t shall compare the 2 uint_fast32_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_153: [** If any of the arguments is NULL, umocktypes_are_equal_uint_fast32_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_154: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint_fast32_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_155: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint_fast32_t shall return 0. **]**

## umocktypes_copy_uint_fast32_t

```c
extern int umocktypes_copy_uint_fast32_t(uint_fast32_t* destination, const uint_fast32_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_156: [** umocktypes_copy_uint_fast32_t shall copy the uint_fast32_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_157: [** On success umocktypes_copy_uint_fast32_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_158: [** If source or destination are NULL, umocktypes_copy_uint_fast32_t shall return a non-zero value. **]**

## umocktypes_free_uint_fast32_t

```c
extern void umocktypes_free_uint_fast32_t(uint_fast32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_159: [** umocktypes_free_uint_fast32_t shall do nothing. **]**

## umocktypes_stringify_int_fast32_t

```c
extern char* umocktypes_stringify_int_fast32_t(const int_fast32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_160: [** umocktypes_stringify_int_fast32_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_161: [** If value is NULL, umocktypes_stringify_int_fast32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_162: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int_fast32_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_163: [** If any other error occurs when creating the string representation, umocktypes_stringify_int_fast32_t shall return NULL. **]**

## umocktypes_are_equal_int_fast32_t

```c
extern int umocktypes_are_equal_int_fast32_t(const int_fast32_t* left, const int_fast32_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_164: [** umocktypes_are_equal_int_fast32_t shall compare the 2 int_fast32_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_165: [** If any of the arguments is NULL, umocktypes_are_equal_int_fast32_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_166: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int_fast32_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_167: [** If the values pointed to by left and right are different, umocktypes_are_equal_int_fast32_t shall return 0. **]**

## umocktypes_copy_int_fast32_t

```c
extern int umocktypes_copy_int_fast32_t(int_fast32_t* destination, const int_fast32_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_168: [** umocktypes_copy_int_fast32_t shall copy the int_fast32_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_169: [** On success umocktypes_copy_int_fast32_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_170: [** If source or destination are NULL, umocktypes_copy_int_fast32_t shall return a non-zero value. **]**

## umocktypes_free_int_fast32_t

```c
extern void umocktypes_free_int_fast32_t(int_fast32_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_171: [** umocktypes_free_int_fast32_t shall do nothing. **]**

## umocktypes_stringify_uint_fast64_t

```c
extern char* umocktypes_stringify_uint_fast64_t(const uint_fast64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_172: [** umocktypes_stringify_uint_fast64_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_173: [** If value is NULL, umocktypes_stringify_uint_fast64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_174: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_uint_fast64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_175: [** If any other error occurs when creating the string representation, umocktypes_stringify_uint_fast64_t shall return NULL. **]**

## umocktypes_are_equal_uint_fast64_t

```c
extern int umocktypes_are_equal_uint_fast64_t(const uint_fast64_t* left, const uint_fast64_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_176: [** umocktypes_are_equal_uint_fast64_t shall compare the 2 uint_fast64_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_177: [** If any of the arguments is NULL, umocktypes_are_equal_uint_fast64_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_178: [** If the values pointed to by left and right are equal, umocktypes_are_equal_uint_fast64_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_179: [** If the values pointed to by left and right are different, umocktypes_are_equal_uint_fast64_t shall return 0. **]**

## umocktypes_copy_uint_fast64_t

```c
extern int umocktypes_copy_uint_fast64_t(uint_fast64_t* destination, const uint_fast64_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_180: [** umocktypes_copy_uint_fast64_t shall copy the uint_fast64_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_181: [** On success umocktypes_copy_uint_fast64_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_182: [** If source or destination are NULL, umocktypes_copy_uint_fast64_t shall return a non-zero value. **]**

## umocktypes_free_uint_fast64_t

```c
extern void umocktypes_free_uint_fast64_t(uint_fast64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_183: [** umocktypes_free_uint_fast64_t shall do nothing. **]**

## umocktypes_stringify_int_fast64_t

```c
extern char* umocktypes_stringify_int_fast64_t(const int_fast64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_184: [** umocktypes_stringify_int_fast64_t shall return the string representation of value. **]**

**SRS_UMOCKTYPES_STDINT_01_185: [** If value is NULL, umocktypes_stringify_int_fast64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_186: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int_fast64_t shall return NULL. **]**

**SRS_UMOCKTYPES_STDINT_01_187: [** If any other error occurs when creating the string representation, umocktypes_stringify_int_fast64_t shall return NULL. **]**

## umocktypes_are_equal_int_fast64_t

```c
extern int umocktypes_are_equal_int_fast64_t(const int_fast64_t* left, const int_fast64_t* right);
```

**SRS_UMOCKTYPES_STDINT_01_188: [** umocktypes_are_equal_int_fast64_t shall compare the 2 int_fast64_t values pointed to by left and right. **]**

**SRS_UMOCKTYPES_STDINT_01_189: [** If any of the arguments is NULL, umocktypes_are_equal_int_fast64_t shall return -1. **]**

**SRS_UMOCKTYPES_STDINT_01_190: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int_fast64_t shall return 1. **]**

**SRS_UMOCKTYPES_STDINT_01_191: [** If the values pointed to by left and right are different, umocktypes_are_equal_int_fast64_t shall return 0. **]**

## umocktypes_copy_int_fast64_t

```c
extern int umocktypes_copy_int_fast64_t(int_fast64_t* destination, const int_fast64_t* source);
```

**SRS_UMOCKTYPES_STDINT_01_192: [** umocktypes_copy_int_fast64_t shall copy the int_fast64_t value from source to destination. **]**

**SRS_UMOCKTYPES_STDINT_01_193: [** On success umocktypes_copy_int_fast64_t shall return 0. **]**

**SRS_UMOCKTYPES_STDINT_01_194: [** If source or destination are NULL, umocktypes_copy_int_fast64_t shall return a non-zero value. **]**

## umocktypes_free_int_fast64_t

```c
extern void umocktypes_free_int_fast64_t(int_fast64_t* value);
```

**SRS_UMOCKTYPES_STDINT_01_195: [** umocktypes_free_int_fast64_t shall do nothing. **]**
