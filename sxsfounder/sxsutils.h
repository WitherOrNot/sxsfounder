#pragma once
#include "wcp.h"
#include <strsafe.h>

typedef struct _ASSEMBLY_ATTRIBUTES {
    LPWSTR name;
    LPWSTR version;
    LPWSTR culture;
    LPWSTR public_key_token;
    LPWSTR architecture;
    LPWSTR version_scope;
    LPWSTR type;
} ASSEMBLY_ATTRIBUTES;

ULONG64 SxSHash(LPCWSTR data);
void DefIdentToAttrs(IDefinitionIdentity* ident, ASSEMBLY_ATTRIBUTES* attrs);
void SxSNameFromAttrs(ASSEMBLY_ATTRIBUTES* attrs, LPWSTR* psxs_name);
void SxSNameFromDefinitionIdentity(IDefinitionIdentity* ident, LPWSTR* sxs_name);

#define HASH_COMBINE(hash, attr, val) val ? (0x1FFFFFFF7 * hash + 0x1FFFFFFF7 * SxSHash(attr) + SxSHash(val)) : hash
