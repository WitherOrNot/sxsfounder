#include <iostream>
#include <objbase.h>
#include <wrl/client.h>
#include "wcp.h"
#include "sxsutils.h"

using Microsoft::WRL::ComPtr;

int wmain(int argc, LPCWSTR argv[])
{
    printf("sxsfounder 0.0.1 by witherornot\n");


    if (argc < 4) {
        wprintf(L"Usage: %s <sxs source folder> <deployment manifest> <offline image path>\n", argv[0]);
        return 1;
    }

    LPCWSTR sxsFolder = argv[1];
    LPCWSTR deplPath = argv[2];
    LPCWSTR offlineImage = argv[3];

    WCHAR fullSxsFolder[MAX_PATH];
    WCHAR fullDeplPath[MAX_PATH];
    WCHAR fullOfflineImage[MAX_PATH];

    GetFullPathNameW(sxsFolder, MAX_PATH, fullSxsFolder, NULL);
    GetFullPathNameW(deplPath, MAX_PATH, fullDeplPath, NULL);
    GetFullPathNameW(offlineImage, MAX_PATH, fullOfflineImage, NULL);

    wprintf(L"SxS Source: %s\nDeployment Manifest: %s\nOffline Image: %s\n", fullSxsFolder, fullDeplPath, fullOfflineImage);

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
    auto ParseManifest = (PPARSE_MANIFEST_FUNCTION)GetProcAddress(wcp, "ParseManifest");

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
    pParameters.ulProcessorArchitecture = 9;

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

    ComPtr<ICSITransaction> txn;
    result = store->BeginTransaction(0, __uuidof(ICSITransaction), L"SxSFounder", &txn);

    ComPtr<ICSITransaction2> deplTxn;
    result = txn->QueryInterface(__uuidof(ICSITransaction2), &deplTxn);

    ComPtr<IDefinitionIdentity> deployment;
    ParseManifest(fullDeplPath, NULL, __uuidof(IDefinitionIdentity), &deployment);

    if (FAILED(result)) {
        wprintf(L"ERROR: Deployment ParseManifest FAILED 0x%08x\n", result);
        DismountRegistryHives(regKeys);
        alloc->Release();
        return 1;
    }

    txn->AddComponent(0, deployment.Get(), fullDeplPath, &disposition);

    ComPtr<IEnumDefinitionIdentity> deplEnum;
    txn->EnumMissingComponents(0, &deplEnum, &disposition);

    SIZE_T cbFetched = 1;

    while (cbFetched) {
        ComPtr<IDefinitionIdentity> depComponent;
        ASSEMBLY_ATTRIBUTES attrs;
        LPWSTR sxsName;
        WCHAR manifPath[MAX_PATH];

        result = deplEnum->Next(1, &depComponent, &cbFetched);

        if (SUCCEEDED(result) && cbFetched) {
            DefIdentToAttrs(depComponent.Get(), &attrs);
            wprintf(L"Found missing component: %s\n", attrs.name);
            SxSNameFromAttrs(&attrs, &sxsName);
            StringCbPrintfW(manifPath, MAX_PATH * sizeof(WCHAR), L"%s\\%s.manifest", fullSxsFolder, sxsName);
            wprintf(L"Reading manifest %s\n", manifPath);

            ComPtr<IDefinitionIdentity> manifest;
            result = ParseManifest(manifPath, NULL, __uuidof(IDefinitionIdentity), &manifest);

            if (FAILED(result)) {
                wprintf(L"ERROR: Component ParseManifest FAILED 0x%08x\n", result);
                DismountRegistryHives(regKeys);
                alloc->Release();
                return 1;
            }

            txn->AddComponent(0, manifest.Get(), manifPath, &disposition);

            ComPtr<IEnumCSI_FILE> manifEnum;
            txn->EnumMissingFiles(0x4 | 0x1, &manifEnum);

            wprintf(L"Placing missing files...\n");

            ULONG64 pbFetched = TRUE;

            while (pbFetched) {
                CSIFILE file;
                result = manifEnum->Next(1, &file, &pbFetched);

                if (SUCCEEDED(result) && pbFetched) {
                    WCHAR fpath[MAX_PATH];
                    StringCbPrintfW(fpath, MAX_PATH * sizeof(WCHAR), L"%s\\%s\\%s", fullSxsFolder, sxsName, file.fname);
                    wprintf(L"Placing %s\n", file.fname);
                    result = txn->AddFile(0, file.defIds[0], file.fname, fpath, &disposition);

                    if (FAILED(result)) {
                        wprintf(L"ERROR: AddFile %s FAILED 0x%08x\n", file.fname, result);
                        DismountRegistryHives(regKeys);
                        alloc->Release();
                        return 1;
                    }
                }
            }

            free(sxsName);
        }
    }

    txn->PinDeployment(0, deployment.Get(), NULL, NULL, NULL, NULL, fullDeplPath, NULL, 0, 0);
    deplTxn->MarkDeploymentStaged(0, deployment.Get(), NULL, NULL, NULL, 0);
    txn->InstallDeployment(0, deployment.Get(), NULL, NULL, NULL, NULL, fullDeplPath, NULL, 0);

    result = txn->Commit(0, NULL, NULL);

    DismountRegistryHives(regKeys);
    // WcpShutdown(cookie);
    FreeLibrary(wcp);

    return 0;
}