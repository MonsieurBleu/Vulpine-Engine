#pragma once
#include <ObjectGroup.hpp>

struct ModelLOD_elem
{
    ObjectGroupRef model;
    float distanceFromLastLevel = 1.f;
};

class ModelLOD : public ObjectGroupRef
{
    private : 
        const std::vector<ModelLOD_elem> levels;

    public : 

        ModelLOD(const std::vector<ModelLOD_elem> & levels) : levels(levels){};
        void update(bool forceUpdate = false);
};

typedef std::shared_ptr<ModelLOD> MeshLODref;