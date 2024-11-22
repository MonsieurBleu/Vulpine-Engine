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
    friend class IterativeGraphLayoutSolver;
};


typedef std::shared_ptr<class EventNodeAnd> EventNodeAndPtr;
class EventNodeAnd : public EventNode
{
  private:
    std::vector<EventNodePtr> parents;

  public:
    EventNodeAnd() : EventNode("AND")
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

  public:
    EventNodeOr() : EventNode("OR")
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

  public:
    EventNodeNot() : EventNode("NOT")
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


// this class solves a graph layout problem
// it does this by iteratively solving the positions of the nodes by computing the forces between them
// the forces are computed by the spring model
class IterativeGraphLayoutSolver
{
private:
    std::vector<EventNodePtr> nodes;
    std::vector<vec3> positions;
    std::vector<std::vector<int>> adjacency;

    // the spring constant
    float k = 1;

    // the damping constant
    float c = 0.1;

    // the distance at which the spring force is 0
    float l = 2;

    // the maximum force
    float maxForce = 2;

    // maximum number of iterations
    const int MAX_ITERATIONS = 1000;
public:
    IterativeGraphLayoutSolver(std::vector<EventNodePtr> nodes) : nodes(nodes)
    {
        int n = nodes.size();
        for (auto &node : nodes)
        {
            vec3 p = vec3(
                (double)rand() / RAND_MAX,
                (double)rand() / RAND_MAX,
                (double)rand() / RAND_MAX) / 10.f;
            positions.push_back(p);

            std::vector<int> adj(n, -1);
            for (auto &child : node->children)
            {
                int i = std::find(nodes.begin(), nodes.end(), child) - nodes.begin();
                adj[i] = 1;
            }
            adjacency.push_back(adj);
        }
    }

    void solve()
    {
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            update();
            if (converged())
                break;
        }
    }

    void update()
    {
        // take into account adjacency
        for (int i = 0; i < nodes.size(); i++)
        {
            for (int j = 0; j < nodes.size(); j++)
            {
                if (i == j)
                    continue;
                vec3 diff = positions[j] - positions[i];
                float dist = length(diff);
                if (adjacency[i][j] == 1)
                {
                    float force = k * (dist - l);
                    vec3 forceVec = normalize(diff) * force;
                    positions[i] += forceVec;
                    positions[j] -= forceVec;
                }
            }
        }

    }

    bool converged()
    {
        for (int i = 0; i < nodes.size(); i++)
        {
            for (int j = 0; j < nodes.size(); j++)
            {
                if (i == j)
                    continue;
                vec3 diff = positions[j] - positions[i];
                float dist = length(diff);
                if (dist > 0.01)
                    return false;
            }
        }
        return true;
    }

    std::vector<vec3> getPositions()
    {
        return positions;
    }
};

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

        IterativeGraphLayoutSolver solver(nodesVec);
        solver.solve();


        std::vector<vec3> positions = solver.getPositions();

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

        for (int i = 0; i < nodesVec.size(); i++)
        {
            auto node = nodesVec[i];
            auto pos = positions[i];

            std::cout << node->getName() << " " << to_string(pos) << std::endl;

            ValueHelperRef<std::string> N(new ValueHelper(node->getName(), U" ", vec3(1, 0, 0)));
            N->state.setPosition(pos);
            N->state.scaleScalar(10);
            N->align = CENTERED;
            

            valueHelpers.push_back(std::make_pair(N, node));

            for (auto &child : node->children)
            {
                auto childPos = positions[std::find(nodesVec.begin(), nodesVec.end(), child) - nodesVec.begin()];
                LineHelperRef L(new LineHelper(pos, childPos, vec3(1, 0, 1)));
                lineHelpers.push_back(std::make_pair(L, node));
            }
        }

        model = (EntityModel{newObjectGroup()});
        for (auto &valueHelper : valueHelpers)
            model->add(valueHelper.first);
        for (auto &lineHelper : lineHelpers)
            model->add(lineHelper.first);
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
                lineHelper.first->color = vec3(0, 1, 0);
            else
                lineHelper.first->color = vec3(1, 0, 0);
        }
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