// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "core/mem/memory.h"
#include "core/util/log.h"
#include "core/math/math.h"
#include "cs2/hooks/hooks.h"
#include "cs2/interface/Interfaces.h"
#include "cs2/convars.h"
#include "cs2/schema/schema.h"
#include "cs2/inputsystem.h"
#include <thread>

bool bInitialized = false;

static bool setup(HMODULE hModule)
{
    if (bInitialized)
        return true;

#ifdef _DEBUG
    if (!console::AttachConsole(XOR(L"Chud Debug Build xp")))
    {
        CS_ASSERT(false);
        return false;
    }
#endif

    if (!console::OpenFile(XOR(L"chudbaselog.txt")))
    {
        L_PRINT(LOG_ERROR) << XOR("failed setting up the log file");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("log initialization completed");

    if (!memory::Setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup memory ");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("memory initialization completed");

    if (!MATH::Setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup math");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("math initialization completed");

    if (!Interface::Setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup interfaces");
        return false;

    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("interface initialization completed");

    std::vector<std::string> vecNeededModules = { XOR("client.dll"), XOR("engine2.dll"), XOR("schemasystem.dll"), XOR("matchmaking.dll") };
    for (auto& szModule : vecNeededModules)
    {
        if (!schema::Setup(XOR(L"schema.txt"), szModule.c_str()))
        {
            L_PRINT(LOG_ERROR) << XOR("failed to setup schema");
            return false;
        }
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("schema initialization completed");

    if (!convar::setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup convars");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("convar initialization completed");


    if (!inputsystem::setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup inputsystem");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("input system initialization completed");

    if (!hooks::setup())
    {
        L_PRINT(LOG_ERROR) << XOR("failed to setup hooks");
        return false;
    }
    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("hooks initialization completed");

    L_PRINT(LOG_NONE) << console::SetColor(CONSOLE_FORE_GREEN | CONSOLE_FORE_INTENSITY) << XOR("Completed chud init!");

    bInitialized = true;
    return true;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	HMODULE hModule = (HMODULE)lpParam;

    //wait for cs modules to load
	while (!memory::GetModuleBaseHandle(NAVSYSTEM_DLL))
	    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    setup(hModule);

	while (!GetAsyncKeyState(VK_DELETE))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));


	FreeLibraryAndExitThread(hModule, 0);
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

