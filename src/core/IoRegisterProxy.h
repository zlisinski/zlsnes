#pragma once

#include <iomanip>
#include <sstream>
#include <unordered_map>

#include "Zlsnes.h"

// This is kind of a mix of Observer and Proxy patterns. Is there already a named patern for this?
// The Subject holds pointers to IoRegisterProxy objects that handle reads and writes to a specific address.
// The classes that inherit from IoRegisterProxy will call AttachIoRegister to add themselves to list of known proxies for a specific address.

class IoRegisterProxy
{
protected:
    ~IoRegisterProxy() {}

private:
    virtual bool WriteByte(uint32_t address, uint8_t byte) = 0;
    virtual uint8_t ReadByte(uint32_t address) const = 0;

    friend class IoRegisterSubject;
};


class IoRegisterSubject
{
public:
    uint8_t *AttachIoRegister(uint32_t address, IoRegisterProxy *proxy)
    {
        ioRegisterProxies.insert({address, proxy});
        return GetBytePtr(address);
    }

    // Don't bother with detaching, since everything is destroyed at the same time.
    // There is no case where one proxy will be destroyed and the subject won't.
    // void DetachIoRegister(uint16_t address) {...}

protected:
    virtual ~IoRegisterSubject() {}

    virtual uint8_t *GetBytePtr(uint32_t address) = 0;

    bool WriteIoRegisterProxy(uint32_t address, uint8_t byte)
    {
        auto it = ioRegisterProxies.find(address);
        if (it == ioRegisterProxies.cend())
            return false;

        return it->second->WriteByte(address, byte);
    }

    uint8_t ReadIoRegisterProxy(uint32_t address) const
    {
        auto it = ioRegisterProxies.find(address);
        if (it == ioRegisterProxies.cend())
        {
            std::stringstream ss;
            ss << "No registered proxy for reads to 0x" << std::hex << std::setw(6) << std::setfill('0') << address;
            throw std::range_error(ss.str());
        }

        return it->second->ReadByte(address);
    }

    bool HasIoRegisterProxy(uint32_t address) const
    {
        return ioRegisterProxies.find(address) != ioRegisterProxies.cend();
    }

private:
    std::unordered_map<uint32_t, IoRegisterProxy*> ioRegisterProxies;
};