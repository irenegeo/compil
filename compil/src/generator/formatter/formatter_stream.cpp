// CompIL - Component Interface Language
// Copyright 2011 George Georgiev.  All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * The name of George Georgiev can not be used to endorse or 
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

#include "formatter_stream.h"

#include "c++/statement/statement_factory.h"

#include "all/list.h"
#include "all/scope.h"

using namespace lang;
using namespace lang::cpp;

FormatterStream::FormatterStream(const FormatterConfigurationPtr& formatterConfiguration,
                                 const AlignerConfigurationPtr& alignerConfiguration)
    : mAligner(alignerConfiguration)
    , mConfiguration(formatterConfiguration)
{
}

FormatterStream::~FormatterStream()
{
}

std::string FormatterStream::str()
{
    return mAligner.str();
}

FormatterStream& operator<<(FormatterStream& stream, const CompoundStatementSPtr& compoundStatement)
{
    Scope scope;
    scope << Scope::ESquiggles::brackets();
    
    const std::vector<StatementSPtr>& statements = compoundStatement->statements();
    for (std::vector<StatementSPtr>::const_iterator it = statements.begin(); it != statements.end(); ++it)
    {
        const StatementSPtr& statement = *it;
        
        FormatterStream formatter(stream.mConfiguration, stream.mAligner.mConfiguration);
        formatter << statement;
        scope << formatter.str();
    }
    
    stream.mAligner << scope;
    return stream << compoundStatement->close();
}

FormatterStream& operator<<(FormatterStream& stream, const DeclarationStatementSPtr& statement)
{
    stream.mAligner << statement->typeId().value();
    stream.mAligner << " ";
    stream.mAligner << statement->typeName();
    return stream << statement->close();
}

FormatterStream& operator<<(FormatterStream& stream, const MacroStatementSPtr& macro)
{
    stream.mAligner << macro->name().value();
    
    List list;
    list << List::ESquiggles::parentheses();
    list << List::EDelimiter::comma();
    
    const std::vector<MacroParameter>& parameters = macro->parameters();
    for (std::vector<MacroParameter>::const_iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        const MacroParameter& parameter = *it;
        list << parameter.value();
    }
    
    stream.mAligner << list;
    return stream << macro->close();
}

FormatterStream& operator<<(FormatterStream& stream, const StatementSPtr& statement)
{
    if (statement->runtimeStatementId() == CompoundStatement::staticStatementId())
        return stream << CompoundStatement::downcast(statement);
    if (statement->runtimeStatementId() == DeclarationStatement::staticStatementId())
        return stream << DeclarationStatement::downcast(statement);
    if (statement->runtimeStatementId() == MacroStatement::staticStatementId())
        return stream << MacroStatement::downcast(statement);
         
    return stream;
}

FormatterStream& operator<<(FormatterStream& stream, const Statement::EClose& close)
{
    if (close == Statement::EClose::yes())
        stream.mAligner << ";";
    return stream;
}
