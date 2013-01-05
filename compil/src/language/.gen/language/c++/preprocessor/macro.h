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

#ifndef __CPP_PREPROCESSOR_MACRO_SCOMPIL_H_
#define __CPP_PREPROCESSOR_MACRO_SCOMPIL_H_

#include "language/c++/preprocessor/macro.h"
#include "language/c++/preprocessor/macro_name.h"

namespace lang
{

namespace cpp
{

class Macro
{
public:
    // Default constructor
                                 Macro     ();
    // Destructor
    /*lax*/                      ~Macro    ();

    // Getter method for the data field name
            const MacroNameSPtr& name      ()                          const;
    // Setter method for the data field name
            Macro&               set_name  (const MacroNameSPtr& name);
    // Store operator for the data field name
            Macro&               operator<<(const MacroNameSPtr& name);

private:
    // variable for the data field name
    MacroNameSPtr mName;
};

// Reference store operator for the data field name
const MacroSPtr& operator<<(const MacroSPtr& , const MacroNameSPtr& );

inline MacroSPtr macroRef()
{
    return boost::make_shared<Macro>();
}

}

}

#else // __CPP_PREPROCESSOR_MACRO_SCOMPIL_H_

namespace lang
{

namespace cpp
{

// Forward declarations
class Macro;
typedef Macro*                         MacroRPtr;
typedef boost::shared_ptr<Macro>       MacroSPtr;
typedef boost::shared_ptr<const Macro> MacroSCPtr;
typedef boost::weak_ptr<Macro>         MacroWPtr;

}

}

#endif // __CPP_PREPROCESSOR_MACRO_SCOMPIL_H_

