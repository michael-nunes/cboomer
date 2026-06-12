#ifndef __OPTION_H__
#define __OPTION_H__

#ifndef option_die
#define option_die _option_die
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
static void _option_die(const char *msg) {
	fprintf(stderr,"Error: %s\n",msg);
	if(errno != 0) {
		perror("\tdetails:");
	}
	exit(1);
}
#endif

#define macro_string(exp) #exp
#define macro_string_valueof(exp) macro_string(exp)

#define option(type) type ##_option

#define declare_option(type) \
	typedef struct { type Value; int HasValue; } option(type);

#define declare_option_ptr(type) \
	typedef struct { type *Value; int HasValue; } option_ptr(type);

#define expect_temp_var(id) _expect_temp_ ## id

#define expect(type, dst, val, msg) \
	do { \
		option(type) expect_temp_var(1) = (val); \
		if(!expect_temp_var(1).HasValue) { option_die(msg); } (dst) = expect_temp_var(1).Value; \
	} while(0)

#define expect_ptr(type, dst, val, msg) \
	do { \
		type expect_temp_var(1) = (val); \
		if(expect_temp_var(1) == NULL) { option_die(msg); } (dst) = expect_temp_var(1); \
	}while(0)

#define unwrap(type, dst, val) expect(type, dst, val, "unwrap:"  __FILE__ ":" macro_string_valueof(__LINE__) ":" macro_string(val))
#define unwrap_ptr(type, dst, val) expect_ptr(type, dst, val, "unwrap:" __FILE__ ":" macro_string_valueof(__LINE__) ":" macro_string(val))

#define option_result_if(type,value,cond) \
	((option(type)){.Value = (value),.HasValue = cond})

#define option_result(type,value) option_result_if(type,value,1)
#define option_result_empty(type, value) option_result_if(type,value,0)


#endif
