#include <ECS/ComponentType3D.hpp>
#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

AUTOGEN_DATA_RW_FUNC(State3D, position, rotation, scale)

AUTOGEN_DATA_RW_FUNC(Movement, walkSpeed, jogSpeed, sprintSpeed, speedSteps)

AUTOGEN_DATA_RW_FUNC(ComplexMovements)

AUTOGEN_DATA_RW_FUNC(Gauges)

DATA_WRITE_FUNC(HierarchyState3D::SynchType)
{
    auto &str = HierarchyState3D::SynchTypeReverseMap[data];
    out->write(str.c_str(), str.size());
    DATA_WRITE_END
}

DATA_READ_FUNC(HierarchyState3D::SynchType)
{
    DATA_READ_INIT(HierarchyState3D::SynchType)

    MAP_SAFE_READ(HierarchyState3D::SynchTypeMap, buff, data, buff->read())

    DATA_READ_END
}

AUTOGEN_DATA_RW_FUNC_STRUCT(HierarchyState3D,
    (
        position, rotation, scale
    ),
    (
        type
    )
);

bool operator==(const State3D &a, const State3D &b)
{
    return 
        a.position == b.position and 
        a.rotation == b.rotation and 
        a.scale    == b.scale    and 
        a.isActive == b.isActive
    ;
}