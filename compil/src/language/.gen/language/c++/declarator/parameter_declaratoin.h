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

#ifndef __CPP_DECLARATOR_PARAMETER_DECLARATOIN_SCOMPIL_H_
#define __CPP_DECLARATOR_PARAMETER_DECLARATOIN_SCOMPIL_H_

#include "language/c++/declarator/declarator.h"
#include "language/c++/declarator/parameter_declaratoin.h"

namespace lang
{

namespace cpp
{

class ParameterDeclaration : public Declarator
{
public:
    // Default constructor
                                     ParameterDeclaration ();
    // Destructor
    virtual                          ~ParameterDeclaration();

    // Downcast method is syntactic sugar for boost::static_pointer_cast.
    // Note that it does not provide any type checks. Use it on your own
    // risk.
    static  ParameterDeclarationSPtr downcast             (const DeclaratorSPtr& object);
};


inline ParameterDeclarationSPtr parameterDeclarationRef()
{
    return boost::make_shared<ParameterDeclaration>();
}

}

}

#else // __CPP_DECLARATOR_PARAMETER_DECLARATOIN_SCOMPIL_H_

namespace lang
{

namespace cpp
{

// Forward declarations
class ParameterDeclaration;
typedef ParameterDeclaration*                         ParameterDeclarationRPtr;
typedef boost::shared_ptr<ParameterDeclaration>       ParameterDeclarationSPtr;
typedef boost::shared_ptr<const ParameterDeclaration> ParameterDeclarationSCPtr;
typedef boost::weak_ptr<ParameterDeclaration>         ParameterDeclarationWPtr;

}

}

#endif // __CPP_DECLARATOR_PARAMETER_DECLARATOIN_SCOMPIL_H_

