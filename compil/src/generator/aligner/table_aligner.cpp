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
#include "table_aligner.h"
#include "line_aligner.h"

#include <boost/algorithm/string.hpp>

#include <assert.h>

namespace compil
{

TableCeil::TableCeil()
{
}

TableCeil::~TableCeil()
{
}

TableAligner::TableAligner(const AlignerConfigurationPtr& pConfig) 
	: Aligner(pConfig)
{
}

TableAligner::~TableAligner()
{
}

bool TableAligner::isEmpty() const
{
	if (mTable.size() == 0) return true;
	if (mTable[0].size() == 0) return true;
	return false;
}

struct dereferencable_bool
{
    dereferencable_bool()
    {
    }
    
    dereferencable_bool(bool b) : val(b)
    {
    }

    operator bool() 
    {
        return val;
    }
private:
    bool val;
};

std::string TableAligner::str(int indent) const
{
	std::vector<size_t> width;
    std::vector<dereferencable_bool> vSplit(mTable.size(), false);
	std::vector<Row>::const_iterator rit;
    size_t r = 0;
	for (rit = mTable.begin(); rit != mTable.end(); ++rit, ++r)
	{
		const Row& row = *rit;
        if (row.mType != Row::normal) continue;
        
        size_t row_length = 0;
        
		for (size_t i = 0; i < row.size(); ++i)
		{
			const TableCeilSPtr& pTableCeil = row[i];
			size_t len = pTableCeil->str().length();
            row_length += len;
			if (width.size() <= i)
				width.push_back(0);
        }
        
        bool too_long = row_length > (size_t)mpConfiguration->mColumnWidthLimit;
        vSplit[r] = too_long;
                
		for (size_t i = 0; i < row.size(); ++i)
		{
			const TableCeilSPtr& pTableCeil = row[i];
            size_t len = pTableCeil->str().length();
            if (too_long && pTableCeil->mOptionalNewLineColumns.size() > 0)
            {
                size_t plen = pTableCeil->mOptionalNewLineColumns[0];
                for (size_t j = 1; j < pTableCeil->mOptionalNewLineColumns.size(); ++j)
                {
                    size_t clen = pTableCeil->mOptionalNewLineColumns[j] -
                                  pTableCeil->mOptionalNewLineColumns[j - 1];
                    if (plen < clen)
                        plen = clen;
                }
                size_t clen = len - *pTableCeil->mOptionalNewLineColumns.rbegin();
                if (plen < clen)
                    plen = clen;
                    
                len = plen;
            }
            if (width[i] < len)
                width[i] = len;
		}
	}

	std::ostringstream out;
    r = 0;
	for (rit = mTable.begin(); rit != mTable.end(); ++rit, ++r)
	{
		LineAlignerPtr aligner(new LineAligner(mpConfiguration));
		const Row& row = *rit;
        if (row.mType == Row::comment)
        {
            const OSStreamPtr& pStream = *row.begin();
            out << aligner->comment(indent + row.mIndent, pStream->str());
        }
        else
        {
            size_t length = 0;
            size_t pos = 0;
            int i = 0;
            std::vector<TableCeilSPtr>::const_iterator sit;
            for (sit = row.begin(); sit != row.end(); ++sit, ++i)
            {
                *aligner << std::string(pos - length, ' ');
                const TableCeilSPtr& pTableCeil = *sit;
                std::string field = pTableCeil->str();
                
                if (vSplit[r] && (pTableCeil->mOptionalNewLineColumns.size() > 0))
                {
                    std::string pfield = field.substr(0, pTableCeil->mOptionalNewLineColumns[0]);
                    boost::trim(pfield);
                    *aligner << pfield;
                    out << aligner->str(indent);
                    aligner.reset(new LineAligner(mpConfiguration));
                    *aligner << std::string(pos, ' ');
                    for (size_t j = 1; j < pTableCeil->mOptionalNewLineColumns.size(); ++j)
                    {
                        pfield = field.substr(pTableCeil->mOptionalNewLineColumns[j - 1], 
                                              pTableCeil->mOptionalNewLineColumns[j] - 
                                              pTableCeil->mOptionalNewLineColumns[j - 1]);
                        boost::trim(pfield);
                        *aligner << pfield;
                        out << aligner->str(indent + row.mIndent);
                        aligner.reset(new LineAligner(mpConfiguration));
                        *aligner << std::string(pos, ' ');
                    }
                    pfield = field.substr(*pTableCeil->mOptionalNewLineColumns.rbegin());
                    boost::trim(pfield);
                    *aligner << pfield;
                    length = pos + pfield.length();
                }
                else
                {
                    boost::trim(field);
                    *aligner << field;
                    length = pos + field.length();
                }
                pos += width[i];
            }
            out << aligner->str(indent + row.mIndent);
        }
	}
	return out.str();
}

TableCeil& TableAligner::ceil() 
{
	assert(pCurrent);
	return *pCurrent;
}

void TableAligner::newColumn()
{
	pCurrent.reset(new TableCeil());
	mTable.rbegin()->push_back(pCurrent);
}

void TableAligner::newRow(Row::Type type, int indent)
{
	mTable.push_back(Row(type, indent));
	newColumn();
}

TableAligner& operator<<(TableAligner& aligner, const TableAligner::col&)
{
	aligner.newColumn();
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const TableAligner::row&)
{
	aligner.newRow();
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const TableAligner::row_line& row_line)
{
    aligner.newRow(TableAligner::Row::line, row_line.mIndent);
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const TableAligner::row_comment&)
{
    aligner.newRow(TableAligner::Row::comment);
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const TableAligner::optional_new_line&)
{
    aligner.ceil().mOptionalNewLineColumns.push_back(aligner.ceil().str().size());
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Aligner::FunctionSpace&)
{
    if (aligner.mpConfiguration->mFunctionSpace)
        aligner << ' ';
    aligner << TableAligner::col();
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Aligner::FunctionDefinitionTypeSpace&)
{
    if (aligner.mpConfiguration->mFunctionTypeOnNewLine)
        aligner << TableAligner::row();
    else
        aligner << ' ';
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Declaration& declaration)
{
    aligner << declaration.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const DecoratedType& decoratedType)
{
    if (decoratedType.exist_declaration())
    if (!decoratedType.declaration().isVoid())
        aligner << decoratedType.declaration() << ' ';
    if (!decoratedType.type().isVoid())
        aligner << decoratedType.type();
        
	if (aligner.mpConfiguration->mDecoration == AlignerConfiguration::part_of_the_type)
	{
        if (decoratedType.exist_decoration())
        if (!decoratedType.decoration().isVoid())
            aligner << decoratedType.decoration();
        if (!decoratedType.type().isVoid())
            aligner << ' ';
        if (decoratedType.aligned())
            aligner << TableAligner::col();
	}
	else if (aligner.mpConfiguration->mDecoration == AlignerConfiguration::part_of_the_name)
	{
        if (!decoratedType.type().isVoid())
            aligner << ' ';
        if (decoratedType.aligned())
            aligner << TableAligner::col();
        if (!decoratedType.decoration().isVoid())
            aligner << decoratedType.decoration();
	}
	else if (aligner.mpConfiguration->mDecoration == AlignerConfiguration::next_to_the_name)
	{
        if (!decoratedType.type().isVoid())
            aligner << ' ';
        if (decoratedType.aligned())
            aligner << TableAligner::col();
        if (!decoratedType.decoration().isVoid())
            aligner << decoratedType.decoration();
        if (decoratedType.aligned())
            aligner << TableAligner::col();
	}
    else
    {
        assert(false && "unknown decoration");
    }
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Decoration& decoration)
{
    aligner << decoration.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Argument& argument)
{
    if (!argument.mDecoratedType.isVoid())
        aligner << argument.mDecoratedType;
    aligner << argument.mValue;
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Function& function)
{
    if (!function.mModifier.isVoid())
        aligner << function.mModifier << ' ';

    aligner << TableAligner::col();

    if (!function.mReturnType.isVoid())
        aligner << function.mReturnType;
    
    if (!function.mNamespace.isVoid())
        aligner << function.mNamespace << "::";
        
    assert(!function.mName.isVoid() || function.mCastOperator);

    if (!function.mName.isVoid())
        aligner << function.mName;

    if (function.mCastOperator)
        aligner << "operator " 
                << function.mCastOperator.mDecoratedType
                << TableAligner::col();

    aligner << Aligner::FunctionSpace();
    
    aligner << "("
            << TableAligner::col();
    if (function.mvArgument.size() > 0)
        aligner << function.mvArgument[0];
    for (size_t i = 1; i < function.mvArgument.size(); ++i)
        aligner << ","
                << TableAligner::optional_new_line()
                << " "
                << function.mvArgument[i];
    aligner << ")";
    
    if (!function.mDeclaration.isVoid())
        aligner << ' ' << TableAligner::col() << function.mDeclaration;
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const FunctionName& functionName)
{
    aligner << functionName.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Constructor& constructor)
{
    assert(constructor);
    
    if (!constructor.mModifier.isVoid())
        aligner << constructor.mModifier << ' ';
        
    aligner << TableAligner::col();
    aligner << TableAligner::col();
    if (aligner.mpConfiguration->mDecoration == AlignerConfiguration::next_to_the_name)
        aligner << TableAligner::col();
    aligner << constructor.mType;
    aligner << Aligner::FunctionSpace();
    aligner << "("
            << TableAligner::col();
    if (constructor.mvArgument.size() > 0)
        aligner << constructor.mvArgument[0];
    for (size_t i = 1; i < constructor.mvArgument.size(); ++i)
        aligner << ", " << constructor.mvArgument[i];
    aligner << ")";
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Destructor& destructor)
{
    assert(destructor);
    
    if (!destructor.mModifier.isVoid())
        aligner << destructor.mModifier << ' ';
        
    aligner << TableAligner::col();
    aligner << TableAligner::col();
    aligner << tilde;
    if (aligner.mpConfiguration->mDecoration == AlignerConfiguration::next_to_the_name)
        aligner << TableAligner::col();
    aligner << destructor.mType;
    aligner << Aligner::FunctionSpace();
    aligner << "("
            << TableAligner::col()
            << ")";
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Namespace& namespace_)
{
    aligner << namespace_.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Modifier& modifier)
{
    aligner << modifier.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const FunctionCall& function)
{
    if (!function.mNamespace.isVoid())
        aligner << function.mNamespace << "::";
    aligner << function.mName << Aligner::FunctionSpace();
    aligner << "(";
    if (function.mvArgument.size() > 0)
        aligner << function.mvArgument[0];
    for (size_t i = 1; i < function.mvArgument.size(); ++i)
        aligner << ", " << function.mvArgument[i];
    aligner << ")";
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const Initialization& initialization)
{
    if (initialization.exist_namespace_())
    if (!initialization.namespace_().isVoid())
        aligner << initialization.namespace_() << "::";
    aligner << initialization.name() << Aligner::FunctionSpace();
    aligner << "(" << initialization.value() << ")";
    return aligner;
}


TableAligner& operator<<(TableAligner& aligner, const SimpleType& type)
{
    if (type.exist_namespace_())
    if (!type.namespace_().isVoid())
        aligner << type.namespace_().value() << "::";
    
    aligner << type.value();
    return aligner;
}

TableAligner& operator<<(TableAligner& aligner, char ch)
{
	aligner.ceil() << ch;
	return aligner;
}

TableAligner& operator<<(TableAligner& aligner, const std::string& str)
{
	aligner.ceil() << str;
	return aligner;
}

}