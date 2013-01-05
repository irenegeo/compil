#include "language/compil/document/structure.h"

namespace lang
{

namespace compil
{

StructurePartial::StructurePartial()
{
    set_literal(alter_literal());
    set_kind(alter_kind());
}

StructurePartial::~StructurePartial()
{
}

StructureSPtr StructurePartial::downcast(const ObjectSPtr& object)
{
    return boost::static_pointer_cast<Structure>(object);
}

StructureSPtr StructurePartial::shared_from_this()
{
    return boost::static_pointer_cast<Structure>(
        boost::enable_shared_from_this<Object>::shared_from_this());
}

StructureSCPtr StructurePartial::shared_from_this() const
{
    return boost::static_pointer_cast<const Structure>(
        boost::enable_shared_from_this<Object>::shared_from_this());
}

TypePartial::ELiteral StructurePartial::alter_literal()
{
    return TypePartial::ELiteral::structure();
}

TypePartial::EKind StructurePartial::alter_kind()
{
    return TypePartial::EKind::object();
}

bool StructurePartial::abstract() const
{
    return mAbstract;
}

Structure& StructurePartial::set_abstract(bool abstract)
{
    mAbstract = abstract;
    return *(Structure*)this;
}

bool StructurePartial::controlled() const
{
    return mControlled;
}

Structure& StructurePartial::set_controlled(bool controlled)
{
    mControlled = controlled;
    return *(Structure*)this;
}

bool StructurePartial::immutable() const
{
    return mImmutable;
}

Structure& StructurePartial::set_immutable(bool immutable)
{
    mImmutable = immutable;
    return *(Structure*)this;
}

bool StructurePartial::partial() const
{
    return mPartial;
}

Structure& StructurePartial::set_partial(bool partial)
{
    mPartial = partial;
    return *(Structure*)this;
}

bool StructurePartial::sharable() const
{
    return mSharable;
}

Structure& StructurePartial::set_sharable(bool sharable)
{
    mSharable = sharable;
    return *(Structure*)this;
}

bool StructurePartial::streamable() const
{
    return mStreamable;
}

Structure& StructurePartial::set_streamable(bool streamable)
{
    mStreamable = streamable;
    return *(Structure*)this;
}

const std::vector<ObjectSPtr>& StructurePartial::objects() const
{
    return mObjects;
}

Structure& StructurePartial::set_objects(const std::vector<ObjectSPtr>& objects)
{
    mObjects = objects;
    return *(Structure*)this;
}

std::vector<ObjectSPtr>& StructurePartial::mutable_objects()
{
    return mObjects;
}

const StructureWPtr& StructurePartial::baseStructure() const
{
    return mBaseStructure;
}

StructureWPtr StructurePartial::default_baseStructure()
{
    static StructureWPtr defaultObject;
    return defaultObject;
}

Structure& StructurePartial::set_baseStructure(const StructureSPtr& baseStructure)
{
    mBaseStructure = baseStructure;
    return *(Structure*)this;
}

Structure& StructurePartial::set_sourceId(const SourceIdSPtr& sourceId)
{
    Object::set_sourceId(sourceId);
    return *(Structure*)this;
}

Structure& StructurePartial::set_line(const Line& line)
{
    Object::set_line(line);
    return *(Structure*)this;
}

Structure& StructurePartial::set_column(const Column& column)
{
    Object::set_column(column);
    return *(Structure*)this;
}

Structure& StructurePartial::set_package(const PackageSPtr& package)
{
    TypePartial::set_package(package);
    return *(Structure*)this;
}

Structure& StructurePartial::set_comment(const CommentSPtr& comment)
{
    TypePartial::set_comment(comment);
    return *(Structure*)this;
}

Structure& StructurePartial::set_name(const NameSPtr& name)
{
    TypePartial::set_name(name);
    return *(Structure*)this;
}

Structure& StructurePartial::set_literal(const ELiteral& literal)
{
    TypePartial::set_literal(literal);
    return *(Structure*)this;
}

Structure& StructurePartial::set_kind(const EKind& kind)
{
    TypePartial::set_kind(kind);
    return *(Structure*)this;
}

}

}

