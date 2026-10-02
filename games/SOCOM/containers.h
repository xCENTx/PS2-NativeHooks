#ifndef SOCOM_CONTAINERS_H
#define SOCOM_CONTAINERS_H

#include "structs.h"

typedef void  (*ZArrayForEach_Callback)(void* obj, void* ctx);
typedef void* (*ZArrayFind_Callback)(void* obj, void* ctx);

void ZArray_ForEach(ZArray* array, ZArrayForEach_Callback callback, void* context);
void* ZArray_Find(ZArray* array, ZArrayFind_Callback callback, void* context);

#endif