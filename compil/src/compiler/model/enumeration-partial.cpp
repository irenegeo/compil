#include "enumeration.h"

namespace compil
{

EnumerationPartial::EnumerationPartial()
{
    set_literal(alter_literal());
    set_kind(alter_kind());
}

EnumerationPartial::~EnumerationPartial()
{
}

EnumerationSPtr EnumerationPartial::downcast(const ObjectSPtr& pObject)
{
    return boost::static_pointer_cast<Enumeration>(pObject);
}

Type::ELiteral EnumerationPartial::alter_literal()
{
    return Type::ELiteral::identifier();
}

Type::EKind EnumerationPartial::alter_kind()
{
    return Type::EKind::object();
}

bool EnumerationPartial::flags() const
{
    return mFlags;
}

void EnumerationPartial::set_flags(bool flags)
{
    mFlags = flags;
}

const std::vector<EnumerationValueSPtr>& EnumerationPartial::enumerationValues() const
{
    return mEnumerationValues;
}

void EnumerationPartial::set_enumerationValues(const std::vector<EnumerationValueSPtr>& enumerationValues)
{
    mEnumerationValues = enumerationValues;
}

const StructureWPtr& EnumerationPartial::structure() const
{
    return mStructure;
}

StructureWPtr EnumerationPartial::default_structure()
{
    static StructureWPtr defaultObject;
    return defaultObject;
}

void EnumerationPartial::set_structure(const StructureSPtr& structure)
{
    mStructure = structure;
}

}
