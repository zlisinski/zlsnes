#include <iterator>
#include <fstream>
#include <unistd.h>

#include "Zlsnes.h"
//#include "Audio.h"
#include "Cpu.h"
//#include "DebuggerInterface.h"
//#include "Display.h"
#include "DisplayInterface.h"
#include "Emulator.h"
//#include "InfoInterface.h"
//#include "Input.h"
#include "Memory.h"
//#include "Serial.h"
//#include "Timer.h"


Emulator::Emulator(DisplayInterface *displayInterface/*, AudioInterface *audioInterface, InfoInterface *infoInterface,
                         DebuggerInterface *debuggerInterface, GameSpeedSubject *gameSpeedSubject*/) :
    paused(false),
    quit(false),
    runBootRom(false),
    displayInterface(displayInterface),
    /*audioInterface(audioInterface),
    infoInterface(infoInterface),
    debuggerInterface(debuggerInterface),
    gameSpeedSubject(gameSpeedSubject),
    audio(NULL),
    buttons(),*/
    cpu(NULL),
    /*display(NULL),
    input(NULL),
    interrupts(NULL),*/
    memory(NULL)
    //serial(NULL),
    //timer(NULL)
{

}


Emulator::~Emulator()
{
    EndEmulation();
}


void Emulator::LoadBootRom(const std::string &filename)
{
    if (filename != "")
    {
        std::ifstream file(filename, std::ios::binary);
        std::istreambuf_iterator<char> start(file), end;
        bootRomMemory = std::vector<uint8_t>(start, end);
        runBootRom = true;
    }
    else
    {
        runBootRom = false;
    }
}


bool Emulator::LoadRom(const std::string &filename)
{
    EndEmulation();

    std::ifstream file(filename, std::ios::binary);
    std::istreambuf_iterator<char> start(file), end;
    gameRomMemory = std::vector<uint8_t>(start, end);

    romFilename = filename;
    ramFilename = romFilename + ".ram";

    quit = false;

    memory = new Memory(/*infoInterface, debuggerInterface*/);
    /*interrupts = new Interrupt(memory);
    timer = new Timer(memory, interrupts);
    display = new Display(memory, interrupts, displayInterface, timer);
    input = new Input(memory, interrupts);
    serial = new Serial(memory, interrupts, timer);*/
    cpu = new Cpu(/*interrupts,*/ memory/*, timer*/);
    //audio = new Audio(memory, timer, audioInterface, gameSpeedSubject);

    // This can't be done in the Memory constructor since Timer doesn't exist yet.
    //timer->AttachObserver(memory);

    if (runBootRom)
    {
        memory->SetRomMemory(bootRomMemory, gameRomMemory);
    }
    else
    {
        memory->SetRomMemory(gameRomMemory);
        SetBootState(memory, cpu);
    }
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
    /*// Lock mutex to make the worker thread wait while the state is saved.
    std::lock_guard<std::mutex> lock(saveStateMutex);

    // Open a temp file so that errors writing don't mess up an existing save file.
    char tempFilename[] = "tmpsav.XXXXXX";
    int fd = mkstemp(tempFilename);
    FILE *file = fdopen(fd, "w");
    if (file == NULL)
    {
        LogError("Error opening save state file %s: %s", tempFilename, strerror(errno));
        displayInterface->RequestMessageBox("Error opening save state file");
        return;
    }

    bool success = true;

    // Write header.
    if (!fwrite("ZLGB", 4, 1, file))
        success = false;

    // Write version.
    const uint16_t version = 2;
    if (!fwrite(&version, sizeof(version), 1, file))
        success = false;

    // Write data.
    success &= memory->SaveState(file);
    success &= interrupts->SaveState(file);
    success &= timer->SaveState(file);
    success &= display->SaveState(file);
    success &= input->SaveState(file);
    success &= serial->SaveState(file);
    success &= cpu->SaveState(file);

    fclose(file);

    if (success == false)
    {
        LogError("Error saving state");
        displayInterface->RequestMessageBox("Error saving state");
        //unlink(tempFilename);
        return;
    }

    std::string saveFilename = romFilename + ".sav" + std::to_string(slot);
    if (rename(tempFilename, saveFilename.c_str()))
    {
        LogError("Error renaming temp save state file %s to %s: %s", tempFilename, saveFilename.c_str(), strerror(errno));
        displayInterface->RequestMessageBox("Error renaming temp save state file");
        return;
    }

    LogError("Saved state to %s", saveFilename.c_str());*/
}


void Emulator::LoadState(int slot)
{
    (void)slot;
    /*std::string loadFilename = romFilename + ".sav" + std::to_string(slot);
    FILE *file = fopen(loadFilename.c_str(), "rb");
    if (file == NULL)
    {
        LogError("Error opening save state file %s: %s", loadFilename.c_str(), strerror(errno));
        displayInterface->RequestMessageBox("Error opening save state file");
        return;
    }

    // Read header.
    const int headerLen = 4;
    char header[headerLen + 1] = {0};
    size_t cnt = fread(header, 1, headerLen, file);
    if (cnt != headerLen)
    {
        LogError("Error reading header from save state file. Only read %u bytes.", cnt);
        displayInterface->RequestMessageBox("Error reading header from save state file.");
        fclose(file);
        return;
    }
    if (strcmp(header, "ZLGB"))
    {
        LogError("Save state header doesn't match expected value: %s", header);
        displayInterface->RequestMessageBox("Save state header doesn't match expected value.");
        fclose(file);
        return;
    }

    // Get version.
    uint16_t version = 0;
    if (!fread(&version, 2, 1, file))
    {
        LogError("Error reading version from state file.");
        displayInterface->RequestMessageBox("Error reading version from state file.");
        fclose(file);
        return;
    }
    if (version == 0x3130)
    {
        // The first version of the save state format saved the version as ASCII "01".
        version = 1;
    }

    // Create new objects so if there is an error loading, the current game doesn't get killed.
    Memory *newMemory = new Memory(infoInterface, debuggerInterface);
    Interrupt *newInterrupts = new Interrupt(newMemory);
    Timer *newTimer = new Timer(newMemory, newInterrupts);
    Display *newDisplay = new Display(newMemory, newInterrupts, displayInterface, newTimer);
    Input *newInput = new Input(newMemory, newInterrupts);
    Serial *newSerial = new Serial(newMemory, newInterrupts, newTimer);
    Cpu *newCpu = new Cpu(newInterrupts, newMemory, newTimer);
    Audio *newAudio = new Audio(newMemory, newTimer, audioInterface, gameSpeedSubject);

    // This can't be done in the memory constructor since Timer doesn't exist yet.
    newTimer->AttachObserver(newMemory);

    newMemory->SetRomMemory(gameRomMemory);

    bool success = true;

    // Load data.
    success &= newMemory->LoadState(version, file);
    success &= newInterrupts->LoadState(version, file);
    success &= newTimer->LoadState(version, file);
    success &= newDisplay->LoadState(version, file);
    success &= newInput->LoadState(version, file);
    success &= newSerial->LoadState(version, file);
    success &= newCpu->LoadState(version, file);

    fclose(file);

    if (success == false)
    {
        LogError("Error loading state");
        displayInterface->RequestMessageBox("Error loading state");
        return;
    }

    // End the current game.
    EndEmulation();

    // Replace pointers with new objects.
    memory = newMemory;
    interrupts = newInterrupts;
    timer = newTimer;
    display = newDisplay;
    input = newInput;
    serial = newSerial;
    cpu = newCpu;
    audio = newAudio;

    // Start emulation.
    paused = false;
    quit = false;
    workThread = std::thread(&Emulator::ThreadFunc, this);

    LogError("Loaded save file %s", loadFilename.c_str());*/
}


void Emulator::ThreadFunc()
{
    try
    {
        /*if (infoInterface)
            infoInterface->SetMemory(memory->GetBytePtr(0));

        if (debuggerInterface)
            debuggerInterface->SetEmulatorObjects(memory, cpu, interrupts);*/

        cpu->PrintState();
        quit = true;

        while (!quit)
        {
            // Block this thread while state is being saved.
            std::lock_guard<std::mutex> lock(saveStateMutex);

            // Run multiple instruction per mutex lock to reduce the impact of locking the mutex.
            for (int i = 0; i < 100; i++)
            {
                if (!paused /*&& debuggerInterface->ShouldRun(cpu->reg.pc)*/)
                {
                    cpu->ProcessOpCode();
                    //if (debuggerInterface->GetDebuggingEnabled())
                    //    debuggerInterface->SetCurrentOp(cpu->reg.pc);
                    cpu->PrintState();
                    //timer->PrintTimerData();
                }
                else
                {
                    // Sleep to avoid pegging the CPU when paused.
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }

        //memory->SaveRam(ramFilename);
    }
    catch(const std::exception& e)
    {
        displayInterface->RequestMessageBox(e.what());
    }

    /*if (infoInterface)
        infoInterface->SetMemory(NULL);
    if (debuggerInterface)
        debuggerInterface->SetEmulatorObjects(NULL, NULL, NULL);*/

    //delete audio;
    //audio = NULL;
    delete cpu;
    cpu = NULL;
    /*delete display;
    display = NULL;
    delete input;
    input = NULL;
    delete interrupts;
    interrupts = NULL;
    delete serial;
    serial = NULL;
    delete timer;
    timer = NULL;*/
    delete memory;
    memory = NULL;
}


void Emulator::SetBootState(Memory *memory, Cpu *cpu)
{
    (void)memory;
    (void)cpu;
    // Set state to what it would be after running the boot ROM.

    /*cpu->reg.a = 0x01;
    cpu->reg.f = 0xB0;
    cpu->reg.bc = 0x0013;
    cpu->reg.de = 0x00D8;
    cpu->reg.hl = 0x014D;
    cpu->reg.sp = 0xFFFE;
    cpu->reg.pc = 0x0100;

    memory->WriteByte(eRegTIMA, 0x00);
    memory->WriteByte(eRegTMA, 0x00);
    memory->WriteByte(eRegTAC, 0x00);

    memory->WriteByte(eRegNR10, 0x80);
    memory->WriteByte(eRegNR11, 0xBF);
    memory->WriteByte(eRegNR12, 0xF3);
    memory->WriteByte(eRegNR14, 0x3F); // This should be 0xBF, but don't set Initialize bit, otherwise a sound will play at startup.
    memory->WriteByte(eRegNR50, 0x77);
    memory->WriteByte(eRegNR51, 0xF3);
    memory->WriteByte(eRegNR52, 0xF1);

    memory->WriteByte(eRegLCDC, 0x91);
    memory->WriteByte(eRegSCY, 0x00);
    memory->WriteByte(eRegSCX, 0x00);
    memory->WriteByte(eRegLYC, 0x00);
    memory->WriteByte(eRegBGP, 0xFC);
    memory->WriteByte(eRegOBP0, 0xFF);
    memory->WriteByte(eRegOBP1, 0xFF);
    memory->WriteByte(eRegWY, 0x00);
    memory->WriteByte(eRegWX, 0x00);

    memory->WriteByte(eRegIE, 0x00);*/
}