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
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#ifndef __CPP_PREPROCESSOR_DECLARATION_MACRO_PARAMETER_SCOMPIL_H_
#define __CPP_PREPROCESSOR_DECLARATION_MACRO_PARAMETER_SCOMPIL_H_

#include "language/c++/declaration/declaration.h"
#include "language/c++/preprocessor/declaration_macro_parameter.h"
#include "language/c++/preprocessor/macro_parameter.h"

namespace lang
{

namespace cpp
{

class DeclarationMacroParameter : public MacroParameter
{
public:
    // Default constructor
                                          DeclarationMacroParameter ();
    // Destructor
    virtual                               ~DeclarationMacroParameter();

    // Downcast method is syntactic sugar for boost::static_pointer_cast.
    // Note that it does not provide any type checks. Use it on your own
    // risk.
    static  DeclarationMacroParameterSPtr downcast                  (const MacroParameterSPtr& object);

    // Identifier for the objects from DeclarationMacroParameter class.
    // Note: it is not defined in the respective cpp file. Instead it is
    // defined in the factory class together with all the other identifiers
    // of the other class objects. This allows all identifiers to be
    // maintained from a single place, which reduces the risk of value
    // collisions
    static  EMacroParameterId             staticMacroParameterId    ();
    // This virtual method provides runtime object identification based on
    // the polymorphic behavior of the virtual methods. Allows having a
    // RTTI like mechanism significantly cheaper than the RTTI provided by
    // the compilers themselves.
    virtual EMacroParameterId             runtimeMacroParameterId   ()                                   const;

    // Getter method for the data field declaration
            const DeclarationSPtr&        declaration               ()                                   const;
    // Setter method for the data field declaration
            DeclarationMacroParameter&    set_declaration           (const DeclarationSPtr& declaration);
    // Store operator for the data field declaration
            DeclarationMacroParameter&    operator<<                (const DeclarationSPtr& declaration);

private:
    // variable for the data field declaration
    DeclarationSPtr mDeclaration;
};

// Reference store operator for the data field declaration
const DeclarationMacroParameterSPtr& operator<<(const DeclarationMacroParameterSPtr& , const DeclarationSPtr& );

inline DeclarationMacroParameterSPtr declarationMacroParameterRef()
{
    return boost::make_shared<DeclarationMacroParameter>();
}

}

}

#else // __CPP_PREPROCESSOR_DECLARATION_MACRO_PARAMETER_SCOMPIL_H_

namespace lang
{

namespace cpp
{

// Forward declarations
class DeclarationMacroParameter;
typedef DeclarationMacroParameter*                         DeclarationMacroParameterRPtr;
typedef boost::shared_ptr<DeclarationMacroParameter>       DeclarationMacroParameterSPtr;
typedef boost::shared_ptr<const DeclarationMacroParameter> DeclarationMacroParameterSCPtr;
typedef boost::weak_ptr<DeclarationMacroParameter>         DeclarationMacroParameterWPtr;

class EMacroParameterId;

}

}

#endif // __CPP_PREPROCESSOR_DECLARATION_MACRO_PARAMETER_SCOMPIL_H_

