#ifndef __STR_H__
#define __STR_H__

#include <string.h>

typedef struct {
	int  Length;
	int  Capacity;
	char Ptr[];
} str;

#define StrGetBuffer(_Str) \
	(char*)((_Str) + 1)

#define STRFMT "%.*s"
#define STRARG(_Str) (_Str)->Length, StrGetBuffer(_Str)

str * StrNew(const char *CStr);
str * StrNewEx(const char *CStr, int MinCapacity);
void  StrFree(str *);

#endif

#if defined(STR_IMPLEMENTATION) 

#ifndef _str_malloc
#include <stdlib.h>
#define _str_malloc malloc
#define _str_free   free
#endif

#define _str_align_size(_sz, _al) \
	(((_sz) + (_al) - 1) & ~((_al) - 1))

str * StrNewEx(const char *CStr, int MinCapacity) {
	int CStrLen    = (CStr != NULL) ?  strlen(CStr) : 0;
	int Capacity = CStrLen;
	if (Capacity < MinCapacity) Capacity = MinCapacity;
	Capacity = _str_align_size(Capacity, 4);
	str *Result = _str_malloc(Capacity + sizeof(str));
	Result->Length = CStrLen;
	Result->Capacity = Capacity;
	if (CStrLen) {
		char *Ptr = (char*)(Result + 1);
		memcpy(Ptr, CStr, CStrLen);
	}
	return (Result);
}

str * StrNew(const char *CStr) {
	return StrNewEx(CStr, 1);
}

void  StrFree(str * Str) {
	if(Str) _str_free(Str);
}

#endif
