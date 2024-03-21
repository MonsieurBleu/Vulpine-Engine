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

        void rearrangeNeighbors();


    public:

        Node(int, vec3);

        ~Node();

        vec3 getPosition() const {return position;};
        Link getLink(int i) const {return neighbors[i];};
        int getId() const {return id;};
        int getNeighborsN() const {return neighborsN;};

        void connectNode(int, double);
        void deconnectNode(int);

        void print();
        

};

class Path;

class NavGraph {

    private:

        std::vector<Node> nodes;
        int nodesN;
        int id;

    public:

        NavGraph(int);

        int addNode(vec3);
        void removeNode(int);
        void connectNodes(int, int);
        void deconnectNodes(int, int);

        void shortestPath(int, int, Path); // A* algorithm
        void reconstructPath(int*, int, int, Path);

        const std::vector<Node>& getNodes(){return nodes;};

        void print();

};

typedef std::shared_ptr<NavGraph> NavGraphRef;

class Path : public std::shared_ptr<std::deque<int>>
{
    private:

        int start;
        int dest;

    public:

        Path(int _start, int _dest): std::shared_ptr<std::deque<int>>(new std::deque<int>), start(_start), dest(_dest){};
        ~Path(){};
        
        void update(NavGraphRef);
        void print();
        
};