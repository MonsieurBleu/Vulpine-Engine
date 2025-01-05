#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>
#include <MappedEnum.hpp>

#include "EventGraph.hpp"

DATA_WRITE_FUNC_INIT(EventNodePrimitive)
    out->Entry();
    WRITE_NAME(name, out)
    out->write("\"", 1);
    out->write(CONST_STRING_SIZED(data.name));
    out->write("\"", 1);

    if (data.label.size())
    {
        out->Entry();
        WRITE_NAME(label, out)
        out->write("\"", 1);
        out->write(CONST_STRING_SIZED(data.label));
        out->write("\"", 1);
    }

    out->Entry();
    WRITE_NAME(state, out)
    FastTextParser::write<bool>(data.state, out->getReadHead());

    out->Entry();
    WRITE_NAME(type, out)
    FastTextParser::write<int>((int)data.type, out->getReadHead());

    out->Entry();
    out->Tabulate();
    WRITE_NAME(children, out);
    FastTextParser::write<int>(data.children.size(), out->getReadHead());
    
    for(auto &i : data.children)
    {
        out->Entry();
        WRITE_NAME(child, out);
        out->write("\"", 1);
        out->write(CONST_STRING_SIZED(i));
        out->write("\"", 1);
    }

    out->Break();
    
DATA_WRITE_END_FUNC

DATA_READ_FUNC(EventNodePrimitive) {
    std::string name;
    std::string label;
    bool state;
    int type;
    std::vector<std::string> childrenNames;

    DATA_READ_INIT(EventNodePrimitive)

    buff->read();
    buff->read();

    WHILE_NEW_VALUE

        IF_MEMBER_READ_VALUE(name)
            name = std::string(value);
        else IF_MEMBER_READ_VALUE(label)
            label = std::string(value);
        else IF_MEMBER_READ_VALUE(state)
            state = FastTextParser::read<bool>(value);
        else IF_MEMBER_READ_VALUE(type)
            type = FastTextParser::read<int>(value);
        else IF_MEMBER_READ_VALUE(children)
        {
            WHILE_NEW_VALUE
                IF_MEMBER_READ_VALUE(child) {
                    childrenNames.push_back(std::string(value));
                }
                
            WHILE_NEW_VALUE_END
        }
    WHILE_NEW_VALUE_END

    data = EventNodePrimitive {name, label, state, type, childrenNames};

    DATA_READ_END
}

DATA_WRITE_FUNC_INIT(EventGraph)
    out->Entry();
    out->Tabulate();
    WRITE_NAME(nodes, out);
    FastTextParser::write<int>(EventGraph::getNodes().size(), out->getReadHead());

    for(auto &i : EventGraph::getNodes())
    {
        DataLoader<EventNodePrimitive>::write(*(i.second), out);
    }

    out->Break();

DATA_WRITE_END_FUNC

DATA_READ_FUNC(EventGraph) {
    int size = 0;
    std::vector<EventNodePrimitive> nodes;

    DATA_READ_INIT(EventGraph)

    buff->read();
    buff->read();

    WHILE_NEW_VALUE

        IF_MEMBER_READ_VALUE(nodes)
        {
            size = FastTextParser::read<int>(value);
            for(int i = 0; i < size; i++)
            {
                nodes.push_back(DataLoader<EventNodePrimitive>::read(buff));
            }
        }
    WHILE_NEW_VALUE_END

    for (auto &i : nodes)
    {
        switch ((NodeType)i.type)
        {
            case NodeType::NODE: {
                auto n = EventGraph::addNode(i.name, i.label);
                n->set(i.state);
                break;
            }
            case NodeType::AND:
                EventGraph::addAnd(i.label, i.name);
                break;
            case NodeType::OR:
                EventGraph::addOr(i.label, i.name);
                break;
            case NodeType::NOT:
                EventGraph::addNot(i.label, i.name);
                break;
        }
    }

    for (auto &i : nodes)
    {
        auto n = EventGraph::getNode(i.name);
        for (auto &j : i.children)
        {
            n->addChild(EventGraph::getNode(j));
        }
    }

    DATA_READ_END
}
