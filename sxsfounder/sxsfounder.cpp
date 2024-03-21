#include <iostream>
#include <objbase.h>
#include <wrl/client.h>
#include "wcp.h"

using Microsoft::WRL::ComPtr;

int wmain(int argc, LPCWSTR argv[])
{
    printf("sxsfounder 0.0.3 by witherornot\n");

    if (argc < 3) {
        wprintf(L"Usage: %s <offline image path> <architecture>\n", argv[0]);
        wprintf(L"Supported architectures: amd64, x86, arm, arm64\n");
        return 1;
    }

    LPCWSTR offlineImage = argv[1];
    LPCWSTR arch = argv[2];
    DWORD archVal = -1;
    
    if (!wcscmp(arch, L"amd64")) {
        archVal = 9;
    }
    else if (!wcscmp(arch, L"x86")) {
        archVal = 0;
    }
    else if (!wcscmp(arch, L"arm")) {
        archVal = 5;
    }
    else if (!wcscmp(arch, L"arm64")) {
        archVal = 12;
    }
    else {
        wprintf(L"ERROR: %s is not a supported architecture\n", arch);
        return 1;
    }
    
    WCHAR fullOfflineImage[MAX_PATH];
    
    GetFullPathNameW(offlineImage, MAX_PATH, fullOfflineImage, NULL);

    wprintf(L"Creating offline image at %s\n", fullOfflineImage);

    HMODULE wcp = LoadLibraryA("wcp.dll");

    if (!wcp) {
        printf("ERROR: wcp.dll failed to load\n");
        return 1;
    }

    if (FAILED(CoInitialize(NULL))) {
        printf("ERROR: CoInitialize FAILED\n");
        return 1;
    }

    auto WcpInitialize = (PWCP_INITIALIZE_FUNCTION)GetProcAddress(wcp, "WcpInitialize");
    auto WcpShutdown = (PWCP_SHUTDOWN_FUNCTION)GetProcAddress(wcp, "WcpShutdown");
    auto CreateNewPseudoWindows = (PCREATE_NEW_PSEUDO_WINDOWS_FUNCTION)GetProcAddress(wcp, "CreateNewPseudoWindows");
    auto SetIsolationIMalloc = (PSET_ISOLATION_MALLOC_FUNCTION)GetProcAddress(wcp, "SetIsolationIMalloc");
    auto CreateNewWindows = (PCREATE_NEW_WINDOWS_FUNCTION)GetProcAddress(wcp, "CreateNewWindows");
    auto CreateNewOfflineStore = (PCREATE_NEW_OFFLINE_STORE_FUNCTION)GetProcAddress(wcp, "CreateNewOfflineStore");
    auto DismountRegistryHives = (PDISMOUNT_REGISTRY_HIVES_FUNCTION)GetProcAddress(wcp, "DismountRegistryHives");

    if (
        !WcpInitialize ||
        !WcpShutdown ||
        !CreateNewPseudoWindows ||
        !SetIsolationIMalloc ||
        !CreateNewOfflineStore ||
        !CreateNewWindows ||
        !DismountRegistryHives
    ) {
        printf("ERROR: wcp.dll does not contain correct functions\n");
        return 1;
    }

    void* cookie;
    if (FAILED(WcpInitialize(&cookie))) {
        printf("ERROR: WcpInitialize FAILED\n");
        return 1;
    }

    IMalloc* alloc;
    if (FAILED(CoGetMalloc(1, &alloc))) {
        printf("ERROR: CoGetMalloc FAILED\n");
        return 1;
    }

    if (FAILED(SetIsolationIMalloc(alloc))) {
        printf("ERROR: SetIsolationMalloc FAILED\n");
        alloc->Release();
        return 1;
    }

    HRESULT result;
    OFFLINE_STORE_CREATION_PARAMETERS pParameters = { sizeof(OFFLINE_STORE_CREATION_PARAMETERS) };

    pParameters.pszHostSystemDrivePath = fullOfflineImage;
    pParameters.ulProcessorArchitecture = archVal;

    void* regKeys;

    DWORD disposition;
    result = CreateNewWindows(0, L"C:\\", &pParameters, &regKeys, &disposition);

    if (FAILED(result)) {
        printf("ERROR: CreateNewWindows FAILED 0x%08x\n", result);
        DismountRegistryHives(regKeys);
        alloc->Release();
        return 1;
    }
    else {
        printf("CreateNewWindows SUCCESS\n");
    }

    ComPtr<ICSIStore> store;
    result = CreateNewOfflineStore(0, &pParameters, __uuidof(ICSIStore), (IUnknown**)&store, &disposition);

    if (FAILED(result)) {
        printf("ERROR: CreateNewOfflineStore FAILED 0x%08x\n", result);
        DismountRegistryHives(regKeys);
        alloc->Release();
        return 1;
    }
    else {
        printf("CreateNewOfflineStore SUCCESS\n\n");
    }

    printf("Parameters:\n");
    wprintf(L"HostSystemDrivePath: %s\n", pParameters.pszHostSystemDrivePath);
    wprintf(L"HostWindowsDirectoryPath: %s\n", pParameters.pszHostWindowsDirectoryPath);
    wprintf(L"TargetWindowsDirectoryPath: %s\n", pParameters.pszTargetWindowsDirectoryPath);
    wprintf(L"HostRegistryMachineSoftwarePath: %s\n", pParameters.pszHostRegistryMachineSoftwarePath);
    wprintf(L"HostRegistryMachineSystemPath: %s\n", pParameters.pszHostRegistryMachineSystemPath);
    wprintf(L"HostRegistryMachineSecurityPath: %s\n", pParameters.pszHostRegistryMachineSecurityPath);
    wprintf(L"HostRegistryMachineSAMPath: %s\n", pParameters.pszHostRegistryMachineSAMPath);
    wprintf(L"HostRegistryMachineComponentsPath: %s\n", pParameters.pszHostRegistryMachineComponentsPath);
    wprintf(L"HostRegistryUserDotDefaultPath: %s\n", pParameters.pszHostRegistryUserDotDefaultPath);
    wprintf(L"HostRegistryDefaultUserPath: %s\n", pParameters.pszHostRegistryDefaultUserPath);
    wprintf(L"ProcessorArchitecture: 0x%08x\n", pParameters.ulProcessorArchitecture);
    wprintf(L"HostRegistryMachineOfflineSchemaPath: %s\n\n", pParameters.pszHostRegistryMachineOfflineSchemaPath);

    DismountRegistryHives(regKeys);
    // WcpShutdown(cookie);
    FreeLibrary(wcp);

    return 0;
}