#ifndef __DYNAMIC_ARRAY__
#define __DYNAMIC_ARRAY__

#include <stdlib.h>

#ifndef addr_of
#define addr_of(x) &(x)
#endif

#define value_of(ptr) *(ptr)
#define set_value_of(ptr, v) value_of(ptr) = (v)

#define DaForeach(el, arr)\
	for(int I = 0; (I < (arr).Count) && ((el = (arr).Items[I]), 1) ; I++) 

#define da_fields(type) int Capacity; int Count; type * Items

void * DaGrow_(void *Storage, int *Capacity, int Count, int ToGrow, int ItemSize) {
	void * Items = Storage;
	int Required = (Count + ToGrow);
	if(value_of(Capacity) < Required) {	
		int NewCapacity = value_of(Capacity) > 0 ? value_of(Capacity) * 2 : 4;
		while(NewCapacity < Required) NewCapacity *= 2;
		Items = realloc(Storage, ItemSize * NewCapacity);
		set_value_of(Capacity, NewCapacity);
	}
	return (Items);
}

void DaFree_(void **Storage, int *Capacity, int *Count) {
	if(value_of(Storage) != NULL) {
		free(*Storage);
		set_value_of(Capacity, 0);
		set_value_of(Count, 0);
		set_value_of(Storage, NULL);
	}
}

#define DaLength(_Arr)   (_Arr)->Count
#define DaCapacity(_Arr) (_Arr)->Capacity

#define DaFree(_Arr) do { if((_Arr)->Items) { DaFree_((void**)addr_of((_Arr)->Items), addr_of((_Arr)->Capacity), addr_of((_Arr)->Count)); } } while(0);

#define DaPush(T, _Arr, _Value) \
	do { \
		/* It avoid bugs when _Value is a function calling eg.: DaPush(int, MyArray, MyIntGeneratorFunc()) */ \
		T _DaPushTempValue = (_Value); \
		if (DaLength(_Arr) >= DaCapacity(_Arr)) { \
			(_Arr)->Items = DaGrow_((_Arr)->Items, addr_of((_Arr)->Capacity), (_Arr)->Count, 1, sizeof(_DaPushTempValue)); \
		} \
		(_Arr)->Items[DaLength(_Arr)] = (_DaPushTempValue); \
		(_Arr)->Count++; \
	} while(0); 

#endif
