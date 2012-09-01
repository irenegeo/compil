// Boost C++ Utility
#include <boost/assert.hpp>

#include "initialization.h"
// Standard C Library
#include <stddef.h>

int Initialization::bitmask_namespace_()
{
    return 0x1;
}

int Initialization::bitmask_name()
{
    return 0x2;
}

int Initialization::bitmask_value()
{
    return 0x4;
}

Initialization::Builder::Builder()
        : mpObject(new Initialization())
{
}

Initialization::Builder::Builder(const Initialization& object)
        : mpObject(new Initialization())
{
    *(Initialization*)mpObject = object;
}

Initialization::Builder::Builder(InitializationRPtr pObject)
        : mpObject(pObject)
{
}

Initialization::Builder::~Builder()
{
    delete (InitializationRPtr)mpObject;
    mpObject = NULL;
}

const Initialization& Initialization::Builder::build() const
{
    BOOST_ASSERT(mpObject->isInitialized());
    return *(InitializationRPtr)mpObject;
}

InitializationSPtr Initialization::Builder::finalize()
{
    BOOST_ASSERT(mpObject->isInitialized());
    InitializationRPtr objectRPtr = (InitializationRPtr)mpObject;
    mpObject = NULL;
    return InitializationSPtr(objectRPtr);
}

Initialization::Initialization()
        : mBits(0)
{
}

Initialization::~Initialization()
{
}

bool Initialization::isInitialized() const
{
    if (!valid_name()) return false;
    if (!valid_value()) return false;
    return true;
}

const Namespace& Initialization::namespace_() const
{
    BOOST_ASSERT(exist_namespace_());
    return mNamespace_;
}

bool Initialization::exist_namespace_() const
{
    return (mBits & bitmask_namespace_()) != 0;
}

Initialization::Builder& Initialization::Builder::set_namespace_(const Namespace& namespace_)
{
    mpObject->mNamespace_  = namespace_;
    mpObject->mBits       |= bitmask_namespace_();
    return *this;
}

Namespace& Initialization::Builder::mutable_namespace_()
{
    mpObject->mBits |= bitmask_namespace_();
    return mpObject->mNamespace_;
}

void Initialization::Builder::clear_namespace_()
{
    mpObject->mNamespace_  =  Namespace();
    mpObject->mBits       &= ~bitmask_namespace_();
}

const std::string& Initialization::name() const
{
    BOOST_ASSERT(valid_name());
    return mName;
}

bool Initialization::valid_name() const
{
    return (mBits & bitmask_name()) != 0;
}

Initialization::Builder& Initialization::Builder::set_name(const std::string& name)
{
    mpObject->mName  = name;
    mpObject->mBits |= bitmask_name();
    return *this;
}

std::string& Initialization::Builder::mutable_name()
{
    mpObject->mBits |= bitmask_name();
    return mpObject->mName;
}

void Initialization::Builder::erase_name()
{
    mpObject->mName.clear();
    mpObject->mBits &= ~bitmask_name();
}

const std::string& Initialization::value() const
{
    BOOST_ASSERT(valid_value());
    return mValue;
}

bool Initialization::valid_value() const
{
    return (mBits & bitmask_value()) != 0;
}

Initialization::Builder& Initialization::Builder::set_value(const std::string& value)
{
    mpObject->mValue  = value;
    mpObject->mBits  |= bitmask_value();
    return *this;
}

std::string& Initialization::Builder::mutable_value()
{
    mpObject->mBits |= bitmask_value();
    return mpObject->mValue;
}

void Initialization::Builder::erase_value()
{
    mpObject->mValue.clear();
    mpObject->mBits &= ~bitmask_value();
}

InitializationSPtr CreateInitialization(const std::string& name, const std::string& value)
{
    Initialization::Builder builder;
    builder.set_name(name);
    builder.set_value(value);
    return builder.finalize();
}

InitializationSPtr CreateInitialization(const Namespace& namespace_, const std::string& name, const std::string& value)
{
    Initialization::Builder builder;
    builder.set_namespace_(namespace_);
    builder.set_name(name);
    builder.set_value(value);
    return builder.finalize();
}
