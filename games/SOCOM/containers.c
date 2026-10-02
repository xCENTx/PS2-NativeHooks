#include "containers.h"

// iterates on each entity
void ZArray_ForEach(ZArray* arr, ZArrayForEach_Callback cb, void* ctx)
{
    ZIterator* it;
    ZIterator* end;

    if (arr == 0 || cb == 0 || arr->count == 0 || arr->begin == 0 || arr->end == 0)
        return;

    it = (ZIterator*)arr->begin;

    if (it == 0)
        return;

    end = (ZIterator*)it->prev;

    if (end == 0)
        return;

    do
    {
        if (it->data != 0)
            cb((void*)it->data, ctx);

        it = (ZIterator*)it->next;

    } while (it != 0 && it->data != end->data);
}

// returns the first object in the array matching the input
void* ZArray_Find(ZArray* arr, ZArrayFind_Callback cb, void* ctx)
{
    ZIterator* it;
    ZIterator* end;

    if (arr == 0 || cb == 0 || arr->count == 0 || arr->begin == 0 || arr->end == 0)
        return 0;

    it = (ZIterator*)arr->begin;

    if (it == 0)
        return 0;

    end = (ZIterator*)it->prev;

    if (end == 0)
        return 0;

    do
    {
        if (it->data != 0)
        {
            void* result = cb((void*)it->data, ctx);

            if (result != 0)
                return result;
        }

        it = (ZIterator*)it->next;

    } while (it != 0 && it->data != end->data);

    return 0;
}