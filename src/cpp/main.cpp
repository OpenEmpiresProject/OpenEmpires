#include "GraphicInstruction.h"
#include "Simulator.h"
#include "ThreadQueue.h"
#include "aion/EventLoop.h"
#include "aion/GameState.h"
#include "aion/GraphicsRegistry.h"
#include "aion/InputListener.h"
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "aion/Viewport.h"
#include "game/ResourceLoader.h"
#include "utils/Logger.h"

#include <iostream>
#include <readerwriterqueue.h>
#include <stop_token>
#include <vector>

#ifdef _WIN32
#include <dbghelp.h>
#include <windows.h>

void printStackTrace()
{
    constexpr int MAX_FRAMES = 64;
    void* frames[MAX_FRAMES];
    USHORT numFrames = CaptureStackBackTrace(0, MAX_FRAMES, frames, nullptr);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE); // Load symbol tables

    SYMBOL_INFO* symbol = (SYMBOL_INFO*) malloc(sizeof(SYMBOL_INFO) + 256);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    std::cout << "Stack trace:\n";
    for (USHORT i = 0; i < numFrames; ++i)
    {
        DWORD64 address = (DWORD64) (frames[i]);
        if (SymFromAddr(process, address, nullptr, symbol))
        {
            std::cout << i << ": " << symbol->Name << " at 0x" << std::hex << symbol->Address
                      << std::dec << "\n";
        }
        else
        {
            std::cout << i << ": [symbol not found]\n";
        }
    }

    free(symbol);
}
#endif

using namespace aion;
using namespace game;

int runGame()
{
    std::cout << "Starting the game1\n";

    utils::initLogger("build/logs/game.log");
    std::cout << "Starting the game2\n";

    spdlog::info("Game started");
    spdlog::info("Initializing subsystems...");

    GameSettings settings;
    GraphicsRegistry graphicsRegistry;
    settings.setWindowDimensions(1900, 1024);

    std::stop_source stopSource;
    std::stop_token stopToken = stopSource.get_token();

    ThreadQueue eventQueue;
    ThreadQueue renderQueue;

    Viewport viewport(settings);

    auto eventLoop = std::make_unique<EventLoop>(&stopToken);
    auto simulator = std::make_unique<Simulator>(renderQueue);
    auto inputHandler = std::make_unique<InputListener>(eventQueue);
    auto renderer = std::make_unique<Renderer>(&stopSource, settings, graphicsRegistry, renderQueue,
                                               eventQueue, viewport);

    eventLoop->registerListener(std::move(simulator));
    eventLoop->registerListener(std::move(inputHandler));
    eventLoop->registerListenerRawPtr(&viewport);
    auto resourceLoader =
        std::make_unique<ResourceLoader>(&stopToken, settings, graphicsRegistry, *(renderer.get()));

    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader", std::move(resourceLoader));
    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));

    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    spdlog::shutdown();
    spdlog::drop_all();

    return 0;
}

// void causeCrash() {
//     int* ptr = nullptr;
//     *ptr = 42;
// }

extern "C" int seh_wrapper()
{
    __try
    {
        std::cout << "Starting the game\n";
        // causeCrash();

        return 1;
        // int runGame();
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        std::cout << "Caught crash via SEH.\n";
        printStackTrace();
        return -1;
    }
}

int main()
{
    // #ifdef _WIN32
    //     return seh_wrapper();
    // #else
    return runGame();
    // #endif
}