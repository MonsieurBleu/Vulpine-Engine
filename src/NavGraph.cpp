#include <NavGraph.hpp>
#include <iostream>

Node::Node(int _id, vec3 _position) {

    id = _id;
    position = _position;
    for(int i = 0; i < MAX_NEIGHBORS; i++) {
        neighbors[i].id = -1;
        neighbors[i].cost = 0;
    }
    neighborsN = 0;

}

Node::~Node() {

}

void Node::connectNode(int id_other, double cost) {

    if(neighborsN >= MAX_NEIGHBORS) {

        // FLAG_CERR
        std::cerr << "Node " << id << " has already 8 neighbors.\n";
        return;

    }

    neighbors[neighborsN].id = id_other;
    neighbors[neighborsN].cost = cost;

    neighborsN++;

}

// FLAG_COUT
void Node::print() {

    std::cout << "Node "
    << id << " in pos ("
    << position[0] << ", "
    << position[1] << ", "
    << position[2] << "), "
    << "is connected to ";

    for(int i = 0; i < neighborsN; i++) {

        std::cout << "(node: " << neighbors[i].id << ", distance: " << neighbors[i].cost << ")";
        if(i < neighborsN-1) std::cout << ", ";

    }

    std::cout << "\n";

}

void Path::update(NavGraphRef graph) {

    get()->clear();
    graph->shortestPath(start, dest, *this);

}

// FLAG_COUT
void Path::print() {

    std::cout << "Path is ";
    for(int node : *get()) {
        std::cout << node << " ";
    }
    std::cout << "\n";

}

NavGraph::NavGraph(int _id) {

    id = _id;
    nodes.reserve(START_GRAPH);
    nodesN = 0;

}

int NavGraph::addNode(vec3 position) {

    Node node(nodesN, position);
    nodes.push_back(node);
    nodesN++;
    return node.getId();

}

void NavGraph::connectNodes(int id_node1, int id_node2) {

    if(id_node1 >= nodesN || id_node2 >= nodesN) {
        
        // FLAG_CERR
        std::cerr << "Can't connect nodes, indexes provided are out of range.\n";
        return;

    }

    double cost = glm::distance(nodes[id_node1].getPosition(), nodes[id_node2].getPosition());

    nodes[id_node1].connectNode(id_node2, cost);
    nodes[id_node2].connectNode(id_node1, cost);

}

// A star implementation, using direct distance as the heuristic (supposedly admissible and consistent)
void NavGraph::shortestPath(int start, int end, Path path) {

    // pseudo-map used to reconstruct the path, we store the node we came from if the neighbor is add to the frontier
    int* cameFrom = new int[nodesN]();
    // pseudo-map used to store the cost of the cheapest path from start to current
    double* gScore = new double[nodesN];
    // pseudo-map used to store the cost + heuristic of the the cheapest path from start to current
    double* fScore = new double[nodesN];
    // pseudo-map used to check if a value is already in the frontier proprity queue without parsing it
    bool* insideFrontier = new bool[nodesN];

    // initial values
    for(int i = 0; i < nodesN; i++) {
        gScore[i] = INF;
        fScore[i] = INF;
        insideFrontier[i] = false;
    }

    // frontier of nodes we need to explore
    auto cmp = [&](int left, int right) {
        return fScore[left] > fScore[right];
    };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> frontier(cmp);

    // initialisation
    frontier.push(start);
    insideFrontier[start] = true;
    gScore[start] = 0;
    fScore[start] = glm::distance(nodes[start].getPosition(), nodes[end].getPosition());

    while(!frontier.empty()) {

        int current = frontier.top();
        // if we're at the goal we're good, return the optimal path
        if(current == end) {

            delete [] gScore;
            delete [] fScore;
            delete [] insideFrontier;
            reconstructPath(cameFrom, current, start, path);
            return;

        }
        insideFrontier[current] = false;
        frontier.pop();

        // for each neighbors
        for(int i = 0; i < nodes[current].getNeighborsN(); i++) {

            // get the cost from current to that neighbor
            Link toNeighbor = nodes[current].getLink(i);
            double tryScore = gScore[current] + toNeighbor.cost;

            // if we find a shorter path to the neighbor
            if(tryScore < gScore[toNeighbor.id]) {

                // new values for the neighbor
                cameFrom[toNeighbor.id] = current;
                gScore[toNeighbor.id] = tryScore;
                fScore[toNeighbor.id] = tryScore + glm::distance(nodes[toNeighbor.id].getPosition(), nodes[end].getPosition());

                // we add the neighbor to the frontier if not already in
                if(!insideFrontier[toNeighbor.id]) {
                    insideFrontier[toNeighbor.id] = true;
                    frontier.push(toNeighbor.id);
                }

            }

        }

    }

    // We should not be here during the execution
    delete [] cameFrom;
    delete [] gScore;
    delete [] fScore;
    delete [] insideFrontier;

    // FLAG_CERR
    std::cerr << "Could not find a path between node " << start << " and node " << end << " in graph " << id << ".\n";
    exit(EXIT_FAILURE);

}

void NavGraph::reconstructPath(int* cameFrom, int current, int start, Path path) {

    path->push_back(current);
    while(current != start) {
        current = cameFrom[current];
        path->push_front(current);
    }

}

// FLAG_COUT
void NavGraph::print() {

    std::cout << "Graph "
    << id << ", nodes are: \n";
    for(int i = 0; i < nodesN; i++) {

        nodes[i].print();

    }

    std::cout << "\n";

}