

#ifndef cJSON_Utils__h
#define cJSON_Utils__h

#ifdef __cplusplus
extern "C"
{
#endif

#include "cJSON.h"

CJSON_PUBLIC(cJSON *) cJSONUtils_GetPointer(cJSON * const object, const char *pointer);
CJSON_PUBLIC(cJSON *) cJSONUtils_GetPointerCaseSensitive(cJSON * const object, const char *pointer);

CJSON_PUBLIC(cJSON *) cJSONUtils_GeneratePatches(cJSON * const from, cJSON * const to);
CJSON_PUBLIC(cJSON *) cJSONUtils_GeneratePatchesCaseSensitive(cJSON * const from, cJSON * const to);

CJSON_PUBLIC(void) cJSONUtils_AddPatchToArray(cJSON * const array, const char * const operation, const char * const path, const cJSON * const value);

CJSON_PUBLIC(int) cJSONUtils_ApplyPatches(cJSON * const object, const cJSON * const patches);
CJSON_PUBLIC(int) cJSONUtils_ApplyPatchesCaseSensitive(cJSON * const object, const cJSON * const patches);

CJSON_PUBLIC(cJSON *) cJSONUtils_MergePatch(cJSON *target, const cJSON * const patch);
CJSON_PUBLIC(cJSON *) cJSONUtils_MergePatchCaseSensitive(cJSON *target, const cJSON * const patch);

CJSON_PUBLIC(cJSON *) cJSONUtils_GenerateMergePatch(cJSON * const from, cJSON * const to);
CJSON_PUBLIC(cJSON *) cJSONUtils_GenerateMergePatchCaseSensitive(cJSON * const from, cJSON * const to);

CJSON_PUBLIC(char *) cJSONUtils_FindPointerFromObjectTo(const cJSON * const object, const cJSON * const target);

CJSON_PUBLIC(void) cJSONUtils_SortObject(cJSON * const object);
CJSON_PUBLIC(void) cJSONUtils_SortObjectCaseSensitive(cJSON * const object);

#ifdef __cplusplus
}
#endif

#endif
