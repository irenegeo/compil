#include "integer.h"

namespace compil
{

Integer::Integer()
{
    set_literal(alter_literal());
    set_kind(alter_kind());
    set_cast(alter_cast());
}

Integer::~Integer()
{
}

IntegerSPtr Integer::downcast(const ObjectSPtr& pObject)
{
    return boost::static_pointer_cast<Integer>(pObject);
}

Type::ELiteral Integer::alter_literal()
{
    return Type::ELiteral::integer();
}

Type::EKind Integer::alter_kind()
{
    return Type::EKind::builtin();
}

CastableType::ECast Integer::alter_cast()
{
    return CastableType::ECast::weak();
}

const std::string& Integer::min() const
{
    return mMin;
}

void Integer::set_min(const std::string& min)
{
    mMin = min;
}

const std::string& Integer::max() const
{
    return mMax;
}

void Integer::set_max(const std::string& max)
{
    mMax = max;
}

}
