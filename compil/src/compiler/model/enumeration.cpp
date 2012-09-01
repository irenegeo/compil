#include "enumeration.h"

#include "object_factory.h"

#include <boost/foreach.hpp>

#include <limits.h>

namespace compil
{

long Enumeration::getMinValue() const
{
    long min = LONG_MAX;
    BOOST_FOREACH(EnumerationValueSPtr pEnumerationValue, enumerationValues())
    {
        AbsoluteEnumerationValueSPtr pValue = 
            ObjectFactory::downcastAbsoluteEnumerationValue(pEnumerationValue);
        if (!pValue) continue;
        if (min > pValue->value())
            min = pValue->value();
    }
    
    if (!flags())
    if (min > 0)
        min = 0;
    
    return min;
}

long Enumeration::getMaxValue() const
{
    long max = 0;
    BOOST_FOREACH(EnumerationValueSPtr pEnumerationValue, enumerationValues())
    {
        AbsoluteEnumerationValueSPtr pValue = 
            ObjectFactory::downcastAbsoluteEnumerationValue(pEnumerationValue);
        if (!pValue) continue;
        if (max < pValue->value())
            max = pValue->value();
    }
    
    return max;
}

}
