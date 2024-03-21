#pragma once

#define GLM_ENABLE_EXPERIMENTAL


#include <vector>
#include <deque>
#include <queue>
#include <memory>

#include <MathsUtils.hpp>

#define MAX_NEIGHBORS 8
#define START_GRAPH 16
#define INF 1e7

// link between two nodes
struct Link {
    int id; // id of neighbors in the graph
    double cost; // cost of link
};

class Node {

    private:

        vec3 position; // position of node
        int id; // id of node
        Link neighbors[MAX_NEIGHBORS]; // storing the id of neighbors
        int neighborsN; // number of neighbors


    public:

        Node(int, vec3);

        ~Node();

        vec3 getPosition() const {return position;};
        Link getLink(int i) const {return neighbors[i];};
        int getId() const {return id;};
        int getNeighborsN() const {return neighborsN;};

        void connectNode(int, double);

        void print();
        

};

class Path : std::shared_ptr<std::deque<int>>
{};

class NavGraph {

    private:

        std::vector<Node> nodes;
        int nodesN;
        int id;

    public:

        NavGraph(int);

        int addNode(vec3);
        void connectNodes(int, int);

        std::deque<int> shortestPath(int, int); // A* algorithm
        std::deque<int> reconstructPath(int*, int, int);

        const std::vector<Node>& getNodes(){return nodes;};

        void print();

};

typedef std::shared_ptr<NavGraph> NavGraphRef;

void printPath(std::deque<int>);