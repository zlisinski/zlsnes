#include "Zlsnes.h"
//#include "Audio.h"
#include "Cartridge.h"
#include "Cpu.h"
#include "DebuggerInterface.h"
#include "DisplayInterface.h"
#include "Emulator.h"
#include "InfoInterface.h"
//#include "Input.h"
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
    /*gameSpeedSubject(gameSpeedSubject),
    audio(NULL),
    buttons(),*/
    cartridge(),
    cpu(NULL),
    /*input(NULL),
    interrupts(NULL),*/
    memory(NULL),
    ppu(NULL),
    timer(NULL)
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
    ramFilename = romFilename + ".ram";

    quit = false;

    memory = new Memory(infoInterface/*, debuggerInterface*/);
    //interrupts = new Interrupt(memory);
    timer = new Timer(/*memory, interrupts*/);
    ppu = new Ppu(memory/*, interrupts, displayInterface, timer*/);
    //input = new Input(memory, interrupts);
    cpu = new Cpu(/*interrupts,*/ memory, timer);
    //audio = new Audio(memory, timer, audioInterface, gameSpeedSubject);

    // This can't be done in the Memory constructor since Timer doesn't exist yet.
    memory->SetTimer(timer);

    memory->SetCartridge(&cartridge);
    SetBootState(memory, cpu);
    //memory->LoadRam(ramFilename);

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


/*void Emulator::ButtonPressed(Buttons::Button button)
{
    uint8_t oldButtonData = buttons.data;

    // Set bit for button.
    buttons.data |= button;

    if (input && buttons.data != oldButtonData)
        input->SetButtons(buttons);
}


void Emulator::ButtonReleased(Buttons::Button button)
{
    uint8_t oldButtonData = buttons.data;

    // Clear bit for button.
    buttons.data &= ~button;

    if (input && buttons.data != oldButtonData)
        input->SetButtons(buttons);
}*/


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
            infoInterface->SetMemory(memory->GetBytePtr(0));

        if (debuggerInterface)
            debuggerInterface->SetEmulatorObjects(memory, cpu/*, interrupts*/);

        // Immediately quit, for now.
        //cpu->PrintState();
        //quit = true;

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

        //memory->SaveRam(ramFilename);
    }
    catch(const std::exception& e)
    {
        displayInterface->RequestMessageBox(e.what());
    }

    if (infoInterface)
        infoInterface->SetMemory(NULL);
    if (debuggerInterface)
        debuggerInterface->SetEmulatorObjects(NULL, NULL/*, NULL*/);

    //delete audio;
    //audio = NULL;
    delete cpu;
    cpu = NULL;
    delete ppu;
    ppu = NULL;
    /*delete input;
    input = NULL;
    delete interrupts;
    interrupts = NULL;*/
    delete timer;
    timer = NULL;
    delete memory;
    memory = NULL;
    cartridge.Reset();
}


void Emulator::SetBootState(Memory *memory, Cpu *cpu)
{
    (void)memory;
    (void)cpu;
}