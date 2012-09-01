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

#include "c++_implementer.h"
#include "aligner.h"
#include "object_factory.h"

#include <sstream>

namespace compil
{

CppImplementer::CppImplementer(const ImplementerConfigurationPtr& pConfig, const CppFormatterPtr& pFrm)
        : mpConfiguration(pConfig)
        , mpFrm(pFrm)
{
}

CppImplementer::~CppImplementer()
{
}

bool CppImplementer::needMutableMethod(const TypeSPtr& pType)
{
    switch (pType->kind().value())
    {
        case Type::EKind::kBuiltin:
            return false;
        case Type::EKind::kObject:
            if (ObjectFactory::downcastReference(pType))
                return false;
            return true;
        case Type::EKind::kString:
            switch (mpConfiguration->mString)
            {
                case ImplementerConfiguration::use_char_pointer:
                    return false;
                case ImplementerConfiguration::use_stl_string:
                    return true;
                default: assert(false && "unknown string implementation type");
            }
            break;
        case Type::EKind::kInvalid:
        default:
            assert(false && "unknown kind");
    }
    return false;
}

bool CppImplementer::needConstructorInitialization(const FieldSPtr& pField)
{
    if (!pField->defaultValue())
        return false;
    if (pField->defaultValue()->optional())
        return false;
    if (pField->defaultValue()->value() == "null")
    if (mpConfiguration->mPointer == ImplementerConfiguration::use_boost_pointers)
        return false;
    return true;
}

DecoratedType CppImplementer::cppDecoratedType(const TypeSPtr& pType)
{
    switch (pType->kind().value())
    {
        case Type::EKind::kBuiltin:
            return *CreateDecoratedType(cppType(pType));
        case Type::EKind::kObject:
            return *CreateDecoratedType(cst, cppType(pType), ref);
        case Type::EKind::kString:
            switch (mpConfiguration->mString)
            {
                case ImplementerConfiguration::use_char_pointer:
                    return *CreateDecoratedType(cst, *CreateSimpleType("char"), ptr);
                case ImplementerConfiguration::use_stl_string:
                    return *CreateDecoratedType(cst, *CreateSimpleType("std::string"), ref);
                default: assert(false && "unknown string implementation type");
            }
            break;
        case Type::EKind::kInvalid:
        default:
            assert(false && "unknown kind");
    }
	return *CreateDecoratedType(*CreateSimpleType("@@@"), *CreateDecoration("&&&"));
}

DecoratedType CppImplementer::cppSetDecoratedType(const TypeSPtr& pType)
{
    ReferenceSPtr pReference = ObjectFactory::downcastReference(pType);
    if (pReference)
        return *CreateDecoratedType(cst, mpFrm->cppSharedPtrName(pReference->parameterType().lock()), ref);
    return cppDecoratedType(pType);
}

DecoratedType CppImplementer::cppInnerDecoratedType(const TypeSPtr& pType,
                                                    const StructureSPtr& pStructure)
{
    if (pType->runtimeObjectId() == EObjectId::enumeration())
    {
        EnumerationSPtr pEnumeration = ObjectFactory::downcastEnumeration(pType);
        return *CreateDecoratedType(cst, mpFrm->cppInnerEnumType(pEnumeration, pStructure), ref);
    }
    return cppDecoratedType(pType);
}

DecoratedType CppImplementer::cppInnerSetDecoratedType(const TypeSPtr& pType,
                                                       const StructureSPtr& pStructure)
{
    ReferenceSPtr pReference = ObjectFactory::downcastReference(pType);
    if (pReference)
        return *CreateDecoratedType(cst, mpFrm->cppSharedPtrName(pReference->parameterType().lock()), ref);
    return cppInnerDecoratedType(pType, pStructure);
}

std::string CppImplementer::cppGetReturn(const FieldSPtr& pField)
{
    ReferenceSPtr pReference = ObjectFactory::downcastReference(pField->type());
    return mpFrm->cppMemberName(pField);
}

SimpleType CppImplementer::cppType(const TypeSPtr& pType)
{
    ReferenceSPtr pReference = ObjectFactory::downcastReference(pType);
    if (pReference)
    {
        if (pReference->weak())
            return mpFrm->cppWeakPtrName(pReference->parameterType().lock());
        else
            return mpFrm->cppSharedPtrName(pReference->parameterType().lock());
    }

    std::string name = pType->name()->value();
    if (!pType->package())
    {
        if (name == "boolean")
            return *CreateSimpleType("bool");
            
        switch (mpConfiguration->mIntegerTypes)
        {
            case ImplementerConfiguration::use_native:
                if (name == "size")
                    return *CreateSimpleType("size_t");
                if (name == "small")
                    return *CreateSimpleType("char");
                if (name == "short")
                    return *CreateSimpleType("short");
                if (name == "integer")
                    return *CreateSimpleType("long");
                if (name == "long")
                    return *CreateSimpleType("long long");
                if (name == "byte")
                    return *CreateSimpleType("unsigned char");
                if (name == "word")
                    return *CreateSimpleType("unsigned short");
                if (name == "dword")
                    return *CreateSimpleType("unsigned long");
                if (name == "qword")
                    return *CreateSimpleType("unsigned long long");
                break;
            case ImplementerConfiguration::use_intnn_t:
                if (name == "size")
                    return *CreateSimpleType("size_t");
                if (name == "small")
                    return *CreateSimpleType("int8_t");
                if (name == "short")
                    return *CreateSimpleType("int16_t");
                if (name == "integer")
                    return *CreateSimpleType("int32_t");
                if (name == "long")
                    return *CreateSimpleType("int64_t");
                if (name == "byte")
                    return *CreateSimpleType("uint8_t");
                if (name == "word")
                    return *CreateSimpleType("uint16_t");
                if (name == "dword")
                    return *CreateSimpleType("uint32_t");
                if (name == "qword")
                    return *CreateSimpleType("uint64_t");
                break;
            case ImplementerConfiguration::use_boost_intnn_t:
                if (name == "size")
                    return *CreateSimpleType("size_t");
                if (name == "small")
                    return *CreateSimpleType("boost::int8_t");
                if (name == "short")
                    return *CreateSimpleType("boost::int16_t");
                if (name == "integer")
                    return *CreateSimpleType("boost::int32_t");
                if (name == "long")
                    return *CreateSimpleType("boost::int64_t");
                if (name == "byte")
                    return *CreateSimpleType("boost::uint8_t");
                if (name == "word")
                    return *CreateSimpleType("boost::uint16_t");
                if (name == "dword")
                    return *CreateSimpleType("boost::uint32_t");
                if (name == "qword")
                    return *CreateSimpleType("boost::uint64_t");
                break;
            default:
                break;
        }
        if (name == "string")
        {
            switch (mpConfiguration->mString)
            {
                case ImplementerConfiguration::use_char_pointer: 
                    return *CreateSimpleType("const char*");
                case ImplementerConfiguration::use_stl_string: 
                    return *CreateSimpleType("std::string");
                default: assert(false && "unknown string implementation type");
            }
        }
        if (name == "vector")
        {
            UnaryContainerSPtr pUnaryContainer = boost::static_pointer_cast<UnaryContainer>(pType);
            SimpleType simpleType = cppType(pUnaryContainer->parameterType().lock());
            std::string result = "std::vector<";
            if (simpleType.exist_namespace_())
            if (!simpleType.namespace_().isVoid())
                result += simpleType.namespace_().value() + "::";
            result += simpleType.value() + ">"; 
            return *CreateSimpleType(result);
        }
    }
    
    if (pType->package())
    {
        if (name == "datetime")
        {
            std::vector<std::string> elements;
            elements.push_back("time");
            
            if (pType->package()->elements() == elements)
            {
                return *CreateSimpleType("boost::posix_time::ptime");
            }
        }
    }
    
    if (pType->runtimeObjectId() == EObjectId::structure())
        return *CreateSimpleType(mpFrm->cppPackageNamespace(pType->package()), 
                                          mpFrm->cppClassName(name));
    if (pType->runtimeObjectId() == EObjectId::enumeration())
        return mpFrm->cppEnumType(ObjectFactory::downcastEnumeration(pType));
    if (pType->runtimeObjectId() == EObjectId::specimen())
        return *CreateSimpleType(mpFrm->cppClassName(name));
    if (pType->runtimeObjectId() == EObjectId::identifier())
        return *CreateSimpleType(mpFrm->cppClassName(name));
        
    return *CreateSimpleType(mpFrm->cppPackageNamespace(pType->package()), name);
}

SimpleType CppImplementer::cppInnerType(const TypeSPtr& pType,
                                        const StructureSPtr& pStructure)
{
    if (pType->runtimeObjectId() == EObjectId::enumeration())
    {
        EnumerationSPtr pEnumeration = ObjectFactory::downcastEnumeration(pType);
        return mpFrm->cppInnerEnumType(pEnumeration, pStructure);
    }
    return cppType(pType);
}

std::vector<Dependency> CppImplementer::classPointerDependencies()
{
    std::vector<Dependency> dep;
    
    if (mpConfiguration->mPointer == ImplementerConfiguration::use_boost_pointers)
    {
        dep.push_back(
            Dependency("boost/shared_ptr.hpp", 
                        Dependency::system_type, 
                        Dependency::boost_level,
                        Dependency::global_section,
                        "Boost C++ Smart Pointers"));

        dep.push_back(
            Dependency("boost/weak_ptr.hpp", 
                        Dependency::system_type, 
                        Dependency::boost_level,
                        Dependency::global_section,
                        "Boost C++ Smart Pointers"));
    }
    
    return dep;
}

std::vector<Dependency> CppImplementer::classReferenceDependencies()
{
    std::vector<Dependency> dep;
    
    if (mpConfiguration->mPointer == ImplementerConfiguration::use_boost_pointers)
    {
        dep.push_back(
            Dependency("boost/make_shared.hpp",
                        Dependency::system_type, 
                        Dependency::boost_level,
                        Dependency::global_section,
                        "Boost C++ Smart Pointers"));
    }
    
    return dep;
}

std::vector<Dependency> CppImplementer::dependencies(const TypeSPtr& pType)
{
    std::vector<Dependency> dep;
    
    dep.push_back(cppHeaderFileDependency(pType));
    
    UnaryTemplateSPtr pUnaryTemplate = ObjectFactory::downcastUnaryTemplate(pType);
    if (pUnaryTemplate)
        dep.push_back(cppHeaderFileDependency(pUnaryTemplate->parameterType().lock()));
    
    IntegerSPtr pInteger = ObjectFactory::downcastInteger(pType);
    
    if (pInteger)
    {
        if (mpConfiguration->mIntegerTypes == ImplementerConfiguration::use_intnn_t)
        {
            dep.push_back(
                Dependency("sys/types.h",
                           Dependency::system_type, 
                           Dependency::system_level,
                           Dependency::private_section, 
                           "Standard C Library"));   
        }
        else
        if (mpConfiguration->mIntegerTypes == ImplementerConfiguration::use_boost_intnn_t)
        {
            dep.push_back(
                Dependency("boost/cstdint.hpp",
                           Dependency::system_type,
                           Dependency::boost_level,
                           Dependency::global_section,
                           "Boost C++ Smart Pointers"));
        }
    }
    
    EnumerationSPtr pEnumeration = ObjectFactory::downcastEnumeration(pType);
    if (pEnumeration)
    {
        if (pEnumeration->flags())
        if (mpConfiguration->mFlagsEnumeration == ImplementerConfiguration::flags_enumeration_use_core_template)
        {
            dep.push_back(
                Dependency("flags_enumeration.hpp",
                           Dependency::quote_type,
                           Dependency::core_level,
                           Dependency::private_section,
                           "Compil C++ Template Library"));
        }
    }

    std::string name = pType->name()->value();
    if (!pType->package())
    {
        if (name == "string")
        {
            if (mpConfiguration->mString == ImplementerConfiguration::use_stl_string)
            {
                dep.push_back(
                    Dependency("string",
                               Dependency::system_type, 
                               Dependency::stl_level,
                               Dependency::global_section, 
                               "Standard Template Library"));
            }
        }
        
        else
        if (name == "vector")
        {
            dep.push_back(
                Dependency("vector",
                           Dependency::system_type,
                           Dependency::stl_level,
                           Dependency::global_section,
                           "Standard Template Library"));
                           
            UnaryContainerSPtr pUnaryContainer = boost::static_pointer_cast<UnaryContainer>(pType);
            std::vector<Dependency> subdep = dependencies(pUnaryContainer->parameterType().lock());
            
            dep.insert(dep.begin(), subdep.begin(), subdep.end());
        }
    }
    
    if (pType->package())
    {
        std::vector<std::string> elements;
        elements.push_back("time");
        
        if (pType->package()->elements() == elements)
        {
            dep.push_back(
                Dependency("boost/date_time/posix_time/posix_time.hpp",
                           Dependency::system_type,
                           Dependency::boost_level,
                           Dependency::private_section,
                           "Boost Posix Time System"));
        }
    }
    return dep;
}

std::vector<Dependency> CppImplementer::dependencies(const FieldSPtr& pField)
{
    return dependencies(pField->type());
}

SimpleType CppImplementer::cppPtrType(const TypeSPtr& pType)
{
    switch (mpConfiguration->mPointer)
    {
        case ImplementerConfiguration::use_raw_pointers: 
            return mpFrm->cppRawPtrName(pType);
        case ImplementerConfiguration::use_boost_pointers: 
            return mpFrm->cppSharedPtrName(pType);
        default: assert(false && "unknown pointer type"); break;
    }
    return *CreateSimpleType("");
}

DecoratedType CppImplementer::cppPtrDecoratedType(const TypeSPtr& pType)
{
    switch (mpConfiguration->mPointer)
    {
        case ImplementerConfiguration::use_raw_pointers: 
            return mpFrm->cppRawPtrDecoratedType(pType);
        case ImplementerConfiguration::use_boost_pointers: 
            return mpFrm->cppSharedPtrDecoratedType(pType);
        default: assert(false && "unknown pointer type"); break;
    }
    return *CreateDecoratedType(*CreateSimpleType(""));
}

std::string CppImplementer::cppNullPtr(const TypeSPtr& pType)
{
    switch (mpConfiguration->mPointer)
    {
        case ImplementerConfiguration::use_raw_pointers: 
            return null();
        case ImplementerConfiguration::use_boost_pointers: 
            return mpFrm->cppSharedPtrName(pType).value() + "()";
        default: assert(false && "unknown pointer type"); break;
    }
    return "";
}

std::string CppImplementer::cppConvertRawPtr(const TypeSPtr& pType, const std::string& variable)
{
    switch (mpConfiguration->mPointer)
    {
        case ImplementerConfiguration::use_raw_pointers: 
            return variable;
        case ImplementerConfiguration::use_boost_pointers: 
            return mpFrm->cppSharedPtrName(pType).value() + "(" + variable + ")";
        default: assert(false && "unknown pointer type"); break;
    }
    return "";
}

std::string CppImplementer::null()
{
    switch (mpConfiguration->mNullOr0)
    {
        case ImplementerConfiguration::use_null: return "NULL";
        case ImplementerConfiguration::use_0: return "0";
        default: assert(false && "unknown Null or 0 type"); break;
    }
    return "";
}

Dependency CppImplementer::nullDependency()
{
    if (mpConfiguration->mNullOr0 == ImplementerConfiguration::use_null)
    {
        return Dependency("stddef.h", 
                          Dependency::system_type, 
                          Dependency::system_level, 
                          Dependency::private_section,
                          "Standard C Library");
    }
    return Dependency();
}


std::string CppImplementer::assert_method()
{
    switch (mpConfiguration->mAssert)
    {
        case ImplementerConfiguration::use_std_assert: return "assert";
        case ImplementerConfiguration::use_boost_assert: return "BOOST_ASSERT";
        default: assert(false && "unknown assert type"); break;
    }
    return "";
}

Dependency CppImplementer::assert_dependency()
{
    switch (mpConfiguration->mAssert)
    {
        case ImplementerConfiguration::use_std_assert:
            return Dependency("assert.h",
                              Dependency::system_type, 
                              Dependency::system_level,
                              Dependency::private_section,
                              "Standard C library");
        case ImplementerConfiguration::use_boost_assert:
            return Dependency("boost/assert.hpp", 
                              Dependency::system_type, 
                              Dependency::boost_level,
                              Dependency::global_section,
                              "Boost C++ Utility");
        default: assert(false && "unknown assert type"); break;
    }
    return Dependency();
}

Dependency CppImplementer::unordered_set_dependency()
{
    return Dependency("boost/unordered_set.hpp", 
                      Dependency::system_type, 
                      Dependency::boost_level,
                      Dependency::private_section,
                      "Boost C++ Unordered");
}
Dependency CppImplementer::unordered_map_dependency()
{
    return Dependency("boost/unordered_map.hpp", 
                      Dependency::system_type, 
                      Dependency::boost_level,
                      Dependency::private_section,
                      "Boost C++ Unordered");
}

bool CppImplementer::alphabeticByName(const StructureSPtr& pStructure1, const StructureSPtr& pStructure2)
{
    return pStructure1->name()->value() < pStructure2->name()->value();
}

std::vector<StructureSPtr> CppImplementer::hierarchie(const ModelPtr& pModel, 
                                                      const StructureSPtr& pBaseStructure,
                                                      if_predicate if_pred,
                                                      less_predicate less_pred)
{
    std::vector<StructureSPtr> hierarchie;
    const std::vector<ObjectSPtr>& objects = pModel->objects();
    std::vector<ObjectSPtr>::const_iterator it;
    for (it = objects.begin(); it != objects.end(); ++it)
    {
        StructureSPtr pStructure = ObjectFactory::downcastStructure(*it);
        if (!pStructure) 
            continue;
        
        if (pStructure != pBaseStructure)
        if (!pStructure->isRecursivelyInherit(pBaseStructure))
            continue;
            
        if (if_pred)
        if (!if_pred(pStructure))
            continue;
            
        hierarchie.push_back(pStructure);
    }
    
    if (less_pred)
        std::sort(hierarchie.begin(), hierarchie.end(), less_pred);
    
    return hierarchie;
}

FunctionName CppImplementer::staticMethodName(const std::string& name)
{
    std::string uname = name;
    std::transform(uname.begin(), uname.begin() + 1, uname.begin(), toupper);
    return mpFrm->methodName("static" + uname);
}

FunctionName CppImplementer::runtimeMethodName(const std::string& name)
{
    std::string uname = name;
    std::transform(uname.begin(), uname.begin() + 1, uname.begin(), toupper);
    return mpFrm->methodName("runtime" + uname);
}

std::string CppImplementer::identificationName(const StructureSPtr& pStructure)
{
    return pStructure->name()->value() + "Id";
}

SimpleType CppImplementer::identificationEnum(const StructureSPtr& pStructure)
{
    return *CreateSimpleType(mpFrm->enumName(identificationName(pStructure)));
}

FunctionName CppImplementer::staticIdentificationMethodName(const StructureSPtr& pStructure)
{
    return staticMethodName(identificationName(pStructure));
}

FunctionName CppImplementer::runtimeIdentificationMethodName(const StructureSPtr& pStructure)
{
    return runtimeMethodName(identificationName(pStructure));
}

EnumerationSPtr CppImplementer::objectEnumeration(const ModelPtr& pModel, const FactorySPtr& pFactory)
{
    std::vector<StructureSPtr> structs = 
        hierarchie(pModel, 
                   ObjectFactory::downcastStructure(pFactory->parameterType().lock()),
                   &Structure::hasRuntimeIdentification);
    return objectEnumeration(pModel, structs, pFactory);
}

EnumerationSPtr CppImplementer::objectEnumeration(const ModelPtr& pModel,
                                                  const std::vector<StructureSPtr>& structs, 
                                                  const FactorySPtr& pFactory)
{
    SourceIdSPtr pSourceId(new SourceId());
    
    EnumerationSPtr pEnumeration(new Enumeration());
    //pEnumeration->setComment(pComment);
    
    pEnumeration->set_sourceId(pSourceId);
    pEnumeration->set_line(-1);
    pEnumeration->set_column(-1);
    pEnumeration->set_cast(CastableType::ECast::strong());
    pEnumeration->set_flags(false);
    
    std::vector<std::string> package_elements;
    pEnumeration->set_parameterType(pModel->findType(PackageSPtr(), package_elements, "integer"));
    
    NameSPtr pName(new Name());
    pName->set_sourceId(pSourceId);
    pName->set_line(-1);
    pName->set_column(-1);
    pName->set_value(identificationName(ObjectFactory::downcastStructure(pFactory->parameterType().lock())));    
    pEnumeration->set_name(pName);
    
    int mEnumValue = 1; // 0 reserved for invalid
    
    std::vector<EnumerationValueSPtr> enumerationValues;
    
    std::vector<StructureSPtr>::const_iterator it;
    for (it = structs.begin(); it != structs.end(); ++it)
    {
        StructureSPtr pStructure = *it;

        AbsoluteEnumerationValueSPtr pEnumerationValue(new AbsoluteEnumerationValue());
        //pEnumerationValue->setComment(pComment);
        
        pEnumerationValue->set_sourceId(pSourceId);
        pEnumerationValue->set_line(-1);
        pEnumerationValue->set_column(-1);

        NameSPtr pName(new Name());
        pName->set_sourceId(pSourceId);
        pName->set_line(-1);
        pName->set_column(-1);
        pName->set_value(pStructure->name()->value());
    
        pEnumerationValue->set_name(pName);
        pEnumerationValue->set_value(mEnumValue++);
        pEnumerationValue->set_enumeration(pEnumeration);
        
        enumerationValues.push_back(pEnumerationValue);
    }
    
    pEnumeration->set_enumerationValues(enumerationValues);
    
    return pEnumeration;
}

Modifier CppImplementer::virtualModifier(const StructureSPtr& pStructure, const Modifier& defaultModifier)
{
    StructureSPtr pStruct = pStructure;
    while (pStruct)
    {
        if (pStruct->hasRuntimeIdentification())
            return vrtl;
        
        pStruct = pStruct->baseStructure().lock();
    }

    return defaultModifier;
}


std::string CppImplementer::inheritClass(const EnumerationSPtr& pEnumeration,
                                         const StructureSPtr& pStructure)
{
    TypeSPtr pParameterType = pEnumeration->parameterType().lock();
    
    if (pEnumeration->flags())
    if (mpConfiguration->mFlagsEnumeration == ImplementerConfiguration::flags_enumeration_use_core_template)
        return "flags_enumeration<" + 
               cppType(pParameterType).value() +
               ", " +
               mpFrm->cppInnerEnumType(pEnumeration, pStructure).value() +
               ">";
        
    return "";
}

bool CppImplementer::implementEnumerationMethods(const EnumerationSPtr& pEnumeration)
{
    if (pEnumeration->flags())
    if (mpConfiguration->mFlagsEnumeration == ImplementerConfiguration::flags_enumeration_use_core_template)
        return false;
        
    return true;
}

bool CppImplementer::implementFlagsEnumerationMethods(const EnumerationSPtr& pEnumeration)
{
    if (!pEnumeration->flags())
        return false;

    if (mpConfiguration->mFlagsEnumeration != ImplementerConfiguration::flags_enumeration_inline_implementation)
        return false;

    return true;
}

bool CppImplementer::hasOperator(const TypeSPtr& pType,
                                 const EOperatorAction& action,
                                 const EOperatorFlags& flags) const
{
    StructureSPtr pStructure = ObjectFactory::downcastStructure(pType);
    if (pStructure)
        return pStructure->hasOperator(action, flags);
        
    if (action == EOperatorAction::lessThan())
    {
        EnumerationSPtr pEnumeration = ObjectFactory::downcastEnumeration(pType);
        if (pEnumeration && pEnumeration->cast() == Enumeration::ECast::strong())
            return false;
    }
    return true;
}

bool CppImplementer::boost_smart_ptr_needed()
{
    return mpConfiguration->mPointer == ImplementerConfiguration::use_boost_pointers;
}

SimpleType CppImplementer::boost_shared_ptr(const SimpleType& type)
{
    return *CreateSimpleType("boost::shared_ptr<" + type.value() + ">");
}

SimpleType CppImplementer::boost_shared_const_ptr(const SimpleType& type)
{
    return *CreateSimpleType("boost::shared_ptr<const " + type.value() + ">");
}

SimpleType CppImplementer::boost_weak_ptr(const SimpleType& type)
{
    return *CreateSimpleType("boost::weak_ptr<" + type.value() + ">");
}

SimpleType CppImplementer::boost_enable_shared_from_this(const SimpleType& type)
{
    return *CreateSimpleType("boost::enable_shared_from_this<" + type.value() + ">");
}

std::string CppImplementer::applicationExtension()
{
    switch (mpConfiguration->applicationCppExtension)
    {
        case ImplementerConfiguration::use_cpp: return ".cpp";
        case ImplementerConfiguration::use_cxx: return ".cxx";
        default : assert(false && "unknown applicationCppExtension");
    }
    return "";
}

std::string CppImplementer::applicationHeaderExtension()
{
    switch (mpConfiguration->applicationCppHeaderExtension)
    {
        case ImplementerConfiguration::use_h: return ".h";
        case ImplementerConfiguration::use_hpp: return ".hpp";
        default : assert(false && "unknown applicationCppHeaderExtension");
    }
    return "";
}

Dependency CppImplementer::cppHeaderFileDependency(const ObjectSPtr& object)
{
    if (!object)
        return Dependency();
        
    SourceIdSPtr sourceId = object->sourceId();
    if (!sourceId)
        return Dependency();    
    
    
    std::string source = sourceId->original();

    std::string compil_ext = ".compil";
    std::string::size_type where = source.find(compil_ext);
    if (where == std::string::npos)
        return Dependency();
       
    std::string h_ext = applicationHeaderExtension(); 
    std::string result = source;
    result.replace(result.begin() + where, result.begin() + ((size_t)where + compil_ext.size()), 
                   h_ext.begin(), h_ext.end());
                   
    return Dependency(result, Dependency::quote_type, Dependency::application_level);
}

}
