#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/norm.hpp>
#include <../Engine/include/MathsUtils.hpp>
#include <vector>
#include <deque>
#include <queue>
#include <iostream>

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

        vec3 getPosition() {return position;};
        Link getLink(int i) {return neighbors[i];};
        int getId() {return id;};
        int getNeighborsN() {return neighborsN;};

        void connectNode(int, double);

        void print();
        

};

class Graph {

    private:

        std::vector<Node> nodes;
        int nodesN;
        int id;

    public:

        Graph(int);

        int addNode(vec3);
        void connectNodes(int, int);

        std::deque<int> shortestPath(int, int); // A* algorithm
        std::deque<int> reconstructPath(int*, int, int);

        void print();

};

void printPath(std::deque<int>);
