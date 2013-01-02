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

#include "parser.h"
#include "parameter_type_validator.h"
#include "structure_fields_validator.h"
#include "structure_sharable_validator.h"

#include "library/compil/document.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <assert.h>

namespace compil
{

static bool bInit = false;
static ParameterTypeValidatorPtr pParameterTypeEnumerationValidator(
            new ParameterTypeValidator(EObjectId::enumeration()));
static ParameterTypeValidatorPtr pParameterTypeIdentifierValidator(
            new ParameterTypeValidator(EObjectId::identifier()));
static StructureFieldsValidatorPtr pStructureFieldsValidator(
            new StructureFieldsValidator());
static StructureSharableValidatorPtr pStructureSharableValidator(
            new StructureSharableValidator());


Parser::Parser()
{
    mpMessageCollector.reset(new MessageCollector());
    mpSources.reset(new std::map<std::string, SourceIdSPtr>());

    if (!bInit)
    {
        bInit = true;

        DocumentSPtr document = lib::compil::CompilDocument::create();
        std::vector<PackageElementSPtr> package_elements;
        pParameterTypeEnumerationValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "small"));
        pParameterTypeEnumerationValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "short"));
        pParameterTypeEnumerationValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "integer"));

        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "small"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "short"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "integer"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "long"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "byte"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "word"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "dword"));
        pParameterTypeIdentifierValidator->addAcceptableType(
            document->findType(PackageSPtr(), package_elements, "qword"));
    }

    addValidator(pParameterTypeEnumerationValidator);
    addValidator(pParameterTypeIdentifierValidator);
    addValidator(pStructureFieldsValidator);
    addValidator(pStructureSharableValidator);
}

Parser::Parser(const Parser& parentParser)
{
    mpMessageCollector = parentParser.mpMessageCollector;

    mpSourceProvider = parentParser.mpSourceProvider;
    mpSources = parentParser.mpSources;
}

Parser::~Parser()
{
    std::vector<Message> messages = mpMessageCollector->messages();
    std::vector<Message>::iterator it;
    for (it = messages.begin(); it != messages.end(); ++it)
    {
        std::string source = it->sourceId() ? it->sourceId()->value() : "compil";
        std::cout << source << ":"
                  << it->line().value() << ":"
                  << it->column().value() << " "
                  << it->text() << "\n";
    }
}

void Parser::setInput(const boost::shared_ptr<std::istream>& pInput)
{
    mContext = boost::make_shared<DocumentParseContext>();
    mContext->mTokenizer.reset(new Tokenizer(mpMessageCollector, mpSourceId, pInput));
}

CommentSPtr Parser::parseComment()
{
    CommentSPtr pComment;
    Line lastCommentLine(-1);
    for (;;)
    {
        if (!mContext->mTokenizer->current())
            break;
        if (mContext->mTokenizer->current()->type() != Token::TYPE_COMMENT)
            break;
        if (    (lastCommentLine != Line(-1))
             && (Line(1) < mContext->mTokenizer->current()->line() - lastCommentLine))
            {
            break;
        }
        if (!pComment)
        {
            pComment.reset(new Comment());
            initilizeObject(pComment);
        }
        lastCommentLine = mContext->mTokenizer->current()->line();
        std::string comment_line = mContext->mTokenizer->current()->text();
        boost::trim(comment_line);
        pComment->mutable_lines().push_back(comment_line);
        mContext->mTokenizer->shift();
    }
    return pComment;
}

CommentSPtr Parser::lastComment()
{
    CommentSPtr pComment = parseComment();
    while (pComment)
    {
        if (!mContext->mTokenizer->current())
            break;
        if (mContext->mTokenizer->current()->type() != Token::TYPE_COMMENT)
            break;
        mpMessageCollector->addMessage(
            Message::SEVERITY_WARNING, Message::p_misplacedComment,
            mpSourceId, pComment->line(), pComment->column());
        pComment = parseComment();
    }
    return pComment;
}

void Parser::skipComments(CommentSPtr pComment)
{
    if (!pComment)
        pComment = parseComment();
    while (pComment)
    {
        mpMessageCollector->addMessage(
            Message::SEVERITY_WARNING, Message::p_misplacedComment,
            mpSourceId, pComment->line(), pComment->column());
        pComment = parseComment();
    }
}

bool Parser::convertStringElementsToPackageElements(const std::vector<std::string>& string_elements,
                                                    std::vector<PackageElementSPtr>& package_elements)
{
    std::vector<PackageElementSPtr>::const_reverse_iterator eit = mpSourceId->externalElements().rbegin();

    std::vector<std::string>::const_reverse_iterator it;
    for (it = string_elements.rbegin(); it != string_elements.rend(); ++it)
    {
        PackageElementSPtr pe;
        if (*it == "*")
        {
            if (eit == mpSourceId->externalElements().rend())
            {
                *this << errorMessage(Message::p_asteriskPackageElement);
                return false;
            }
            pe = *eit;
            ++eit;
        }
        else
        {
            if ((*eit)->value() == *it)
                ++eit;

            pe = boost::make_shared<PackageElement>();
            pe->set_value(*it);
        }
        package_elements.insert(package_elements.begin(), pe);
    }
    return true;
}

PackageSPtr Parser::parsePackage()
{
    PackageSPtr pPackage(new Package());
    initilizeObject(pPackage);

    std::vector<std::string> short_elements;
    do
    {
        mContext->mTokenizer->shift();
        skipComments();

        if (expect(Token::TYPE_IDENTIFIER) || expect(Token::TYPE_ASTERISK))
        {
            short_elements.push_back(mContext->mTokenizer->current()->text());
        }
        else
        {
            *this << (errorMessage(Message::p_expectStatementName)
                        << Message::Statement("package"));
            return PackageSPtr();
        }

        mContext->mTokenizer->shift();
        skipComments();

    } while (expect(Token::TYPE_DOT));
    
    std::vector<std::string> levels_elements;
    if (mContext->mTokenizer->check(Token::TYPE_BITWISE_OPERATOR, "|"))
    {
        do
        {
            mContext->mTokenizer->shift();
            skipComments();

            if (expect(Token::TYPE_IDENTIFIER) || expect(Token::TYPE_ASTERISK))
            {
                levels_elements.push_back(mContext->mTokenizer->current()->text());
            }
            else
            {
                *this << (errorMessage(Message::p_expectStatementName)
                            << Message::Statement("package"));
                return PackageSPtr();
            }

            mContext->mTokenizer->shift();
            skipComments();

        } while (expect(Token::TYPE_DOT));
    }
    else
    {
        levels_elements = short_elements;
    }

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return PackageSPtr();
    }

    std::vector<PackageElementSPtr> short_;
    if (!convertStringElementsToPackageElements(short_elements, short_))
        return PackageSPtr();
        
    std::vector<PackageElementSPtr> levels;
    if (!convertStringElementsToPackageElements(levels_elements, levels))
        return PackageSPtr();
    

    pPackage->set_short(short_);
    pPackage->set_levels(levels);
    return pPackage;
}


bool Parser::parseType(std::vector<PackageElementSPtr>& package_elements, TokenPtr& pNameToken)
{
    pNameToken = mContext->mTokenizer->current();

    mContext->mTokenizer->shift();
    while (mContext->mTokenizer->check(Token::TYPE_DOT))
    {
        mContext->mTokenizer->shift();
        if (!expect(Token::TYPE_IDENTIFIER))
            return false;

        PackageElementSPtr pe = boost::make_shared<PackageElement>();
        pe->set_value(pNameToken->text());

        package_elements.push_back(pe);
        pNameToken = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
    }

    return true;
}

// returns false if the format is broken
// if it returns true the type still could be unknown
bool Parser::parseParameterType(InitTypeMethod initTypeMethod,
                                const std::string& defaultTypeName)
{
    TypeSPtr pType;

    std::vector<PackageElementSPtr> package_elements;
    if (!mContext->mTokenizer->check(Token::TYPE_ANGLE_BRACKET, "<"))
    {
        if (defaultTypeName.empty())
        {
            *this << (errorMessage(Message::p_expectType)
                        << Message::Classifier("class parameter"));
            return false;
        }

        StructureSPtr pStructure;
        if (pType) pStructure = ObjectFactory::downcastStructure(pType);

        if (pStructure)
            pType = mDocument->findType(mpPackage, package_elements, pStructure->objects(), defaultTypeName);
        else
            pType = mDocument->findType(mpPackage, package_elements, defaultTypeName);

        if (!pType)
        {
            *this << (errorMessage(Message::p_unknownClassifierType)
                        << Message::Classifier("default class parameter")
                        << Message::Type(defaultTypeName));
        }
        else
        {
            initTypeMethod(pType);
        }
        return true;
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectType)
                    << Message::Classifier("class parameter"));
        return false;
    }

    TokenPtr pNameToken;
    if (!parseType(package_elements, pNameToken))
        return FieldSPtr();

    skipComments();

    UnaryTemplateSPtr pUnaryTemplate = mDocument->findUnfinishedUnaryTemplate(pNameToken->text());
    if (pUnaryTemplate)
    {
        UnaryTemplateSPtr pUnaryTemplateClone =
            ObjectFactory::downcastUnaryTemplate(ObjectFactory::clone(pUnaryTemplate));

        parseParameterType(boost::bind(&UnaryTemplate::set_parameterType, pUnaryTemplateClone, _1));

        mDocument->cache(pUnaryTemplateClone);
        pType = pUnaryTemplateClone;

        ReferenceSPtr pReference = ObjectFactory::downcastReference(pType);
        if (pReference)
            pReference->set_weak(false);
    }
    else
    {
        pType = mDocument->findType(mpPackage, package_elements, pNameToken->text());
    }

    if (!expect(Token::TYPE_ANGLE_BRACKET, ">"))
    {
        *this << errorMessage(Message::p_expectClosingAngleBracket);
        return false;
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (pType)
    {
        initTypeMethod(pType);
    }
    else
    {
        LateTypeResolveInfo info;
        info.pToken = pNameToken;
        info.classifier = "class parameter";
        info.initTypeMethod = initTypeMethod;
        mLateTypeResolve.push_back(info);
    }
    return true;
}

EnumerationValueSPtr Parser::parseEnumerationValue(const CommentSPtr& pComment,
                                                   const std::vector<EnumerationValueSPtr>& values)
{
    assert(mContext->mTokenizer->check(Token::TYPE_IDENTIFIER));

    TokenPtr pNameToken = mContext->mTokenizer->current();

    EnumerationValueSPtr pEnumerationValue;
    mContext->mTokenizer->shift();
    skipComments();

    if (mContext->mTokenizer->check(Token::TYPE_OPERATOR, "="))
    {
        mContext->mTokenizer->shift();
        skipComments();

        std::vector<EnumerationValueSPtr> composeValues;
        for (;;)
        {
            if (!expect(Token::TYPE_IDENTIFIER))
            {
                *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                          << Message::Statement("enumeration"));
                return EnumerationValueSPtr();
            }

            EnumerationValueSPtr pFoundValue;
            BOOST_FOREACH(EnumerationValueSPtr pValue, values)
            {
                if (mContext->mTokenizer->current()->text() == pValue->name()->value())
                {
                    pFoundValue = pValue;
                    break;
                }
            }

            if (!pFoundValue)
            {
                *this << (errorMessage(Message::p_expectStatementName)
                          << Message::Statement("enumeration value"));
                return EnumerationValueSPtr();
            }

            composeValues.push_back(pFoundValue);

            mContext->mTokenizer->shift();
            skipComments();

            if (!mContext->mTokenizer->check(Token::TYPE_BITWISE_OPERATOR, "|"))
                break;

            mContext->mTokenizer->shift();
            skipComments();
        }

        ComposedEnumerationValue* pComposedEnumerationValue = new ComposedEnumerationValue();
        pEnumerationValue.reset(pComposedEnumerationValue);

        pComposedEnumerationValue->set_values(composeValues);
    }
    else
    {
        pEnumerationValue.reset(new AbsoluteEnumerationValue());
    }

    initilizeObject(pEnumerationValue, pNameToken);
    pEnumerationValue->set_comment(pComment);

    NameSPtr pName(new Name());
    initilizeObject(pName, pNameToken);
    pName->set_value(pNameToken->text());

    pEnumerationValue->set_name(pName);

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return EnumerationValueSPtr();
    }

    return pEnumerationValue;
}

EnumerationSPtr Parser::parseEnumeration(const CommentSPtr& pComment,
                                         const TokenPtr& pCast,
                                         const TokenPtr& pFlags)
{
    EnumerationSPtr pEnumeration(new Enumeration());
    pEnumeration->set_comment(pComment);
    initilizeObject(pEnumeration, pCast ? pCast :
                                          pFlags);
    pEnumeration->set_package(mpPackage);

    CastableType::ECast cast = CastableType::ECast::weak();
    if (pCast && pCast->text() == "strong")
        cast = CastableType::ECast::strong();
    pEnumeration->set_cast(cast);
    pEnumeration->set_flags(pFlags);

    mContext->mTokenizer->shift();
    skipComments();

    if (!parseParameterType(boost::bind(&Enumeration::set_parameterType, pEnumeration, _1), "integer"))
        return EnumerationSPtr();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("enumeration"));
        return EnumerationSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pEnumeration->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("enumeration"));
        return EnumerationSPtr();
    }

    mContext->mTokenizer->shift();

    std::vector<EnumerationValueSPtr> enumerationValues;

    for (;;)
    {
        if (eof())
        {
            *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                      << Message::Statement("enumeration"));
            return EnumerationSPtr();
        }

        CommentSPtr pEnumerationComment = lastComment();

        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER))
        {
            EnumerationValueSPtr pEnumerationValue =
                parseEnumerationValue(pEnumerationComment, enumerationValues);
            if (!pEnumerationValue)
            {
                recover();
                break;
            }
            pEnumerationValue->set_enumeration(pEnumeration);

            enumerationValues.push_back(pEnumerationValue);
        }
        else
        {
            skipComments(pEnumerationComment);
            break;
        }

        mContext->mTokenizer->shift();
    }

    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                      << Message::Statement("enumeration"));
        return EnumerationSPtr();
    }

    int mEnumValue = pEnumeration->flags() ? 0 : 1; // 0 reserved for invalid
    BOOST_FOREACH(EnumerationValueSPtr pEnumerationValue, enumerationValues)
    {
        AbsoluteEnumerationValueSPtr pAbsoluteValue =
            ObjectFactory::downcastAbsoluteEnumerationValue(pEnumerationValue);
        if (!pAbsoluteValue) continue;
        pAbsoluteValue->set_value(mEnumValue++);
    }

    pEnumeration->set_enumerationValues(enumerationValues);

    if (!validate(pEnumeration))
        return EnumerationSPtr();

    return pEnumeration;
}

IdentifierSPtr Parser::parseIdentifier(const CommentSPtr& pComment,
                                      const TokenPtr& pCast)
{
    IdentifierSPtr pIdentifier(new Identifier());
    pIdentifier->set_comment(pComment);
    initilizeObject(pIdentifier, pCast);
    pIdentifier->set_package(mpPackage);

    CastableType::ECast cast = CastableType::ECast::weak();
    if (pCast && pCast->text() == "strong")
        cast = CastableType::ECast::strong();
    pIdentifier->set_cast(cast);

    mContext->mTokenizer->shift();
    skipComments();

    if (!parseParameterType(boost::bind(&Identifier::set_parameterType, pIdentifier, _1), "integer"))
        return IdentifierSPtr();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("identifier"));
        return IdentifierSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pIdentifier->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("identifier"));
        return IdentifierSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("identifier"));
        return IdentifierSPtr();
    }

    if (!validate(pIdentifier))
        return IdentifierSPtr();

    return pIdentifier;
}

SpecimenSPtr Parser::parseSpecimen(const CommentSPtr& pComment)
{
    SpecimenSPtr pSpecimen(new Specimen());
    pSpecimen->set_comment(pComment);
    initilizeObject(pSpecimen);
    pSpecimen->set_package(mpPackage);

    mContext->mTokenizer->shift();
    skipComments();

    if (!parseParameterType(boost::bind(&Specimen::set_parameterType, pSpecimen, _1), "integer"))
        return SpecimenSPtr();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("specimen"));
        return SpecimenSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pSpecimen->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "inherit"))
    {
        mContext->mTokenizer->shift();
        skipComments();
        if (!expect(Token::TYPE_IDENTIFIER))
        {
            *this << (errorMessage(Message::p_expectClassifierStatementName)
                        << Message::Classifier("base")
                        << Message::Statement("specimen"));
            return SpecimenSPtr();
        }

        std::vector<PackageElementSPtr> package_elements;
        TokenPtr pTypeNameToken;
        if (!parseType(package_elements, pTypeNameToken))
            return SpecimenSPtr();

        skipComments();

        TypeSPtr pType = mDocument->findType(mpPackage, package_elements, pTypeNameToken->text());
        if (!pType)
        {
            *this << (errorMessage(Message::p_unknownClassifierType,
                                   pTypeNameToken->line(), pTypeNameToken->beginColumn())
                        << Message::Classifier("base")
                        << Message::Type(pTypeNameToken->text()));
            return SpecimenSPtr();
        }
        if (pType->runtimeObjectId() != EObjectId::specimen())
        {
            *this << (errorMessage(Message::p_expectAppropriateType,
                                   pTypeNameToken->line(), pTypeNameToken->beginColumn())
                        << Message::Classifier("base")
                        << Message::Options("speciment"));
            return SpecimenSPtr();
        }

        SpecimenSPtr pBaseSpecimen = boost::static_pointer_cast<Specimen>(pType);
        pSpecimen->set_baseSpecimen(pBaseSpecimen);
    }

    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("specimen"));
        return SpecimenSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("specimen"));
        return SpecimenSPtr();
    }

    if (!validate(pSpecimen))
        return SpecimenSPtr();

    return pSpecimen;
}

FilterSPtr Parser::parseFilter(const CommentSPtr& pComment,
                               const StructureSPtr& pStructure)
{
    FilterSPtr pFilter(new Filter());
    pFilter->set_comment(pComment);
    initilizeObject(pFilter);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("field"));
        return FilterSPtr();
    }

    FieldSPtr pField = pStructure->findField(mContext->mTokenizer->current()->text());
    if (!pField)
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("field"));
        return FilterSPtr();
    }

    pFilter->set_field(pField);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER, "with"))
    {
        *this << (errorMessage(Message::p_expectKeyword)
                    << Message::Keyword("with"));
        return FilterSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("external method"));
        return FilterSPtr();
    }

    pFilter->set_method(mContext->mTokenizer->current()->text());

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return FilterSPtr();
    }

    return pFilter;
}

FactorySPtr Parser::parseFactory(const CommentSPtr& pComment,
                                 const TokenPtr& pFunctionType,
                                 const TokenPtr& pFactoryType)
{
    FactorySPtr pFactory(new Factory());
    pFactory->set_comment(pComment);
    initilizeObject(pFactory, pFunctionType ? pFunctionType :
                                              pFactoryType);
    pFactory->set_package(mpPackage);

    mContext->mTokenizer->shift();
    skipComments();

    if (!pFactoryType)
    {
        *this << (errorMessage(Message::p_expectAppropriateType)
                      << Message::Classifier("factory")
                      << Message::Options("hierarchy, object or plugin"));
        return FactorySPtr();
    }

    pFactory->set_function(pFunctionType);

    Factory::EType type = Factory::EType::invalid();
    if (pFactoryType->text() == "hierarchy")
        type = Factory::EType::hierarchy();
    if (pFactoryType->text() == "object")
        type = Factory::EType::object();
    if (pFactoryType->text() == "plugin")
        type = Factory::EType::plugin();

    if (pFunctionType && type != Factory::EType::object())
    {
        *this << (errorMessage(Message::p_expectAppropriateType)
                      << Message::Classifier("factory")
                      << Message::Options("object"));
        return FactorySPtr();
    }

    pFactory->set_type(type);

    if (!parseParameterType(boost::bind(&Factory::set_parameterType, pFactory, _1)))
        return FactorySPtr();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("factory"));
        return FactorySPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pFactory->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("factory"));
        return FactorySPtr();
    }

    mContext->mTokenizer->shift();

    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "filter"))
    {
        if (pFactory->type() != Factory::EType::object())
        {
            *this << (errorMessage(Message::p_filterInNonObjectFactory));
            recover();
            return FactorySPtr();
        }

        if    (!pFactory->parameterType().lock()
           || (!ObjectFactory::downcastStructure(pFactory->parameterType().lock())))
        {
            *this << (errorMessage(Message::p_filterInFactoryForNonStructure));
            recover();
            return FactorySPtr();
        }
    }

    std::vector<FilterSPtr> filters;
    for (;;)
    {
        if (eof())
        {
            *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                        << Message::Statement("factory"));
            return FactorySPtr();
        }

        CommentSPtr pBodyComment = lastComment();

        if (!mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "filter"))
        {
            skipComments(pBodyComment);
            break;
        }

        FilterSPtr pFilter = parseFilter(pBodyComment,
            ObjectFactory::downcastStructure(pFactory->parameterType().lock()));
        if (!pFilter)
        {
            recover();
            break;
        }
        filters.push_back(pFilter);

        mContext->mTokenizer->shift();
    }

    pFactory->set_filters(filters);

    skipComments();
    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("factory"));
        return FactorySPtr();
    }

    if (!validate(pFactory))
        return FactorySPtr();

    return pFactory;
}


// TODO: string default value multi line support
// TODO: binary default value - binaty bin[] = base64:YASfsdfj==,
//                              binaty bin[] = hex:AC2312DFF004234534
// TODO: limits - string s[10], binary bin[10]
// TODO: auto limits - string s[] = "blah", binaty bin[] = base64:YASfsdfj==

FieldSPtr Parser::parseField(const CommentSPtr& pComment,
                             const std::vector<ObjectSPtr>& structureObjects,
                             TokenPtr& pWeak)
{
    FieldSPtr pField(new Field());
    initilizeObject(pField, pWeak);
    pField->set_comment(pComment);

    assert(mContext->mTokenizer->check(Token::TYPE_IDENTIFIER));

    std::vector<PackageElementSPtr> package_elements;
    TokenPtr pTypeNameToken;
    if (!parseType(package_elements, pTypeNameToken))
        return FieldSPtr();

    skipComments();

    TypeSPtr pType;
    UnaryTemplateSPtr pUnaryTemplate = mDocument->findUnfinishedUnaryTemplate(pTypeNameToken->text());
    if (pUnaryTemplate)
    {
        UnaryTemplateSPtr pUnaryTemplateClone =
            ObjectFactory::downcastUnaryTemplate(ObjectFactory::clone(pUnaryTemplate));

        if (!parseParameterType(boost::bind(&UnaryTemplate::set_parameterType, pUnaryTemplateClone, _1)))
            return FieldSPtr();

        pType = pUnaryTemplateClone;

        ReferenceSPtr pReference = ObjectFactory::downcastReference(pType);
        if (pReference)
        {
            pReference->set_weak(pWeak);
            pWeak.reset();
        }
    }
    else
    {
        pType = mDocument->findType(mpPackage, package_elements, structureObjects, pTypeNameToken->text());
    }

    if (pType)
    {
        pField->set_type(pType);
    }
    else
    {
        LateTypeResolveInfo info;
        info.pToken = pTypeNameToken;
        info.classifier = "field";
        info.initTypeMethod = boost::bind(&Field::set_type, pField, _1);
        mLateTypeResolve.push_back(info);
    }

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("field"));
        return FieldSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pField->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();

    if (mContext->mTokenizer->check(Token::TYPE_DELIMITER, ";"))
        return pField;

    if (!expect(Token::TYPE_OPERATOR, "="))
    {
        *this << errorMessage(Message::p_expectSemicolonOrAssignmentOperator);
        return FieldSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();

    DefaultValueSPtr pDefaultValue(new DefaultValue());
    initilizeObject(pDefaultValue);

    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "optional"))
    {
        pDefaultValue->set_optional(true);
    }
    else
    {
        // TODO: LateTypeResolve need to have a late value evaluation

        Token::Type type = getTokenType(pType->literal());
        if (!expect(type))
        {
            *this << errorMessage(Message::p_expectFieldDefaultValue);
            return FieldSPtr();
        }

        pDefaultValue->set_value(mContext->mTokenizer->current()->text());
    }
    pField->set_defaultValue(pDefaultValue);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return FieldSPtr();
    }

    return pField;
}

FieldOverrideSPtr Parser::parseFieldOverride(const FieldSPtr& pField,
                                             const StructureSPtr& pStructure,
                                             const TokenPtr& pOverride)
{
    FieldOverrideSPtr pFieldOverride(new FieldOverride());
    initilizeObject(pFieldOverride, pOverride);

    StructureSPtr pBaseStructure = pStructure->baseStructure().lock();
    if (!pBaseStructure)
    {
        // TODO:
        *this << errorMessage(Message::p_expectSemicolon);
        return FieldOverrideSPtr();
    }


    FieldSPtr pBaseField = pBaseStructure->findField(pField->name()->value());
    if (!pBaseField)
    {
        // TODO:
        *this << errorMessage(Message::p_expectSemicolon);
        return FieldOverrideSPtr();
    }

    pFieldOverride->set_overriddenField(pBaseField);
    pFieldOverride->set_field(pField);

    return pFieldOverride;
}

IdentificationSPtr Parser::parseIdentification(const CommentSPtr& pComment,
                                               const TokenPtr& pType)
{
    IdentificationSPtr pIdentification(new Identification());
    pIdentification->set_comment(pComment);
    initilizeObject(pIdentification, pType);

    if (!pType)
    {
        *this << (errorMessage(Message::p_expectAppropriateType)
                    << Message::Classifier("identification")
                    << Message::Options("runtime or inproc"));
        return IdentificationSPtr();
    }

    Identification::EType type = Identification::EType::invalid();
    if (pType->text() == "runtime")
        type = Identification::EType::runtime();
    if (pType->text() == "inproc")
        type = Identification::EType::inproc();

    pIdentification->set_type(type);

    assert(mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "identification"));

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return IdentificationSPtr();
    }

    return pIdentification;
}

UpcopySPtr Parser::parseUpcopy(const CommentSPtr& pComment,
                               const StructureSPtr& pStructure)
{
    UpcopySPtr pUpcopy(new Upcopy());
    pUpcopy->set_comment(pComment);
    initilizeObject(pUpcopy);

    assert(mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "upcopy"));

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER, "from"))
    {
        *this << (errorMessage(Message::p_expectKeyword)
                    << Message::Keyword("from"));
        return UpcopySPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectClassifierStatementName)
                    << Message::Classifier("upcopy")
                    << Message::Statement("structure"));
        return UpcopySPtr();
    }

    std::vector<PackageElementSPtr> package_elements;
    TokenPtr pTypeNameToken;
    if (!parseType(package_elements, pTypeNameToken))
        return UpcopySPtr();

    TypeSPtr pType = mDocument->findType(mpPackage, package_elements, pTypeNameToken->text());
    if (!pType)
    {
        *this << (errorMessage(Message::p_unknownClassifierType,
                               pTypeNameToken->line(), pTypeNameToken->beginColumn())
                    << Message::Classifier("upcopy")
                    << Message::Type(pTypeNameToken->text()));
        return UpcopySPtr();
    }

    StructureSPtr pBaseStructure = ObjectFactory::downcastStructure(pType);
    if (!pBaseStructure)
    {
        *this << (errorMessage(Message::p_expectAppropriateType,
                               pTypeNameToken->line(), pTypeNameToken->beginColumn())
                    << Message::Classifier("upcopy")
                    << Message::Options("structure"));
        return UpcopySPtr();
    }

    if (!pStructure->isRecursivelyInherit(pBaseStructure))
    {
        *this << (errorMessage(Message::p_expectAppropriateType,
                               pTypeNameToken->line(), pTypeNameToken->beginColumn())
                    << Message::Classifier("upcopy")
                    << Message::Options("base structure"));
        return UpcopySPtr();
    }

    pUpcopy->set_baseStructure(pBaseStructure);

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return UpcopySPtr();
    }

    return pUpcopy;
}

OperatorSPtr Parser::parseOperator(const CommentSPtr& pComment,
                                   const TokenPtr& pOperatorDeclaration)
{
    OperatorSPtr pOperator(new Operator());
    pOperator->set_comment(pComment);
    initilizeObject(pOperator, pOperatorDeclaration);

    if (!pOperatorDeclaration)
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return OperatorSPtr();
    }

    EOperatorFlags flags;
    if (pOperatorDeclaration->text() == "native")
    {
        flags.set(EOperatorFlags::native());
        flags.set(EOperatorFlags::member());
    }
    else
    if (pOperatorDeclaration->text() == "function")
    {
        flags.set(EOperatorFlags::function());
        flags.set(EOperatorFlags::member());
    }
    else
    if (pOperatorDeclaration->text() == "functor")
    {
        flags.set(EOperatorFlags::functor());
        flags.set(EOperatorFlags::internal());
    }

    flags.set(EOperatorFlags::object());
    flags.set(EOperatorFlags::reference());

    pOperator->set_flags(flags);


    mContext->mTokenizer->shift();
    skipComments();

    if (expect(Token::TYPE_RELATIONAL_OPERATOR1, "=="))
    {
        pOperator->set_action(EOperatorAction::equalTo());
    }
    else
    if (expect(Token::TYPE_RELATIONAL_OPERATOR2, "<"))
    {
        pOperator->set_action(EOperatorAction::lessThan());
    }
    else
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return OperatorSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return OperatorSPtr();
    }

    return pOperator;
}

StructureSPtr Parser::parseStructure(const CommentSPtr& pComment,
                                     const TokenPtr& pAbstract,
                                     const TokenPtr& pContolled,
                                     const TokenPtr& pImmutable,
                                     const TokenPtr& pPartial,
                                     const TokenPtr& pSharable,
                                     const TokenPtr& pStreamable)
{
    StructureSPtr pStructure(new Structure());
    pStructure->set_comment(pComment);
    initilizeObject(pStructure, (pAbstract   ? pAbstract   :
                                (pContolled  ? pContolled  :
                                (pImmutable  ? pImmutable  :
                                (pPartial    ? pPartial    :
                                (pSharable   ? pSharable   :
                                               pStreamable))))));
    pStructure->set_package(mpPackage);

    pStructure->set_abstract(pAbstract);
    pStructure->set_controlled(pContolled);
    pStructure->set_immutable(pImmutable);
    pStructure->set_partial(pPartial);
    pStructure->set_sharable(pSharable);
    pStructure->set_streamable(pStreamable);


    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("structure"));
        return StructureSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pStructure->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();

    std::vector<ObjectSPtr> objects;

    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "inherit"))
    {
        mContext->mTokenizer->shift();
        skipComments();
        if (!expect(Token::TYPE_IDENTIFIER))
        {
            *this << (errorMessage(Message::p_expectClassifierStatementName)
                        << Message::Classifier("base")
                        << Message::Statement("structure"));
            return StructureSPtr();
        }

        std::vector<PackageElementSPtr> package_elements;
        TokenPtr pTypeNameToken;
        if (!parseType(package_elements, pTypeNameToken))
            return StructureSPtr();

        skipComments();

        TypeSPtr pType = mDocument->findType(mpPackage, package_elements, pTypeNameToken->text());
        if (!pType)
        {
            *this << (errorMessage(Message::p_unknownClassifierType,
                                   pTypeNameToken->line(), pTypeNameToken->beginColumn())
                        << Message::Classifier("base")
                        << Message::Type(pTypeNameToken->text()));
            return StructureSPtr();
        }
        if (pType->runtimeObjectId() != EObjectId::structure())
        {
            *this << (errorMessage(Message::p_expectAppropriateType,
                                   pTypeNameToken->line(), pTypeNameToken->beginColumn())
                        << Message::Classifier("base")
                        << Message::Options("structure"));
            return StructureSPtr();
        }

        StructureSPtr pBaseStructure = boost::static_pointer_cast<Structure>(pType);
        pStructure->set_baseStructure(pBaseStructure);

        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "alter"))
        {
            mContext->mTokenizer->shift();
            skipComments();

            for (;;)
            {
                if (eof())
                {
                    *this << (errorMessage(Message::p_expectStatementName)
                                << Message::Statement("field"));
                    return StructureSPtr();
                }

                if (!expect(Token::TYPE_IDENTIFIER))
                {
                    *this << (errorMessage(Message::p_expectStatementName)
                                << Message::Statement("field"));
                    return StructureSPtr();
                }

                AlterSPtr pAlter(new Alter());
                initilizeObject(pAlter);

                FieldSPtr pField = pBaseStructure->findField(mContext->mTokenizer->current()->text());
                if (!pField)
                {
                    *this << (errorMessage(Message::p_expectBaseStructureFieldName));
                    return StructureSPtr();
                }

                pAlter->set_field(pField);

                mContext->mTokenizer->shift();
                skipComments();

                if (!expect(Token::TYPE_OPERATOR, "="))
                {
                    *this << errorMessage(Message::p_expectAssignmentOperator);
                    return StructureSPtr();
                }

                mContext->mTokenizer->shift();
                skipComments();

                Token::Type type = getTokenType(pField->type()->literal());
                if (!expect(type))
                {
                    *this << errorMessage(Message::p_expectFieldDefaultValue);
                    return StructureSPtr();
                }

                DefaultValueSPtr pDefaultValue(new DefaultValue());
                initilizeObject(pDefaultValue);
                pDefaultValue->set_value(mContext->mTokenizer->current()->text());

                pAlter->set_defaultValue(pDefaultValue);

                mContext->mTokenizer->shift();
                skipComments();

                objects.push_back(pAlter);

                if (!mContext->mTokenizer->check(Token::TYPE_DELIMITER, ","))
                    break;

                mContext->mTokenizer->shift();
                skipComments();
            }
        }
    }

    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("structure"));
        return StructureSPtr();
    }


    int bitmask = 0;
    mContext->mTokenizer->shift();
    for (;;)
    {
        if (eof())
        {
            *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                        << Message::Statement("structure"));
            return StructureSPtr();
        }

        CommentSPtr pBodyComment = lastComment();

        TokenPtr pStrong;
        TokenPtr pWeak;
        TokenPtr pIdentificationType;
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "weak"))
        {
            pWeak = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }
        else
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "strong"))
        {
            pStrong = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }
        else
        if (   mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "runtime")
            || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "inproc"))
        {
            pIdentificationType = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }

        TokenPtr pFlags;
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "flags"))
        {
            pFlags = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }

        TokenPtr pOverride;
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "override"))
        {
            pOverride = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }

        TokenPtr pOperatorDeclaration;
        if (   mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "native")
            || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "function")
            || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "functor"))
        {
            pOperatorDeclaration = mContext->mTokenizer->current();
            mContext->mTokenizer->shift();
            skipComments();
        }

        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "identification"))
        {
            IdentificationSPtr pIdentification =
                parseIdentification(pBodyComment, pIdentificationType);
            if (!pIdentification)
            {
                recover();
                break;
            }
            pIdentificationType.reset();
            pIdentification->set_structure(pStructure);
            objects.push_back(pIdentification);
        }
        else
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "upcopy"))
        {
            UpcopySPtr pUpcopy =
                parseUpcopy(pBodyComment, pStructure);
            if (!pUpcopy)
            {
                recover();
                break;
            }
            pUpcopy->set_structure(pStructure);
            objects.push_back(pUpcopy);
        }
        else
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "operator"))
        {
            OperatorSPtr pOperator = parseOperator(pBodyComment, pOperatorDeclaration);
            pOperatorDeclaration.reset();
            if (!pOperator)
            {
                recover();
                break;
            }
            pOperator->set_structure(pStructure);
            objects.push_back(pOperator);
        }
        else
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "enum"))
        {
            EnumerationSPtr pEnumeration =
                parseEnumeration(pBodyComment, pStrong ? pStrong : pWeak, pFlags);
            if (!pEnumeration)
            {
                recover();
                break;
            }
            pStrong.reset();
            pWeak.reset();
            pFlags.reset();
            pEnumeration->set_structure(pStructure);
            objects.push_back(pEnumeration);
        }
        else
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER))
        {
            FieldSPtr pField = parseField(pBodyComment, objects, pWeak);
            if (!pField)
            {
                recover();
                break;
            }

            pField->set_structure(pStructure);

            if (pOverride)
            {
                FieldOverrideSPtr pFieldOverride = parseFieldOverride(pField, pStructure, pOverride);
                pOverride.reset();
                if (!pFieldOverride)
                {
                    recover();
                    break;
                }

                objects.push_back(pFieldOverride);
            }
            else
            {
                // TODO: address the limit
                pField->set_bitmask(bitmask++);
                objects.push_back(pField);
            }
        }
        else
        {
            skipComments(pBodyComment);
            break;
        }

        mContext->mTokenizer->shift();

        unexpectedStatement(pStrong);
        unexpectedStatement(pWeak);
        unexpectedStatement(pFlags);
        unexpectedStatement(pOverride);
        unexpectedStatement(pOperatorDeclaration);
        unexpectedStatement(pIdentificationType);
    }

    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("structure"));
        return StructureSPtr();
    }

    pStructure->set_objects(objects);

    if (!validate(pStructure))
        return StructureSPtr();

    return pStructure;
}

ParameterSPtr Parser::parseParameter(const CommentSPtr pComment)
{
    ParameterSPtr pParameter(new Parameter());
    initilizeObject(pParameter);
    pParameter->set_comment(pComment);

    if (!expect(Token::TYPE_OPERATOR_ARROW))
    {
        *this << (errorMessage(Message::p_unknownStatment)
                    << Message::Context("method")
                    << Message::Options("direction arrow (-->, <--, <->)"));
        return ParameterSPtr();
    }
    Parameter::EDirection direction = Parameter::EDirection::invalid();
    if (mContext->mTokenizer->current()->text() == "-->")
        direction = Parameter::EDirection::in();
    else if (mContext->mTokenizer->current()->text() == "<--")
        direction = Parameter::EDirection::out();
    else if (mContext->mTokenizer->current()->text() == "<->")
        direction = Parameter::EDirection::io();
    else
        assert(false && "unknown arrow");

    pParameter->set_direction(direction);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectType)
                    << Message::Classifier("parameter"));
        return ParameterSPtr();
    }

    std::vector<PackageElementSPtr> package_elements;
    TypeSPtr pType = mDocument->findType(mpPackage, package_elements, mContext->mTokenizer->current()->text());
    if (!pType)
    {
        *this << (errorMessage(Message::p_unknownClassifierType)
                    << Message::Classifier("parameter")
                    << Message::Type(mContext->mTokenizer->current()->text()));
        return ParameterSPtr();
    }
    pParameter->set_type(pType);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("parameter"));
        return ParameterSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pParameter->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return ParameterSPtr();
    }

    return pParameter;
}

MethodSPtr Parser::parseMethod(const CommentSPtr& pComment)
{
    MethodSPtr pMethod(new Method());
    initilizeObject(pMethod);
    pMethod->set_comment(pComment);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("method"));
        return MethodSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pMethod->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("method"));
        return MethodSPtr();
    }
    mContext->mTokenizer->shift();

    CommentSPtr pParameterComment = lastComment();

    std::vector<ObjectSPtr> parameters;
    while (!mContext->mTokenizer->check(Token::TYPE_BRACKET, "}"))
    {
        if (eof())
        {
            *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                        << Message::Statement("method"));
            return MethodSPtr();
        }

        ParameterSPtr pParameter = parseParameter(pParameterComment);
        if (!pParameter)
        {
            recover();
            break;
        }

        pParameter->set_method(pMethod);
        parameters.push_back(pParameter);

        mContext->mTokenizer->shift();
    }

    pMethod->set_objects(parameters);

    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("method"));
        return MethodSPtr();
    }

    mContext->mTokenizer->shift();

    return pMethod;
}

InterfaceSPtr Parser::parseInterface(const CommentSPtr& pComment)
{
    InterfaceSPtr pInterface(new Interface());
    initilizeObject(pInterface);
    pInterface->set_comment(pComment);

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_IDENTIFIER))
    {
        *this << (errorMessage(Message::p_expectStatementName)
                    << Message::Statement("interface"));
        return InterfaceSPtr();
    }

    NameSPtr pName(new Name());
    initilizeObject(pName);
    pName->set_value(mContext->mTokenizer->current()->text());

    pInterface->set_name(pName);

    mContext->mTokenizer->shift();
    skipComments();
    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("interface"));
        return InterfaceSPtr();
    }
    mContext->mTokenizer->shift();

    std::vector<ObjectSPtr> methods;
    while (!mContext->mTokenizer->check(Token::TYPE_BRACKET, "}"))
    {
        if (eof())
        {
            *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("interface"));
            return InterfaceSPtr();
        }

        CommentSPtr pMethodComment = lastComment();
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "method"))
        {
            MethodSPtr pMethod = parseMethod(pMethodComment);
            if (!pMethod)
            {
                recover();
                break;
            }
            pMethod->set_interface(pInterface);
            methods.push_back(pMethod);
        }
    }

    pInterface->set_objects(methods);

    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("interface"));
        return InterfaceSPtr();
    }

    mContext->mTokenizer->shift();
    return pInterface;
}

bool Parser::unexpectedStatement(const TokenPtr& pToken)
{
    if (!pToken)
        return false;

    *this << (errorMessage(Message::p_unexpectedStatmentModificator, pToken->line(), pToken->beginColumn())
          << Message::Modificator(pToken->text()));

    recover();

    return true;
}

void Parser::parseAnyStatement(const CommentSPtr& pComment)
{
    TokenPtr pAbstract;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "abstract"))
    {
        pAbstract = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pControlled;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "controlled"))
    {
        pControlled = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pImmutable;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "immutable"))
    {
        pImmutable = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pPartial;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "partial"))
    {
        pPartial = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pSharable;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "sharable"))
    {
        pSharable = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pStreamable;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "streamable"))
    {
        pStreamable = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pCast;
    if (  mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "strong")
       || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "weak"))
    {
        pCast = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pFlags;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "flags"))
    {
        pFlags = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pFunctionType;
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "function"))
    {
        pFunctionType = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    TokenPtr pFactoryType;
    if (  mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "hierarchy")
       || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "object")
       || mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "plugin"))
    {
        pFactoryType = mContext->mTokenizer->current();
        mContext->mTokenizer->shift();
        skipComments();
    }

    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "structure"))
    {
        StructureSPtr pStructure = parseStructure(pComment, pAbstract, pControlled, pImmutable,
                                                  pPartial, pSharable, pStreamable);
        pAbstract.reset();
        pControlled.reset();
        pImmutable.reset();
        pPartial.reset();
        pSharable.reset();
        pStreamable.reset();
        if (pStructure)
        {
            mDocument->addStructure(pStructure);
            lateTypeResolve(pStructure);
        }
    }
    else
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "interface"))
    {
        InterfaceSPtr pInterface = parseInterface(pComment);
        if (pInterface)
            mDocument->addInterface(pInterface);
    }
    else
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "enum"))
    {
        EnumerationSPtr pEnumeration = parseEnumeration(pComment, pCast, pFlags);
        pCast.reset();
        pFlags.reset();
        if (pEnumeration)
            mDocument->addEnumeration(pEnumeration);
    }
    else
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "specimen"))
    {
        SpecimenSPtr pSpecimen = parseSpecimen(pComment);
        if (pSpecimen)
        {
            mDocument->addSpecimen(pSpecimen);
        }
    }
    else
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "identifier"))
    {
        IdentifierSPtr pIdentifier = parseIdentifier(pComment, pCast);
        pCast.reset();
        if (pIdentifier)
            mDocument->addIdentifier(pIdentifier);
    }
    else
    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "factory"))
    {
        FactorySPtr pFactory = parseFactory(pComment, pFunctionType, pFactoryType);
        pFunctionType.reset();
        pFactoryType.reset();
        if (pFactory)
            mDocument->addFactory(pFactory);
    }
    else
    {
        *this << (errorMessage(Message::p_unknownStatment)
                << Message::Context("top")
                << Message::Options("structure, interface, enum, specimen, identifier or factory"));
        mContext->mTokenizer->shift();
    }

    unexpectedStatement(pAbstract);
    unexpectedStatement(pControlled);
    unexpectedStatement(pImmutable);
    unexpectedStatement(pPartial);
    unexpectedStatement(pSharable);
    unexpectedStatement(pStreamable);
    unexpectedStatement(pCast);
    unexpectedStatement(pFlags);
    unexpectedStatement(pFunctionType);
    unexpectedStatement(pFactoryType);
}

bool Parser::parseImport()
{
    ImportSPtr pImport(new Import());
    initilizeObject(pImport);

    assert(mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "import"));

    TokenPtr pImportToken = mContext->mTokenizer->current();

    mContext->mTokenizer->shift();
    if (!expect(Token::TYPE_STRING_LITERAL))
    {
        *this << errorMessage(Message::p_expectImportSource);
        return false;
    }

    pImport->set_source(mContext->mTokenizer->current()->text());

    mContext->mTokenizer->shift();
    if (!expect(Token::TYPE_DELIMITER, ";"))
    {
        *this << errorMessage(Message::p_expectSemicolon);
        return false;
    }

    mDocument->addImport(pImport);

    if (!mpSourceProvider)
    {
        *this << warningMessage(Message::p_importWithoutSourceProvider,
                              pImportToken->line(), pImportToken->beginColumn());
        return true;
    }

    SourceIdSPtr pSourceId = mpSourceProvider->sourceId(mpSourceId, pImport->source());
    if (!pSourceId)
    {
        *this << errorMessage(Message::p_sourceNotFound,
                              pImportToken->line(), pImportToken->beginColumn());
        return false;
    }

    std::map<std::string, SourceIdSPtr>::iterator it = mpSources->find(pSourceId->value());
    if (it != mpSources->end())
    {
        SourceIdSPtr parent = mpSourceId;
        while (parent)
        {
            if (parent->value() == pSourceId->value())
                return true;

            if (!parent->parent())
                break;
            parent = parent->parent();
        }
        return true;
    }

    StreamPtr pStream = mpSourceProvider->openInputStream(pSourceId);
    if (!pStream)
    {
        *this << errorMessage(Message::p_openSourceFailed,
                              pImportToken->line(), pImportToken->beginColumn());
        return false;
    }

    Parser parser(*this);
    if (!parser.parse(pSourceId, pStream, mDocument))
        return false;

    mLateTypeResolve.insert(mLateTypeResolve.end(), parser.mLateTypeResolve.begin(), parser.mLateTypeResolve.end());

    return true;
}

FileSPtr Parser::parseFile()
{
    FileSPtr file = boost::make_shared<File>();

    CommentSPtr pComment = parseComment();
    while (pComment)
    {
        file->mutable_comments().push_back(pComment);
        pComment = parseComment();
    }

    if (!mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "compil"))
        return FileSPtr();

    initilizeObject(file);

    mContext->mTokenizer->shift();
    if (!expect(Token::TYPE_BRACKET, "{"))
    {
        *this << (errorMessage(Message::p_expectStatementBody)
                    << Message::Statement("compil"));
        return FileSPtr();
    }

    mContext->mTokenizer->shift();
    skipComments();

    if (!expect(Token::TYPE_BRACKET, "}"))
    {
        *this << (errorMessage(Message::p_unexpectEOFInStatementBody)
                    << Message::Statement("compil"));
        return FileSPtr();
    }

    mContext->mTokenizer->shift();

    return file;
}

void Parser::addValidator(const ValidatorPtr& pValidator)
{
    mvValidator.push_back(pValidator);
}

bool Parser::parse(const StreamPtr& pInput, const DocumentSPtr& document)
{
    mContext = boost::make_shared<DocumentParseContext>();
    
    
    mDocument = document;
    mContext->mTokenizer.reset(new Tokenizer(mpMessageCollector, mpSourceId, pInput));

    FileSPtr file = parseFile();
    if (!file)
        return false;

    if (!mDocument->mainFile())
        mDocument->set_mainFile(file);

    CommentSPtr pStatementComment = lastComment();
    while (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "import"))
    {
        if (!parseImport())
            return false;
        mContext->mTokenizer->shift();
        pStatementComment = lastComment();
    }

    if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER, "package"))
    {
        mpPackage = parsePackage();
        if (!mpPackage)
            return false;
        if (mDocument->mainFile() == file)
            mDocument->setPackage(mpPackage);

        mContext->mTokenizer->shift();
        pStatementComment = lastComment();
    }

    while (mContext->mTokenizer->current())
    {
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER))
        {
            parseAnyStatement(pStatementComment);
        }
        else
        {
            mContext->mTokenizer->shift();
        }
        pStatementComment = lastComment();
    }

    if (mDocument->mainFile() == file)
    {

        std::vector<LateTypeResolveInfo>::iterator it;
        for (it = mLateTypeResolve.begin(); it != mLateTypeResolve.end() ; ++it)
        {
            *this << (errorMessage(Message::p_unknownClassifierType,
                                   it->pToken->line(), it->pToken->beginColumn())
                  << Message::Classifier(it->classifier)
                  << Message::Type(it->pToken->text()));
        }

        if (mpMessageCollector->severity() > Message::SEVERITY_WARNING)
            return false;

        if (!validate(mDocument))
            return false;
    }
    return true;
}

bool Parser::parse(const SourceIdSPtr& pSourceId,
                   const StreamPtr& pInput,
                   const DocumentSPtr& document)
{
    assert(pSourceId);
    mpSourceId = pSourceId;
    mpSources->insert(
        std::map<std::string, SourceIdSPtr>::value_type(pSourceId->value(), mpSourceId));

    return parse(pInput, document);
}

bool Parser::parse(const ISourceProviderPtr& pSourceProvider,
                   const SourceIdSPtr& pSourceId,
                   const DocumentSPtr& document)

{
    mpSourceProvider = pSourceProvider;

    StreamPtr pStream = mpSourceProvider->openInputStream(pSourceId);
    if (!pStream)
    {
        *this << Message(Message::SEVERITY_ERROR, Message::p_openSourceFailed, mpSourceId, Line(1), Column(0));
        return false;
    }
    return parse(pSourceId, pStream, document);
}


bool Parser::parse(const StreamPtr& pInput,
                   const ProjectSPtr& project)
{
    ProjectParseContextSPtr context = boost::make_shared<ProjectParseContext>();
    context->mProject = boost::make_shared<Project>();
    context->mTokenizer = boost::make_shared<Tokenizer>(mpMessageCollector, mpSourceId, pInput);
    
    FileSPtr file = parseFile();
    if (!file)
        return false;
        
    if (!context->mProject->mainFile())
        context->mProject->set_mainFile(file);
        
    CommentSPtr pStatementComment = lastComment();
    while (mContext->mTokenizer->current())
    {
        if (mContext->mTokenizer->check(Token::TYPE_IDENTIFIER))
        {
            parseProjectStatement(context, pStatementComment);
        }
        else
        {
            mContext->mTokenizer->shift();
        }
        pStatementComment = lastComment();
    }

    
    return true;
}

bool Parser::eof()
{
    if (mContext->mTokenizer->current())
        return false;

    return true;
}

bool Parser::expect(Token::Type type)
{
    return expect(type, NULL);
}

bool Parser::expect(Token::Type type, const char* text)
{
    if (eof())
        return false;

    if (mContext->mTokenizer->check(type, text))
        return true;

    return false;
}

void Parser::recover()
{
    for (;;)
    {
        if (!mContext->mTokenizer->current())
            return;

        if (mContext->mTokenizer->check(Token::TYPE_BRACKET, "}"))
            break;

        //if (check(Token::TYPE_DELIMITER))
        //    break;

        mContext->mTokenizer->shift();
    }
}

void Parser::lateTypeResolve(const TypeSPtr& pNewType)
{
    std::vector<LateTypeResolveInfo>::iterator it = mLateTypeResolve.begin();
    while (it != mLateTypeResolve.end())
    {
        if (it->pToken->text() == pNewType->name()->value())
        {
            it->initTypeMethod(pNewType);
            it = mLateTypeResolve.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

Message Parser::errorMessage(const char* message, const Line& line, const Column& column)
{
    Line theLine = line;
    if (theLine == Line(-1))
    {
        theLine = mContext->mTokenizer->current()
                ? mContext->mTokenizer->current()->line()
                : mContext->mTokenizer->line();
    }

    Column theColumn = column;
    if (theColumn == Column(-1))
    {
        theColumn = mContext->mTokenizer->current()
                  ? mContext->mTokenizer->current()->beginColumn()
                  : mContext->mTokenizer->column();
    }

    return Message(Message::SEVERITY_ERROR, message, mpSourceId, theLine, theColumn);
}

Message Parser::warningMessage(const char* message, const Line& line, const Column& column)
{
    Line theLine = line;
    if (theLine == Line(-1))
    {
        theLine = mContext->mTokenizer->current()
                ? mContext->mTokenizer->current()->line()
                : mContext->mTokenizer->line();
    }

    Column theColumn = column;
    if (theColumn == Column(-1))
    {
        theColumn = mContext->mTokenizer->current()
                  ? mContext->mTokenizer->current()->beginColumn()
                  : mContext->mTokenizer->column();
    }

    return Message(Message::SEVERITY_WARNING, message, mpSourceId, theLine, theColumn);
}

Parser& Parser::operator<<(const Message& message)
{
    assert(message.text().find("%") == std::string::npos);
    mpMessageCollector->addMessage(message);
	return *this;
}

void Parser::initilizeObject(ObjectSPtr pObject, const TokenPtr& pToken)
{
    TokenPtr token = pToken ? pToken : mContext->mTokenizer->current();
    pObject->set_sourceId(mpSourceId);
    pObject->set_line(token->line());
    pObject->set_column(token->beginColumn());
}

bool Parser::validate(const DocumentSPtr&)
{
    return true;
}

bool Parser::validate(const ObjectSPtr& pObject)
{
    bool bResult = true;

    for (std::vector<ValidatorPtr>::iterator it = mvValidator.begin(); it != mvValidator.end(); ++it)
    {
        ValidatorPtr& pValidator = *it;

        if (!pValidator->validate(pObject, mpMessageCollector))
            bResult = false;
    }

    return bResult;
}

Token::Type Parser::getTokenType(const Type::ELiteral& literal)
{
    switch (literal.value())
    {
        case Type::ELiteral::kBoolean:
        case Type::ELiteral::kReference:
        case Type::ELiteral::kIdentifier:
            return Token::TYPE_IDENTIFIER;
        case Type::ELiteral::kInteger:
            return Token::TYPE_INTEGER_LITERAL;
        case Type::ELiteral::kReal:
            return Token::TYPE_REAL_LITERAL;
        case Type::ELiteral::kString:
            return Token::TYPE_STRING_LITERAL;
        case Type::ELiteral::kBinary:
            break;
        default:
            assert(false);
    }
    return Token::TYPE_INVALID;
}

}
