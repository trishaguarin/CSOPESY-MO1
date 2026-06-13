#ifndef KHR_KHRPLATFORM_H
#define KHR_KHRPLATFORM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#ifndef KHRONOS_APICALL
#  if defined(_WIN32)
#    define KHRONOS_APICALL __declspec(dllimport)
#  else
#    define KHRONOS_APICALL
#  endif
#endif

#ifndef KHRONOS_APIENTRY
#  if defined(_WIN32)
#    define KHRONOS_APIENTRY __stdcall
#  else
#    define KHRONOS_APIENTRY
#  endif
#endif

#ifndef KHRONOS_APIATTRIBUTES
#  define KHRONOS_APIATTRIBUTES
#endif

#ifndef KHRONOS_SUPPORT_INT64
#  define KHRONOS_SUPPORT_INT64 1
#endif

typedef unsigned char khronos_uint8_t;
typedef signed char khronos_int8_t;
typedef unsigned short khronos_uint16_t;
typedef signed short khronos_int16_t;
typedef unsigned int khronos_uint32_t;
typedef signed int khronos_int32_t;

#if KHRONOS_SUPPORT_INT64
typedef unsigned long long khronos_uint64_t;
typedef signed long long khronos_int64_t;
#endif

typedef khronos_uint32_t khronos_float_t;
typedef khronos_uint32_t khronos_boolean_enum_t;

typedef size_t khronos_usize_t;

typedef ptrdiff_t khronos_ssize_t;

#if defined(_WIN64)
typedef unsigned long long khronos_uintptr_t;
typedef long long khronos_intptr_t;
#else
typedef unsigned long khronos_uintptr_t;
typedef long khronos_intptr_t;
#endif

#if defined(__cplusplus)
}
#endif

#endif /* KHR_KHRPLATFORM_H */
