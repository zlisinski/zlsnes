#pragma once

#include <unordered_map>

#include "Zlsnes.h"
#include "IoRegisters.h"


// This is kind of a mix of Observer and Proxy patterns. Is there already a named patern for this?
// The Subject holds pointers to IoRegisterProxy objects that own IO ports and handle reads and writes to that port.
// The classes that inherit from IoRegisterProxy will call RequestOwnership to add themselves to list of known proxies for a specific address.

class IoRegisterProxy
{
protected:
    ~IoRegisterProxy() {}

private:
    virtual bool WriteRegister(EIORegisters ioReg, uint8_t byte) = 0;
    virtual uint8_t ReadRegister(EIORegisters ioReg) = 0;

    friend class IoRegisterSubject;
};


class IoRegisterSubject
{
public:
    uint8_t &RequestOwnership(EIORegisters ioReg, IoRegisterProxy *proxy)
    {
        if (ioRegisterProxies.count(ioReg) != 0)
            throw std::runtime_error(fmt("IO port %04X is already owned", ioReg));
        ioRegisterProxies.insert({ioReg, proxy});
        return GetIoRegisterRef(ioReg);
    }

    uint8_t *RequestOwnershipBlock(uint16_t start, uint16_t size, IoRegisterProxy *proxy)
    {
        for (uint16_t i = start; i < start + size; i++)
        {
            if (ioRegisterProxies.count(static_cast<EIORegisters>(i)) != 0)
                throw std::runtime_error(fmt("IO port %04X is already owned", i));
            ioRegisterProxies.insert({static_cast<EIORegisters>(i), proxy});
        }
        return GetBytePtr(start);
    }

    // Don't bother with detaching, since everything is destroyed at the same time.
    // There is no case where one proxy will be destroyed and the subject won't.
    // void DetachIoRegister(uint16_t address) {...}

protected:
    virtual ~IoRegisterSubject() {}

    virtual uint8_t &GetIoRegisterRef(EIORegisters ioReg) = 0;
    virtual uint8_t *GetBytePtr(uint32_t addr) = 0;

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