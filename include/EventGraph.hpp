#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include <Utils.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace glm;

typedef std::shared_ptr<class EventNode> EventNodePtr;

enum class NodeType
{
    NODE,
    AND,
    OR,
    NOT
};

class EventNode;

// Generic primitive version of EventNode, used for serialization
// contains only the string names of the children
// instead of the pointers
struct EventNodePrimitive
{
    std::string name;
    std::string label;
    bool state;
    int type;
    std::vector<std::string> children;
};

class EventNode : public std::enable_shared_from_this<EventNode>
{
  private:
    EventNodePtr parent;
    NodeType type;

  protected:
    bool state = false;
    std::string name;
    std::string label;
    std::vector<EventNodePtr> children;

  public:
    EventNode(std::string name, std::string label = "", NodeType type = NodeType::NODE)
     : type(type), name(name), label(label)
    {
    }

    void set(bool state);

    bool get() const;

    virtual void update(bool state, std::vector<EventNodePtr> &explored);

    void update();

    void addChild(EventNodePtr child);

    virtual void addParent(EventNodePtr parent);

    virtual ~EventNode();

    NodeType getType() const;

    std::string getName() const;

    std::string getLabel() const;

    std::vector<EventNodePtr> getChildren() const
    {
        return children;
    }

    operator EventNodePrimitive()
    {
        EventNodePrimitive res;
        res.name = name;
        res.label = label;
        res.state = state;
        res.type = (int)type;
        for (auto &child : children)
            res.children.push_back(child->getName());
        return res;
    }

    // virtual EntityRef getWidget(vec2 position = vec2(0));

    friend class EventGraph;
    friend class EventGraphWidget;
    friend void generateGraphLayout(
        const std::vector<EventNodePtr> &nodes, 
        std::vector<vec3> &positions, 
        std::vector<std::vector<vec3>> &splines, 
        std::vector<int> &spline2Node
    );
};

typedef std::shared_ptr<class EventNodeAnd> EventNodeAndPtr;
class EventNodeAnd : public EventNode
{
  private:
    std::vector<EventNodePtr> parents;
    static inline int count = 0;

  public:
    EventNodeAnd(std::string label = "") 
     : EventNode("AND_" + std::to_string(count++), label, NodeType::AND)
    {
    }

    void addParent(EventNodePtr parent) override;

    void update(bool state, std::vector<EventNodePtr> &explored) override;

    // EntityRef getWidget(vec2 position = vec2(0)) override;
    friend class EventGraph;
};

typedef std::shared_ptr<class EventNodeOr> EventNodeOrPtr;
class EventNodeOr : public EventNode
{
  private:
    std::vector<EventNodePtr> parents;
    static inline int count = 0;

  public:
    EventNodeOr(std::string label = "") 
     : EventNode("OR_" + std::to_string(count++), label, NodeType::OR)
    {
    }

    void addParent(EventNodePtr parent) override;

    void update(bool state, std::vector<EventNodePtr> &explored) override;

    // EntityRef getWidget(vec2 position = vec2(0)) override;
    friend class EventGraph;
};

typedef std::shared_ptr<class EventNodeNot> EventNodeNotPtr;
class EventNodeNot : public EventNode
{
  private:
    EventNodePtr parent;
    static inline int count = 0;

  public:
    EventNodeNot(std::string label = "") 
     : EventNode("NOT_" + std::to_string(count++), label, NodeType::NOT)
    {
    }

    void addParent(EventNodePtr parent) override;

    void update(bool state, std::vector<EventNodePtr> &explored) override;

    // EntityRef getWidget(vec2 position = vec2(0)) override;
    friend class EventGraph;
};

void generateGraphLayout(
    const std::vector<EventNodePtr> &nodes, 
    std::vector<vec3> &positions, 
    std::vector<std::vector<vec3>> &splines, 
    std::vector<int> &spline2Node
    );

class EventGraph
{
  private:
    static inline std::unordered_map<std::string, EventNodePtr> nodes;
    static inline std::vector<vec3> positions;
    static inline std::vector<std::vector<vec3>> Bsplines;
    static inline std::vector<int> spline2Node;


    static inline bool dirty = true;

    // static inline EntityRef view = EntityRef();

    // static inline std::vector<std::pair<EntityRef, EventNodePtr>> nodeWidgets;

  public:
    static EventNodePtr getNode(std::string name);
    static std::unordered_map<std::string, EventNodePtr> getNodes() { return nodes; }

    static std::vector<EventNodePtr> findNodesWithLabel(std::string label);

    static void clear();

    static EventNodePtr addNode(std::string name, std::string label = "");
    static EventNodeAndPtr addAnd(std::string label = "", std::string name = "");
    static EventNodeOrPtr addOr(std::string label = "", std::string name = "");
    static EventNodeNotPtr addNot(std::string label = "", std::string name = "");

    // static void createView();

    // update the model values by coloring the nodes and edges
    // static void updateView();

    static void update();

    // static vec3 getNodePos(std::string name);

    // static EntityRef getView();

    static void load(std::string filename);

    friend class EventGraphWidget;
};