// Boost C++ Utility
#include <boost/assert.hpp>

#include "decorated_type.h"

namespace cpp
{

namespace frm
{

DecoratedType::DecoratedType()
        : mDeclaration(default_declaration())
        , mDecoration (default_decoration())
{
}

DecoratedType::~DecoratedType()
{
}

const ETypeDeclaration& DecoratedType::declaration() const
{
    return mDeclaration;
}

ETypeDeclaration DecoratedType::default_declaration()
{
    return ETypeDeclaration::invalid();
}

DecoratedType& DecoratedType::set_declaration(const ETypeDeclaration& declaration)
{
    mDeclaration = declaration;
    return *this;
}

ETypeDeclaration& DecoratedType::mutable_declaration()
{
    return mDeclaration;
}

DecoratedType& DecoratedType::operator<<(const ETypeDeclaration& declaration)
{
    return set_declaration(declaration);
}

const DecoratedTypeSPtr& operator<<(const DecoratedTypeSPtr& object, const ETypeDeclaration& declaration)
{
    BOOST_ASSERT(object);
    *object << declaration;
    return object;
}

const SimpleTypeSPtr& DecoratedType::type() const
{
    return mType;
}

DecoratedType& DecoratedType::set_type(const SimpleTypeSPtr& type)
{
    mType = type;
    return *this;
}

DecoratedType& DecoratedType::operator<<(const SimpleTypeSPtr& type)
{
    return set_type(type);
}

const DecoratedTypeSPtr& operator<<(const DecoratedTypeSPtr& object, const SimpleTypeSPtr& type)
{
    BOOST_ASSERT(object);
    *object << type;
    return object;
}

const ETypeDecoration& DecoratedType::decoration() const
{
    return mDecoration;
}

ETypeDecoration DecoratedType::default_decoration()
{
    return ETypeDecoration::invalid();
}

DecoratedType& DecoratedType::set_decoration(const ETypeDecoration& decoration)
{
    mDecoration = decoration;
    return *this;
}

ETypeDecoration& DecoratedType::mutable_decoration()
{
    return mDecoration;
}

DecoratedType& DecoratedType::operator<<(const ETypeDecoration& decoration)
{
    return set_decoration(decoration);
}

const DecoratedTypeSPtr& operator<<(const DecoratedTypeSPtr& object, const ETypeDecoration& decoration)
{
    BOOST_ASSERT(object);
    *object << decoration;
    return object;
}

}

}
