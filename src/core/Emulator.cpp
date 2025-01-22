#include "Zlsnes.h"
#include "Apu.h"
#include "Cartridge.h"
#include "Cpu.h"
#include "DebuggerInterface.h"
#include "DisplayInterface.h"
#include "Emulator.h"
#include "InfoInterface.h"
#include "Input.h"
#include "Interrupt.h"
#include "Memory.h"
#include "Ppu.h"
#include "Timer.h"


Emulator::Emulator(DisplayInterface *displayInterface, /*AudioInterface *audioInterface,*/ InfoInterface *infoInterface,
                   DebuggerInterface *debuggerInterface/*, GameSpeedSubject *gameSpeedSubject*/) :
    paused(false),
    quit(false),
    displayInterface(displayInterface),
    //audioInterface(audioInterface),
    infoInterface(infoInterface),
    debuggerInterface(debuggerInterface),
    //gameSpeedSubject(gameSpeedSubject),
    apu(NULL),
    buttons(),
    cartridge(),
    cpu(NULL),
    input(NULL),
    interrupts(NULL),
    memory(NULL),
    ppu(NULL),
    timer(NULL),
    enabledLayers{true, true, true, true, true}
{

}


Emulator::~Emulator()
{
    EndEmulation();
}


bool Emulator::LoadRom(const std::string &filename)
{
    EndEmulation();

    if (!cartridge.LoadRom(filename))
    {
        displayInterface->RequestMessageBox("Error loading ROM. See log window for details.");
        return false;
    }

    if (infoInterface)
        infoInterface->UpdateCartridgeInfo(cartridge);

    romFilename = filename;

    quit = false;

    memory = new Memory(infoInterface, debuggerInterface);
    interrupts = new Interrupt();
    timer = new Timer(memory, interrupts);
    // This can't be done in the Memory constructor since Timer doesn't exist yet.
    memory->SetTimer(timer);
    ppu = new Ppu(memory, timer, displayInterface, debuggerInterface);
    memory->SetPpu(ppu);
    input = new Input(memory, timer);
    cpu = new Cpu(memory, timer, interrupts);
    apu = new Apu(memory/*, timer, audioInterface, gameSpeedSubject*/);

    memory->SetCartridge(&cartridge);

    // Set enabled layers based on what the GUI has enabled.
    for (int i = 0; i < 5; i++)
        ppu->ToggleLayer(i, enabledLayers[i]);

    workThread = std::thread(&Emulator::ThreadFunc, this);

    return true;
}


void Emulator::ResetEmulation()
{
    EndEmulation();
    LoadRom(romFilename.c_str());
}


void Emulator::PauseEmulation(bool pause)
{
    paused = pause;
}


void Emulator::EndEmulation()
{
    if (workThread.joinable())
    {
        quit = true;
        workThread.join();
    }
}


void Emulator::ButtonPressed(Buttons::Button button)
{
    uint16_t oldButtonData = buttons.data;

    // Set bit for button.
    buttons.data |= button;

    if (input && buttons.data != oldButtonData)
        input->SetButtons(buttons);
}


void Emulator::ButtonReleased(Buttons::Button button)
{
    uint16_t oldButtonData = buttons.data;

    // Clear bit for button.
    buttons.data &= ~button;

    if (input && buttons.data != oldButtonData)
        input->SetButtons(buttons);
}


void Emulator::ToggleLayer(int layer, bool enabled)
{
    enabledLayers[layer] = enabled;
    if (ppu)
        ppu->ToggleLayer(layer, enabled);
}


void Emulator::SaveState(int slot)
{
    (void)slot;
}


void Emulator::LoadState(int slot)
{
    (void)slot;
}


void Emulator::ThreadFunc()
{
    try
    {
        if (infoInterface)
        {
            infoInterface->SetIoPorts21(memory->GetBytePtr(0x2100));
            infoInterface->SetPpu(ppu);
        }

        if (debuggerInterface)
        {
            debuggerInterface->SetEmulatorObjects(memory, cpu, ppu);
            // SetEmulatorObjects sends a signal that needs to be handled by the gui thread before continuing.
            // TODO: Add proper thread sync later. I don't feel like dealing with this now.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        cpu->Reset();

        while (!quit)
        {
            // Block this thread while state is being saved.
            std::lock_guard<std::mutex> lock(saveStateMutex);

            // Run multiple instruction per mutex lock to reduce the impact of locking the mutex.
            for (int i = 0; i < 100; i++)
            {
                if (!paused && debuggerInterface->ShouldRun(cpu->GetFullPC()))
                {
                    cpu->ProcessOpCode();
                    if (debuggerInterface->GetDebuggingEnabled())
                        debuggerInterface->SetCurrentOp(cpu->GetFullPC());
                    cpu->PrintState();
                    //timer->PrintTimerData();
                }
                else
                {
                    // Sleep to avoid pegging the CPU when paused.
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            //quit = true;
        }

        cartridge.SaveSram();
    }
    catch(const std::exception& e)
    {
        displayInterface->RequestMessageBox(e.what());
    }

    if (infoInterface)
    {
        infoInterface->SetIoPorts21(nullptr);
        infoInterface->SetPpu(nullptr);
    }
    if (debuggerInterface)
        debuggerInterface->SetEmulatorObjects(nullptr, nullptr, nullptr);

    // SetEmulatorObjects sends a signal that needs to be handled by the gui thread before continuing.
    // TODO: Add proper thread sync later. I don't feel like dealing with this now.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    delete apu;
    apu = nullptr;
    delete cpu;
    cpu = nullptr;
    delete ppu;
    ppu = nullptr;
    delete input;
    input = nullptr;
    delete interrupts;
    interrupts = nullptr;
    delete timer;
    timer = nullptr;
    delete memory;
    memory = nullptr;
    cartridge.Reset();
}
