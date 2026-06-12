#ifndef __CBOOMER_STD__
#define __CBOOMER_STD__

// 
// CBoomer Standard Library
//

#include "da.h"
#include "str.h"

typedef struct {
	da_fields(char);
} string_builder;


string_builder GetConfigDir(const char *Append);

void StringBuilder_Append(string_builder *, const char *);

void Path_Append(string_builder *, const char *Part);

int  Path_Exists(string_builder *);

//
// Read all file content as str
// 
// On sucess return a pointer of str that should be freed with StrFree, otherwise
// returns NULL
str * ReadEntireFile(const char *Path);

#endif

#ifdef CBOOMER_STD_IMPLEMENTATION
#undef CBOOMER_STD_IMPLEMENTATION

#define STR_IMPLEMENTATION
#include "str.h"

#include <stdlib.h>
#include <unistd.h>

str * ReadEntireFile(const char *Path) {
	str * Result  = 0;
	int RequiredSize;
	int Error = 1;
	FILE *File = fopen(Path, "r");
	if(!File) goto defer;

	fseek(File,0,SEEK_END);
	RequiredSize = ftell(File);
	fseek(File,0,SEEK_SET);
	Result = StrNewEx(NULL, RequiredSize);
	if(Result== NULL) goto defer;
	Error = fread(StrGetBuffer(Result), 1, RequiredSize, File) == 0;
	Result->Length = RequiredSize;
	defer: {
				 if(File) fclose(File);
				 if(Error && Result != NULL) {
					 StrFree(Result);
					 Result = NULL;
				 }
				 return (Result);
			 }
}

string_builder GetConfigDir(const char *Append) {
	string_builder Result = {0};
	char *Home = getenv("HOME");
	Path_Append(&Result, Home);
	Path_Append(&Result, ".config");
	Path_Append(&Result, Append);
	return (Result);
}

void StringBuilder_Append(string_builder *Sb, const char *CStr) {
	int Length = strlen(CStr);
	Sb->Items = DaGrow_(
			Sb->Items,
			addr_of(Sb->Capacity),
			Sb->Count, 
			Length,
			sizeof(char));
	char *Dst = (Sb->Items + Sb->Count);
	memcpy(Dst,CStr,Length);
	Sb->Count += Length;
}

void Path_Append(string_builder *Path, const char *Append) {
	if(Append && Append[0] == '/') {
		if(Path->Count && Path->Items[Path->Count-1] == '/') {
			Path->Count--;
		}
		StringBuilder_Append(Path, Append);
	} else if(Append) {
		if(Path->Count && Path->Items[Path->Count-1] != '/') {
			DaPush(char, Path, '/');
		}
		StringBuilder_Append(Path,Append);
	}
}

int  Path_Exists(string_builder *Sb) {
	int Discard_End = 0;
	int Exists = 0;
	if(Sb->Count) {
		if(Sb->Items[Sb->Count-1] != '\0') {
			DaPush(char, Sb, '\0');
			Discard_End = 1;
		}
		Exists = access(Sb->Items, F_OK) == 0;
		if(Discard_End) Sb->Count--;
	}
	return (Exists);
}

#endif
