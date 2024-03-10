#include "sxsutils.h"

/*
Implementation referenced from HaveSxS and manifestenum
https://github.com/asdcorp/haveSxS
https://github.com/himselfv/manifestenum
*/

void SxSLower(LPWSTR s, LPWSTR* out) {
    if (!s) { *out = NULL; return; }

    int len = lstrlenW(s);
    LPWSTR res = (LPWSTR)malloc((len + 1) * sizeof(WCHAR));

    if (!res) { *out = NULL; return; }

    memset(res, 0, (len + 1) * sizeof(WCHAR));

    int j = 0;
    for (int i = 0; i < len; i++) {
        wchar_t c = s[i];
        if ((c >= L'0' && c <= L'9') ||
            (c >= L'a' && c <= L'z') ||
            c == L'.' || c == L'\\' || c == L'-' || c == L'_')
        {
            res[j] = c;
            j++;
        }
        else if (c >= L'A' && c <= L'Z') {
            res[j] = c + L' ';
            j++;
        }
    }

    *out = res;
}

ULONG64 SxSHash(LPCWSTR data) {
    if (!data) {
        return 0;
    }

    int len = lstrlenW(data);

    ULONG64 hashes[4] = { 0 };

    for (int i = 0; i < len; i++) {
        WORD c = data[i];
        int j = i % 4;

        hashes[j] = (0x1003F * hashes[j] + c) & 0xFFFFFFFF;
    }

    return 0x1E5FFFFFD27 * hashes[0] + 0xFFFFFFDC00000051 * hashes[1] + 0x1FFFFFFF7 * hashes[2] + hashes[3];
}

void SxSNameFromAttrs(ASSEMBLY_ATTRIBUTES* attrs, LPWSTR* psxs_name) {
    LPWSTR new_sxs_name = (LPWSTR)malloc(sizeof(WCHAR) * MAX_PATH);

    if (!new_sxs_name || !attrs->name) {
        *psxs_name = NULL;
        return;
    }

    ULONG64 hash = 0;
    hash = HASH_COMBINE(hash, L"name", attrs->name);
    hash = HASH_COMBINE(hash, L"culture", attrs->culture);
    hash = HASH_COMBINE(hash, L"type", attrs->type);
    hash = HASH_COMBINE(hash, L"version", attrs->version);
    hash = HASH_COMBINE(hash, L"publickeytoken", attrs->public_key_token);
    hash = HASH_COMBINE(hash, L"processorarchitecture", attrs->architecture);
    hash = HASH_COMBINE(hash, L"versionscope", attrs->version_scope);
    hash &= 0xFFFFFFFFFFFFFFFF;

    LPWSTR shortname = _wcsdup(attrs->name);
    int name_len = lstrlenW(shortname);

    if (lstrlenW(shortname) > 40) {
        wsprintf(shortname, L"%.19s..%.19s", attrs->name, &attrs->name[name_len - 19]);
    }

    StringCbPrintfW(
        new_sxs_name,
        sizeof(WCHAR) * MAX_PATH,
        L"%s_%s_%s_%s_%s_%016llx",
        attrs->architecture,
        shortname,
        attrs->public_key_token,
        attrs->version,
        attrs->culture ? attrs->culture : L"none",
        hash
    );
    free(shortname);

    *psxs_name = new_sxs_name;
}

void DefIdentToAttrs(IDefinitionIdentity* ident, ASSEMBLY_ATTRIBUTES* attrs) {
    LPWSTR name, version, culture, public_key_token, architecture, version_scope, type;

    ident->GetAttribute(NULL, L"Name", &name);
    ident->GetAttribute(NULL, L"Version", &version);
    ident->GetAttribute(NULL, L"Language", &culture);
    ident->GetAttribute(NULL, L"PublicKeyToken", &public_key_token);
    ident->GetAttribute(NULL, L"ProcessorArchitecture", &architecture);
    ident->GetAttribute(NULL, L"VersionScope", &version_scope);
    ident->GetAttribute(NULL, L"Type", &type);

    SxSLower(name, &attrs->name);
    SxSLower(version, &attrs->version);
    SxSLower(culture, &attrs->culture);
    SxSLower(public_key_token, &attrs->public_key_token);
    SxSLower(architecture, &attrs->architecture);
    SxSLower(version_scope, &attrs->version_scope);
    SxSLower(type, &attrs->type);
}

void SxSNameFromDefinitionIdentity(IDefinitionIdentity* ident, LPWSTR* psxs_name) {
    ASSEMBLY_ATTRIBUTES attrs;
    DefIdentToAttrs(ident, &attrs);
    SxSNameFromAttrs(&attrs, psxs_name);
}