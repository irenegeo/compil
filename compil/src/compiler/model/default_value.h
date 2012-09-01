// CompIL - Component Interface Language
// Copyright 2011 George Georgiev.  All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// * The name of George Georgiev can not be used to endorse or
// promote products derived from this software without specific prior
// written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: george.georgiev@hotmail.com (George Georgiev)
// 

// Boost C++ Smart Pointers
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
// Standard Template Library
#include <string>

#ifndef __GENERATOR_SELF_COMPILER_MODEL_DEFAULT_VALUE_COMPIL_H_
#define __GENERATOR_SELF_COMPILER_MODEL_DEFAULT_VALUE_COMPIL_H_

#include "default_value.h"
#include "object.h"

namespace compil
{

class DefaultValue : public Object
{
public:
    // Default constructor
                               DefaultValue    ();
    // Destructor
    virtual                    ~DefaultValue   ();

    // Downcast method is syntactic sugar for boost::static_pointer_cast.
    // Note that it does not provide any type checks. Use it on your own
    // risk.
    static  DefaultValueSPtr   downcast        (const ObjectSPtr& pObject);

    // Returns true if every required field is initialized.
    virtual bool               isInitialized   ()                          const;

    // Identifier for the objects from DefaultValue class.
    // Note: it is not defined in the respective cpp file. Instead it is
    // defined in the factory class together with all the other identifiers
    // of the other class objects. This allows all identifiers to be
    // maintained from a single place, which reduces the risk of value
    // collisions
    static  EObjectId          staticObjectId  ();
    // This virtual method provides runtime object identification based on
    // the polymorphic behavior of the virtual methods. Allows having a
    // RTTI like mechanism significantly cheaper than the RTTI provided by
    // the compilers themselves.
    virtual EObjectId          runtimeObjectId ()                          const;

    // Getter method for the data field optional
            bool               optional        ()                          const;
    // Returns true if the data field value optional was changed from its
    // default value false
            bool               changed_optional()                          const;
    // Returns the default value false of the field optional
    static  bool               default_optional();
    // Setter method for the data field optional
            void               set_optional    (bool optional);
    // Resets the data field optional to its default value false
            void               reset_optional  ();

    // Getter method for the data field value
            const std::string& value           ()                          const;
    // Returns true if the data field value was set and could be considered
    // valid
            bool               valid_value     ()                          const;
    // Setter method for the data field value
            void               set_value       (const std::string& value);
            std::string&       mutable_value   ();
    // Erases the required data field value. Object can not be instantiated
    // before the field data is set again
            void               erase_value     ();

private:
    // Returns unique bitmask value for the field optional
    static int bitmask_optional();
    // Returns unique bitmask value for the field value
    static int bitmask_value   ();

    // Stores availability information for the fields
    int         mBits;

    // variable for the data field optional
    bool        mOptional;
    // variable for the data field value
    std::string mValue;
};

}

#else // __GENERATOR_SELF_COMPILER_MODEL_DEFAULT_VALUE_COMPIL_H_

namespace compil
{

// Forward declarations
class DefaultValue;
typedef DefaultValue*                         DefaultValueRPtr;
typedef boost::shared_ptr<DefaultValue>       DefaultValueSPtr;
typedef boost::shared_ptr<const DefaultValue> DefaultValueSCPtr;
typedef boost::weak_ptr<DefaultValue>         DefaultValueWPtr;

class EObjectId;

}

#endif // __GENERATOR_SELF_COMPILER_MODEL_DEFAULT_VALUE_COMPIL_H_
