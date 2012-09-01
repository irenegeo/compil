#include "import.h"

namespace compil
{

Import::Import()
{
}

Import::~Import()
{
}

ImportSPtr Import::downcast(const ObjectSPtr& pObject)
{
    return boost::static_pointer_cast<Import>(pObject);
}

const std::string& Import::source() const
{
    return mSource;
}

void Import::set_source(const std::string& source)
{
    mSource = source;
}

}
