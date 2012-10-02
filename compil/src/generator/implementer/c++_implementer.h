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


#ifndef _CPP_IMPLEMENTER_H__
#define _CPP_IMPLEMENTER_H__

#include "cpp/format/destructor_specifier.h"

#include "implementer_configuration.h"
#include "c++_formatter.h"
#include "dependency.h"
#include "factory.h"
#include "model.h"
#include "method_specifier.h"

namespace compil
{

extern cpp::frm::NamespaceSPtr nsStd;
extern cpp::frm::NamespaceSPtr nsBoost;
extern cpp::frm::NamespaceSPtr nsBoostPosixTime;

class CppImplementer
{
public:
    CppImplementer(const ImplementerConfigurationPtr& pConfig, const CppFormatterPtr& pFrm);
    virtual ~CppImplementer();
    
    virtual bool needMutableMethod(const FieldSPtr& pField, const StructureSPtr& pCurrentStructure);
    virtual bool needConstructorInitialization(const FieldSPtr& pField);
    
    virtual cpp::frm::SimpleTypeSPtr cppType(const TypeSPtr& pType);
    virtual cpp::frm::SimpleTypeSPtr cppInnerType(const TypeSPtr& pType,
                                                  const StructureSPtr& pStructure);
    
    virtual cpp::frm::DecoratedTypeSPtr cppDecoratedType(const TypeSPtr& pType);
    virtual cpp::frm::DecoratedTypeSPtr cppSetDecoratedType(const TypeSPtr& pType);
    virtual cpp::frm::DecoratedTypeSPtr cppInnerDecoratedType(const TypeSPtr& pType,
                                                    const StructureSPtr& pStructure);
    virtual cpp::frm::DecoratedTypeSPtr cppInnerSetDecoratedType(const TypeSPtr& pType,
                                                       const StructureSPtr& pStructure);
    
    virtual std::string cppGetReturn(const FieldSPtr& pField);
    
    virtual std::vector<Dependency> classPointerDependencies();
    virtual std::vector<Dependency> classReferenceDependencies();
    
    virtual std::vector<Dependency> dependencies(const TypeSPtr& pType);
    virtual std::vector<Dependency> dependencies(const FieldSPtr& pField);
    
    virtual cpp::frm::SimpleTypeSPtr cppPtrType(const TypeSPtr& pType);
    virtual cpp::frm::DecoratedTypeSPtr cppPtrDecoratedType(const TypeSPtr& pType);
    
    virtual std::string cppNullPtr(const TypeSPtr& pType);
    virtual std::string cppConvertRawPtr(const TypeSPtr& pType, const std::string& variable);
    
    virtual std::string null();
    virtual Dependency nullDependency();
    
    virtual std::string assert_method();
    virtual Dependency assert_dependency();
    
    virtual Dependency unordered_set_dependency();
    virtual Dependency unordered_map_dependency();
    
    static bool alphabeticByName(const StructureSPtr& pStructure1, const StructureSPtr& pStructure2);
    
    typedef bool (*if_predicate)(const StructureSPtr& pStructure);
    typedef bool (*less_predicate)(const StructureSPtr& pStructure1, const StructureSPtr& pStructure2);
    virtual std::vector<StructureSPtr> hierarchie(const ModelPtr& pModel, 
                                                  const StructureSPtr& pBaseStructure,
                                                  if_predicate if_pred = NULL,
                                                  less_predicate less_pred = alphabeticByName);
                                                 
    virtual MethodNameSPtr staticMethodName(const std::string& name);
    virtual MethodNameSPtr runtimeMethodName(const std::string& name);
    
    virtual std::string identificationName(const StructureSPtr& pStructure);
    virtual cpp::frm::SimpleTypeSPtr identificationEnum(const StructureSPtr& pStructure);
    virtual MethodNameSPtr staticIdentificationMethodName(const StructureSPtr& pStructure);
    virtual MethodNameSPtr runtimeIdentificationMethodName(const StructureSPtr& pStructure);
    
    virtual EnumerationSPtr objectEnumeration(const ModelPtr& pModel, const FactorySPtr& pFactory);
    virtual EnumerationSPtr objectEnumeration(const ModelPtr& pModel,
                                              const std::vector<StructureSPtr>& structs,
                                              const FactorySPtr& pFactory);
  
    virtual EMethodSpecifier methodSpecifier(const StructureSPtr& pStructure);
    virtual cpp::frm::EDestructorSpecifier destructorSpecifier(const StructureSPtr& pStructure);
                                     
    cpp::frm::ConstructorNameSPtr inheritClass(const EnumerationSPtr& pEnumeration,
                                               const StructureSPtr& pStructure);
    virtual bool implementEnumerationMethods(const EnumerationSPtr& pEnumeration);
    virtual bool implementFlagsEnumerationMethods(const EnumerationSPtr& pEnumeration);
                             
    virtual bool boost_smart_ptr_needed();
    virtual cpp::frm::SimpleTypeSPtr boost_shared_ptr(const cpp::frm::SimpleTypeSPtr& type);
    virtual cpp::frm::SimpleTypeSPtr boost_shared_const_ptr(const cpp::frm::SimpleTypeSPtr& type);
    virtual cpp::frm::SimpleTypeSPtr boost_weak_ptr(const cpp::frm::SimpleTypeSPtr& type);
    virtual cpp::frm::SimpleTypeSPtr boost_enable_shared_from_this(const cpp::frm::SimpleTypeSPtr& type);
    
    virtual std::string applicationExtension();
    virtual std::string applicationHeaderExtension();
    
    virtual Dependency cppHeaderFileDependency(const ObjectSPtr& object);
    
    ImplementerConfigurationPtr mpConfiguration;
    
private:
    CppFormatterPtr mpFrm;
};

typedef boost::shared_ptr<CppImplementer> CppImplementerPtr;

}

#else

namespace compil
{

class CppImplementer;
typedef boost::shared_ptr<CppImplementer> CppImplementerPtr;

}

#endif

