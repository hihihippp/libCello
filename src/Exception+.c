
#include "Exception+.h"
#include "Type+.h"
#include "None+.h"

var TypeError = Singleton(TypeError);
var ValueError = Singleton(ValueError);
var ClassError = Singleton(ClassError);
var IndexOutOfBoundsError = Singleton(IndexOutOfBoundsError);
var KeyError = Singleton(KeyError);
var OutOfMemoryError = Singleton(OutOfMemoryError);

bool __exc_active = false;
int __exc_depth = -1;
jmp_buf __exc_buffers[__EXC_MAX_DEPTH];

local var __exc_obj = NULL;
local char __exc_msg[2048];
local const char* __exc_file = "";
local const char* __exc_func = "";
local unsigned int __exc_lineno = 0;

#ifdef __unix__
#include <execinfo.h>

local void* __exc_backtrace[25];
local int __exc_backtrace_count;

#endif

local void __exc_error(void)  {
  
  fprintf(stderr, "\n");
  fprintf(stderr, "!!\t\n");
  
  if (type_implements(type_of(__exc_obj), AsStr)) {
    fprintf(stderr, "!!\tUncaught '%s' at (%s:%s:%i) \n", as_str(__exc_obj), __exc_file, __exc_func, __exc_lineno);
  } else {
    fprintf(stderr, "!!\tUncaught Object at (%s:%s:%i) \n", __exc_file, __exc_func, __exc_lineno);
  }
  
  fprintf(stderr, "!!\t\n");
  fprintf(stderr, "!!\t\t '%s'\n", __exc_msg);
  fprintf(stderr, "!!\t\n");
  
#ifdef __unix__
    
  fprintf(stderr, "!!\tStack Trace: \n");
  fprintf(stderr, "!!\t\n");

  char** symbols = backtrace_symbols(__exc_backtrace, __exc_backtrace_count);

  for (int i = 0; i < __exc_backtrace_count; i++){
    fprintf(stderr, "!!\t\t[%i] %s\n", i, symbols[i]);
  }

  fprintf(stderr, "!!\t\n");

  free(symbols);

#endif
  
  exit(EXIT_FAILURE);
  
}

var __exc_throw(var obj, const char* fmt, const char* file, const char* func, int lineno, ...) {
  
  __exc_obj = obj;
  __exc_file = file;
  __exc_func = func;
  __exc_lineno = lineno;
  
#ifdef __unix__
  __exc_backtrace_count = backtrace(__exc_backtrace, 25);
#endif
  
  va_list va;
  va_start(va, lineno);
  
  vsnprintf(__exc_msg, 2047, fmt, va);
  
  if (__exc_depth >= 0) {
    longjmp(__exc_buffers[__exc_depth], 1);
  } else {
    __exc_error();
  }
  
  return Undefined;
  
}

var __exc_catch(void* empty, ...) {
  
  if (!__exc_active) { return Undefined; }
  
  va_list va;
  va_start(va, empty);
  var e = va_arg(va, var);
  
  while (e != Undefined) {
    if_eq(e, __exc_obj) { return e; }
    e = va_arg(va, var);
  }
  
  return __exc_error(), Undefined;
  
}