#include "compil/integer.h"
#include "compil/operator.h"

namespace lang
{

namespace compil
{

Integer::Integer()
{
}

Integer::~Integer()
{
}

bool Integer::hasOperator(const EOperatorAction& action,
                       const EOperatorFlags& flags) const
{
    if (flags.isSet(EOperatorFlags::native()))
    {
        return true;
    }

    return false;
}

}

}