#pragma once

#include <unordered_map>

#include "Zlsnes.h"
#include "IoRegisters.h"


// This is kind of a mix of Observer and Proxy patterns. Is there already a named patern for this?
// The Subject holds pointers to IoRegisterProxy objects that handle reads and writes to a specific address.
// The classes that inherit from IoRegisterProxy will call AttachIoRegister to add themselves to list of known proxies for a specific address.

class IoRegisterProxy
{
protected:
    ~IoRegisterProxy() {}

private:
    virtual bool WriteRegister(EIORegisters ioReg, uint8_t byte) = 0;
    virtual uint8_t ReadRegister(EIORegisters ioReg) const = 0;

    friend class IoRegisterSubject;
};


class IoRegisterSubject
{
public:
    uint8_t *AttachIoRegister(EIORegisters ioReg, IoRegisterProxy *proxy)
    {
        ioRegisterProxies.insert({ioReg, proxy});
        return GetIoRegisterPtr(ioReg);
    }

    // Don't bother with detaching, since everything is destroyed at the same time.
    // There is no case where one proxy will be destroyed and the subject won't.
    // void DetachIoRegister(uint16_t address) {...}

protected:
    virtual ~IoRegisterSubject() {}

    virtual uint8_t *GetIoRegisterPtr(EIORegisters ioReg) = 0;

    bool WriteIoRegisterProxy(EIORegisters ioReg, uint8_t byte)
    {
        auto it = ioRegisterProxies.find(ioReg);
        if (it == ioRegisterProxies.cend())
            return false;

        return it->second->WriteRegister(ioReg, byte);
    }

    uint8_t ReadIoRegisterProxy(EIORegisters ioReg) const
    {
        auto it = ioRegisterProxies.find(ioReg);
        if (it == ioRegisterProxies.cend())
        {
            throw std::range_error(fmt("No registered proxy for reads to 0x04X", ioReg));
        }

        return it->second->ReadRegister(ioReg);
    }

    bool HasIoRegisterProxy(EIORegisters ioReg) const
    {
        return ioRegisterProxies.find(ioReg) != ioRegisterProxies.cend();
    }

private:
    std::unordered_map<EIORegisters, IoRegisterProxy*> ioRegisterProxies;
};