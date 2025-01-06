#include "EventGraph.hpp"
#include <graphviz/gvc.h>

void EventNode::set(bool state)
{
    this->state = state;
    update();
}

bool EventNode::get() const
{
    return state;
}

NodeType EventNode::getType() const
{
    return type;
}

void EventNode::update(bool state, std::vector<EventNodePtr> &explored)
{
    if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
        return;

    explored.push_back(shared_from_this());

    this->state = state;
    for (auto &child : children)
        child->update(state, explored);
}

void EventNode::update()
{
    std::vector<EventNodePtr> explored;
    update(state, explored);
}

void EventNode::addChild(EventNodePtr child)
{
    children.push_back(child);
    child->addParent(shared_from_this());
}

void EventNode::addParent(EventNodePtr parent)
{
    this->parent = parent;
}

EventNode::~EventNode()
{
}

std::string EventNode::getName() const
{
    return name;
}

std::string EventNode::getLabel() const
{
    return label;
}

void EventNodeAnd::addParent(EventNodePtr parent)
{
    parents.push_back(parent);
}

void EventNodeAnd::update(bool state, std::vector<EventNodePtr> &explored)
{
    if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
        return;

    bool newState = true;
    for (auto &parent : parents)
        newState &= parent->get();
    EventNode::update(newState, explored);
}

void EventNodeOr::addParent(EventNodePtr parent)
{
    parents.push_back(parent);
}

void EventNodeOr::update(bool state, std::vector<EventNodePtr> &explored)
{
    if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
        return;

    bool newState = false;
    for (auto &parent : parents)
        newState |= parent->get();
    EventNode::update(newState, explored);
}


void EventNodeNot::addParent(EventNodePtr parent)
{
    this->parent = parent;
}

void EventNodeNot::update(bool state, std::vector<EventNodePtr> &explored)
{
    EventNode::update(parent != nullptr && !parent->get(), explored);
}


void generateGraphLayout(const std::vector<EventNodePtr> &nodes, std::vector<vec3> &positions, std::vector<std::vector<vec3>> &splines)
{
    Agraph_t *g;
    Agnode_t *n;
    Agedge_t *e;

    std::vector<Agnode_t *> agnodes;
    std::vector<Agedge_t *> agedges;

    g = agopen((char *)"g", Agdirected, 0);
    // agsafeset(g, (char *)"splines", (char *)"polyline", (char *)"");
    agsafeset(g, (char *)"start", (char *)"1", (char *)"");
    

    std::unordered_map<EventNodePtr, Agnode_t *> nodeToAgnode;

    for (auto &node : nodes)
    {
        n = agnode(g, (char *)node->getName().c_str(), 1);
        agsafeset(n, (char *)"shape", (char *)"point", (char *)"");

        agnodes.push_back(n);
        nodeToAgnode[node] = n;
    }

    std::unordered_map<Agnode_t*, int> agnodeToIndex;
    for (int i = 0; i < nodes.size(); i++)
    {
        positions.push_back(vec3(0));
        agnodeToIndex[agnodes[i]] = i;
    }

    std::unordered_map<Agedge_t*, std::pair<Agnode_t*, Agnode_t*>> edgeToNode;

    for (auto &node : nodes)
    {
        for (auto &child : node->children)
        {
            Agnode_t *n0 = nodeToAgnode[node];
            Agnode_t *n1 = nodeToAgnode[child];
            e = agedge(g, n0, n1, 0, 1);
            agsafeset(e, (char *)"dir", (char *)"none", (char *)"");

            agedges.push_back(e);
            edgeToNode[e] = std::make_pair(n0, n1);
        }
    }

    GVC_t *gvc = gvContext();
    gvLayout(gvc, g, "neato");

    // gvRender(gvc, g, "dot", stdout);
    

    // for (auto &node : nodes)
    // {
    //     n = agnode(g, (char *)node->getName().c_str(), 0);
    //     positions[nodeToIndex[node]] = vec3(0, ND_coord(n).y, ND_coord(n).x);

    //     std::vector<std::pair<vec3, vec3>> spline;
    //     for (auto &child : node->children)
    //     {
    //         e = agedge(g, agnode(g, (char *)node->getName().c_str(), 0), agnode(g, (char *)child->getName().c_str(), 0), 0, 1);
    //         auto spl = ED_spl(e);
    //         for (int i = 0; i < spl->size; i++)
    //         {
    //             auto &point = spl->list[i];
    //             spline.push_back(std::make_pair(vec3(0, point.sp.y, point.sp.x), vec3(0, point.ep.y, point.ep.x)));
    //         }
            
    //     }
    //     splines.push_back(spline);
    // }

    for (auto& n : agnodes)
    {
        positions[agnodeToIndex[n]] = vec3(0, ND_coord(n).y, ND_coord(n).x);
    }

    for (auto& e : agedges)
    {
        auto spl = ED_spl(e);
        std::vector<vec3> spline;
        for (int i = 0; i < spl->size; i++)
        {
            auto &point = spl->list[i];
            vec3 p0 = vec3(0, ND_coord(edgeToNode[e].first).y, ND_coord(edgeToNode[e].first).x);
            vec3 p1 = vec3(0, ND_coord(edgeToNode[e].second).y, ND_coord(edgeToNode[e].second).x);
            spline.push_back(p0);
            for (int j = 1; j < point.size - 1; j++)
            {
                vec3 p = vec3(0, point.list[j+0].y, point.list[j+0].x);
                spline.push_back(p);
            }
            spline.push_back(p1);
        }
        splines.push_back(spline);
    }


    agclose(g);

    gvFreeLayout(gvc, g);
    gvFinalize(gvc);

}

EventNodePtr EventGraph::getNode(std::string name)
{
    if (nodes.find(name) == nodes.end())
        return nullptr;
    return nodes[name];
}

std::vector<EventNodePtr> EventGraph::findNodesWithLabel(std::string label)
{
    std::vector<EventNodePtr> res;
    for (auto &node : nodes)
        if (node.second->getLabel() == label)
            res.push_back(node.second);
    return res;
}

void EventGraph::clear()
{
    nodes.clear();

    EventNodeAnd::count = 0;
    EventNodeOr::count = 0;
    EventNodeNot::count = 0;

    dirty = true;
}

EventNodePtr EventGraph::addNode(std::string name, std::string label)
{
    if (nodes.find(name) != nodes.end())
    {
        ERROR_MESSAGE("Node with name " + name + " already exists");
        return nullptr;
    }

    EventNodePtr node = std::make_shared<EventNode>(name, label);
    nodes[name] = node;

    dirty = true;
    return node;
}

EventNodeAndPtr EventGraph::addAnd(std::string label, std::string name)
{
    EventNodeAndPtr node = std::make_shared<EventNodeAnd>(label);
    if (name != "")
        node->name = name;
    nodes[node->getName()] = node;

    dirty = true;
    return node;
}

EventNodeOrPtr EventGraph::addOr(std::string label, std::string name)
{
    EventNodeOrPtr node = std::make_shared<EventNodeOr>(label);
    if (name != "")
        node->name = name;
    nodes[node->getName()] = node;

    dirty = true;
    return node;
}

EventNodeNotPtr EventGraph::addNot(std::string label, std::string name)
{
    EventNodeNotPtr node = std::make_shared<EventNodeNot>(label);
    if (name != "")
        node->name = name;
    nodes[node->getName()] = node;

    dirty = true;
    return node;
}

void EventGraph::update()
{
    if (dirty)
    {
        std::set<EventNodePtr> nodesSet;
        std::stack<EventNodePtr> stack;
        for (auto &node : EventGraph::nodes)
            stack.push(node.second);

        while (!stack.empty())
        {
            auto node = stack.top();
            stack.pop();
            if (nodesSet.find(node) != nodesSet.end())
                continue;
            nodesSet.insert(node);
            for (auto &child : node->children)
                stack.push(child);
        }

        std::vector<EventNodePtr> nodesVec;
        for (auto &node : nodesSet)
            nodesVec.push_back(node);

        generateGraphLayout(nodesVec, positions, Bsplines);
        
        dirty = false;
    }

    // for (auto &valueHelper : valueHelpers)
    //     valueHelper.second->update();
    for (auto &node : nodes)
    {
        node.second->update();
    }
}



