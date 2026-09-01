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

AUTOGEN_DATA_READ_FUNC_STRUCT(HierarchyState3D,
    (position, rotation, scale),
    (type)
);

DATA_WRITE_FUNC(HierarchyState3D) \
{ \
    DATA_WRITE_INIT(HierarchyState3D); \
    FTXTP_WRITE_ELEMENT_AUTOGEN(position)
    FTXTP_WRITE_ELEMENT_AUTOGEN(rotation)
    FTXTP_WRITE_ELEMENT_AUTOGEN(scale)
    if(data.type != HierarchyState3D::ALL)
        FTXTP_WRITE_ELEMENT_AUTOGEN_STRUCT(type)
    else
        out->Break();
    
    return out; \
}

DATA_WRITE_FUNC(std::vector<EntitySpawner::info>)
{
    out->Tabulate();

    for(auto &d : data)
    {
        out->Entry();
        out->write("\"", 1);
        out->write(d.name.c_str(), d.name.size());
        out->write("\"", 1);
        out->Tabulate();

        if(!d.cond.empty())
        {
            out->Entry();
            WRITE_NAME(cond, out)
            out->write("\"", 1);
            out->write(d.cond.c_str(), d.cond.size());
            out->write("\"", 1);
        }

        DataLoader<HierarchyState3D>::write(d.state, out);

        out->Break();
    }

    DATA_WRITE_END
}

DATA_READ_FUNC_INIT(std::vector<EntitySpawner::info>)

    if(true)
        {
            data.push_back(EntitySpawner::info());

            data.back().name = member;

            WHILE_NEW_VALUE
                IF_MEMBER_READ_VALUE(cond)
                {
                    data.back().cond = value;
                }
                else
                IF_MEMBER(HierarchyState3D)
                {
                    data.back().state = DataLoader<HierarchyState3D>::read(buff);
                }
            WHILE_NEW_VALUE_END
        }

DATA_READ_END_FUNC

AUTOGEN_DATA_RW_FUNC_STRUCT(EntitySpawner,
    (),
    (onLoading, onTrigger)
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