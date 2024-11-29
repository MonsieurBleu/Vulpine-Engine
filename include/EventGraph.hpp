#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <stack>
#include <Utils.hpp>

#include <Helpers.hpp>

#include <glm/gtx/string_cast.hpp>

#include <graphviz/gvc.h>


typedef std::shared_ptr<class EventNode> EventNodePtr;

// TODO: move this whole thing to a cpp file
class EventNode : public std::enable_shared_from_this<EventNode>
{
  private:
    EventNodePtr parent;

  protected:
    bool state = false;
    std::string name;
    std::vector<EventNodePtr> children;

  public:
    EventNode(std::string name) : name(name)
    {
    }
    
    void set(bool state)
    {
        this->state = state;
        update();
    }
    bool get()
    {
        return state;
    }

    virtual void update(bool state, std::vector<EventNodePtr> &explored)
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        explored.push_back(shared_from_this());

        this->state = state;
        for (auto &child : children)
            child->update(state, explored);
    }

    void update()
    {
        std::vector<EventNodePtr> explored;
        update(state, explored);
    }

    void addChild(EventNodePtr child)
    {
        children.push_back(child);
        child->addParent(shared_from_this());
    }

    virtual void addParent(EventNodePtr parent)
    {
        this->parent = parent;
    }

    virtual ~EventNode()
    {
    }

    std::string& getName()
    {
        return name;
    }

    virtual void print(int depth = 0, int tab = 4, EventNodePtr from = nullptr, std::vector<EventNodePtr> explored = {}, bool propagate = true)
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        explored.push_back(shared_from_this());

        for (int i = 0; i < depth; i++)
        {
            if (i < depth - tab)
                std::cout << " ";
            else if (i == depth - tab)
                std::cout << "└";
            else if (i == depth - 1)
                std::cout << "╴";
            else
                std::cout << "─";
        }

        std::string termColor = state ? TERMINAL_OK : TERMINAL_ERROR;
        std::cout << termColor << name << TERMINAL_RESET << std::endl;
        
        if (propagate)
            for (auto &child : children)
                child->print(depth + tab, tab, shared_from_this(), explored);
    }

    friend class EventGraph;
    friend void generateGraphLayout(const std::vector<EventNodePtr> &nodes, std::vector<vec3> &positions, std::vector<std::vector<vec3>> &splines);
};


typedef std::shared_ptr<class EventNodeAnd> EventNodeAndPtr;
class EventNodeAnd : public EventNode
{
  private:
    std::vector<EventNodePtr> parents;
    static inline int count = 0;

  public:
    EventNodeAnd() : EventNode("AND" + std::to_string(count++))
    {
    }

    void addParent(EventNodePtr parent) override
    {
        parents.push_back(parent);
    }

    void update(bool state, std::vector<EventNodePtr> &explored) override
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        bool newState = true;
        for (auto &parent : parents)
            newState &= parent->get();
        EventNode::update(newState, explored);
    }

    void print(int depth = 0, int tab = 4, EventNodePtr from = nullptr, std::vector<EventNodePtr> explored = {}, bool propagate = true) override
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        explored.push_back(shared_from_this());

        // print all the parents except from
        for (auto &parent : parents)
            if (parent != from)
                parent->print(depth - tab, tab, shared_from_this(), explored, false);

        for (int i = 0; i < depth; i++)
        {
            if (i < depth - tab)
                std::cout << " ";
            else if (i == depth - tab)
                std::cout << "└";
            else if (i == depth - 1)
                std::cout << "╴";
            else
                std::cout << "─";
        }

        std::string termColor = state ? TERMINAL_OK : TERMINAL_ERROR;
        std::cout << termColor << name << TERMINAL_RESET << std::endl;
        
        if (propagate)
            for (auto &child : children)
                child->print(depth + tab, tab, shared_from_this(), explored);
    }
};


typedef std::shared_ptr<class EventNodeOr> EventNodeOrPtr;
class EventNodeOr : public EventNode
{
  private:
    std::vector<EventNodePtr> parents;
    static inline int count = 0;

  public:
    EventNodeOr() : EventNode("OR" + std::to_string(count++))
    {
    }

    void addParent(EventNodePtr parent) override
    {
        parents.push_back(parent);
    }

    void update(bool state, std::vector<EventNodePtr> &explored) override
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        bool newState = false;
        for (auto &parent : parents)
            newState |= parent->get();
        EventNode::update(newState, explored);
    }

    void print(int depth = 0, int tab = 4, EventNodePtr from = nullptr, std::vector<EventNodePtr> explored = {}, bool propagate = true) override
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        explored.push_back(shared_from_this());

        // print all the parents except from
        for (auto &parent : parents)
            if (parent != from)
                parent->print(depth - tab, tab, shared_from_this(), explored, false);

        for (int i = 0; i < depth; i++)
        {
            if (i < depth - tab)
                std::cout << " ";
            else if (i == depth - tab)
                std::cout << "└";
            else if (i == depth - 1)
                std::cout << "╴";
            else
                std::cout << "─";
        }

        std::string termColor = state ? TERMINAL_OK : TERMINAL_ERROR;
        std::cout << termColor << name << TERMINAL_RESET << std::endl;
        
        if (propagate)
            for (auto &child : children)
                child->print(depth + tab, tab, shared_from_this(), explored);
    }
};


typedef std::shared_ptr<class EventNodeNot> EventNodeNotPtr;
class EventNodeNot : public EventNode
{
  private:
    EventNodePtr parent;
    static inline int count = 0;

  public:
    EventNodeNot() : EventNode("NOT" + std::to_string(count++))
    {
    }

    void addParent(EventNodePtr parent) override
    {
        this->parent = parent;
    }

    void update(bool state, std::vector<EventNodePtr> &explored) override
    {
        EventNode::update(!parent->get(), explored);
    }

    void print(int depth = 0, int tab = 4, EventNodePtr from = nullptr, std::vector<EventNodePtr> explored = {}, bool propagate = true) override
    {
        if (std::find(explored.begin(), explored.end(), shared_from_this()) != explored.end())
            return;

        explored.push_back(shared_from_this());

        for (int i = 0; i < depth; i++)
        {
            if (i < depth - tab)
                std::cout << " ";
            else if (i == depth - tab)
                std::cout << "└";
            else if (i == depth - 1)
                std::cout << "╴";
            else
                std::cout << "─";
        }

        std::string termColor = state ? TERMINAL_OK : TERMINAL_ERROR;
        std::cout << termColor << name << TERMINAL_RESET << std::endl;
        
        if (propagate)
            for (auto &child : children)
                child->print(depth + tab, tab, shared_from_this(), explored);
    }
};



inline void generateGraphLayout(const std::vector<EventNodePtr> &nodes, std::vector<vec3> &positions, std::vector<std::vector<vec3>> &splines)
{

    Agraph_t *g;
    Agnode_t *n;
    Agedge_t *e;

    std::vector<Agnode_t *> agnodes;
    std::vector<Agedge_t *> agedges;

    g = agopen((char *)"g", Agdirected, 0);
    // agsafeset(g, (char *)"splines", (char *)"polyline", (char *)"");

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

    static GVC_t *gvc = gvContext();
    gvLayout(gvc, g, "dot");

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
}

class EventGraph
{
  private:
    static inline std::unordered_map<std::string, EventNodePtr> nodes;
    static inline EntityModel model;

    // the nodes
    static inline std::vector<std::pair<ValueHelperRef<std::string>, EventNodePtr>> valueHelpers;
    // the edges
    static inline std::vector<std::pair<LineHelperRef, EventNodePtr>> lineHelpers;

  public:
    static EventNodePtr getNode(std::string name)
    {
        if (nodes.find(name) == nodes.end())
            return nullptr;
        return nodes[name];
    }

    static void clear()
    {
        nodes.clear();
    }

    static EventNodePtr addNode(std::string name)
    {
        if (nodes.find(name) != nodes.end())
            return nullptr;

        EventNodePtr node = std::make_shared<EventNode>(name);
        nodes[name] = node;
        return node;
    }

    static void createModel()
    {
        std::set<EventNodePtr> nodesSet;
        std::stack<EventNodePtr> stack;
        for (auto &node : nodes)
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


        
        std::vector<vec3> positions;
        std::vector<std::vector<vec3>> Bsplines;
        generateGraphLayout(nodesVec, positions, Bsplines);

        // recenter the positions by computing the bounding box
        vec3 minPos = vec3(1e9);
        vec3 maxPos = vec3(-1e9);
        for (auto &pos : positions)
        {
            minPos = min(minPos, pos);
            maxPos = max(maxPos, pos);
        }

        vec3 center = (minPos + maxPos) / 2.0f;

        for (auto &pos : positions)
            pos -= center;

        float scale = 3.0f;
        // normalize the positions 
        float maxDist = 0;
        for (auto &pos : positions)
            maxDist = max(maxDist, length(pos));

        for (auto &pos : positions)
        {
            pos /= maxDist;
            pos *= scale;
            pos.x += 0.05f;
        }

        std::vector<std::vector<vec3>> splines;
        for (auto &spline : Bsplines)
        {
            std::vector<vec3> s;
            BSpline(spline, s);
            splines.push_back(s);
        }

        // apply the same transformation to the splines
        for (auto &spline : splines)
        {
            for (auto &point : spline)
            {
                point -= center;
                point /= maxDist;
                point *= scale;
            }
        }

        

        // std::cout << splines[0][0].first.x << " " << splines[0][0].first.y << " " << splines[0][0].first.z << std::endl;

        for (int i = 0; i < nodesVec.size(); i++)
        {
            auto node = nodesVec[i];
            auto pos = positions[i];

            // std::cout << node->getName() << " " << to_string(pos) << std::endl;

            ValueHelperRef<std::string> N(new ValueHelper(node->getName(), U" ", vec3(1, 0, 0)));
            N->state.setPosition(pos);
            N->state.scaleScalar(10);
            N->align = CENTERED;
            

            valueHelpers.push_back(std::make_pair(N, node));

            // for (auto &child : node->children)
            // {
            //     auto childPos = positions[std::find(nodesVec.begin(), nodesVec.end(), child) - nodesVec.begin()];
            //     LineHelperRef L(new LineHelper(pos, childPos, vec3(1, 0, 1)));
            //     lineHelpers.push_back(std::make_pair(L, node));
            // }

            for (int j = 0; j < splines[i].size() - 1; j++)
            {
                LineHelperRef L(new LineHelper(splines[i][j], splines[i][j+1], vec3(1, 0, 1)));
                lineHelpers.push_back(std::make_pair(L, node));
            }
        }

        model = (EntityModel{newObjectGroup()});
        for (auto &lineHelper : lineHelpers)
            model->add(lineHelper.first);
        for (auto &valueHelper : valueHelpers)
            model->add(valueHelper.first);
    }

    // update the model values by coloring the nodes and edges
    static void updateModel()
    {
        for (auto &valueHelper : valueHelpers)
        {
            if (valueHelper.second->get())
                valueHelper.first->color = vec3(0, 1, 0);
            else
                valueHelper.first->color = vec3(1, 0, 0);
        }

        for (auto &lineHelper : lineHelpers)
        {
            if (lineHelper.second->get())
                lineHelper.first->color = vec3(0, .2, 0);
            else
                lineHelper.first->color = vec3(.2, 0, 0);
        }
    }

    static void update()
    {
        for (auto &valueHelper : valueHelpers)
            valueHelper.second->update();
        updateModel();
    }

    static vec3 getNodePos(std::string name)
    {
        for (auto &valueHelper : valueHelpers)
            if (valueHelper.second->getName() == name)
                return valueHelper.first->state.position;
        return vec3(0);
    }

    static EntityModel getModel()
    {
        return model;
    }
};