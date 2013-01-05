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

#ifndef __CPP_STATEMENT_STATEMENT_FACTORY_SCOMPIL_H_
#define __CPP_STATEMENT_STATEMENT_FACTORY_SCOMPIL_H_

#include "language/c++/statement/compound_statement.h"
#include "language/c++/statement/declaration_statement.h"
#include "language/c++/statement/expression_statement.h"
#include "language/c++/statement/macro_statement.h"
#include "language/c++/statement/statement.h"
#include "language/c++/statement/statement_factory.h"
#include "language/c++/statement/throw_test_statement.h"
#include "language/c++/statement/unary_test_statement.h"
#include "language/c++/statement/variable_declaration_statement.h"

namespace lang
{

namespace cpp
{

// This enumeration class provides namespaced set of constant values.
// It is designed to provide type control over operations with enum values
// without need to wait all compilers you are going to use for this project
// to support C++0x standard. The type control do not prevent the variables
// to be used in switch statements.
class EStatementId
{
public:
    // Default constructor - sets the value to invalid
                         EStatementId();
    // Constructor from an integer value.
    // Note: Avoid using this constructor if possible. It is appropriate
    // only in case of importing value from components that you do not have
    // control over (such as 3rd party libraries or serialization
    // functionality).
    explicit             EStatementId(long value);

    // Returns the integer value of the enum.
    // Note: Avoid using this method if possible. It is appropriate only in
    // case of exporting value to components that you do not have control
    // over (such as 3rd party libraries or serialization functionality).
             long        value       ()                           const;
    // Returns short name for a specified enum value
    static   const char* shortName   (long value);
    // Returns short name of the enum value
             const char* shortName   ()                           const;
    // returns true if the value of the enum is equal to the value of the
    // argument
             bool        operator==  (const EStatementId& rValue) const;
    // returns true if the value of the enum is not equal to the value of
    // the argument
             bool        operator!=  (const EStatementId& rValue) const;

    // static methods for enum class objects
    static const EStatementId invalid                     ();
    static const EStatementId compoundStatement           ();
    static const EStatementId declarationStatement        ();
    static const EStatementId expressionStatement         ();
    static const EStatementId macroStatement              ();
    static const EStatementId statement                   ();
    static const EStatementId throwTestStatement          ();
    static const EStatementId unaryTestStatement          ();
    static const EStatementId variableDeclarationStatement();

    // This enum provides static constants that could be used in a switch
    // statements
    enum
    {
        kInvalid                      = 0,
        kCompoundStatement            = 1,
        kDeclarationStatement         = 2,
        kExpressionStatement          = 3,
        kMacroStatement               = 4,
        kStatement                    = 5,
        kThrowTestStatement           = 6,
        kUnaryTestStatement           = 7,
        kVariableDeclarationStatement = 8,
    };

private:
    // Member variable that holds the enum value
    long mValue;
};

class StatementFactory
{
public:
    static StatementSPtr                    clone                               (const StatementSPtr& object);
    static CompoundStatementSPtr            downcastCompoundStatement           (const StatementSPtr& object);
    static DeclarationStatementSPtr         downcastDeclarationStatement        (const StatementSPtr& object);
    static ExpressionStatementSPtr          downcastExpressionStatement         (const StatementSPtr& object);
    static MacroStatementSPtr               downcastMacroStatement              (const StatementSPtr& object);
    static ThrowTestStatementSPtr           downcastThrowTestStatement          (const StatementSPtr& object);
    static UnaryTestStatementSPtr           downcastUnaryTestStatement          (const StatementSPtr& object);
    static VariableDeclarationStatementSPtr downcastVariableDeclarationStatement(const StatementSPtr& object);
};

}

}

#else // __CPP_STATEMENT_STATEMENT_FACTORY_SCOMPIL_H_

namespace lang
{

namespace cpp
{

// Forward declarations
class StatementFactory;
typedef StatementFactory*                         StatementFactoryRPtr;
typedef boost::shared_ptr<StatementFactory>       StatementFactorySPtr;
typedef boost::shared_ptr<const StatementFactory> StatementFactorySCPtr;
typedef boost::weak_ptr<StatementFactory>         StatementFactoryWPtr;

class EStatementId;
typedef EStatementId*                         EStatementIdRPtr;
typedef boost::shared_ptr<EStatementId>       EStatementIdSPtr;
typedef boost::shared_ptr<const EStatementId> EStatementIdSCPtr;
typedef boost::weak_ptr<EStatementId>         EStatementIdWPtr;

class StatementFactory;

}

}

#endif // __CPP_STATEMENT_STATEMENT_FACTORY_SCOMPIL_H_

