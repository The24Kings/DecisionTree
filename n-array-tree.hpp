#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <queue>

class Connection;
class Node;

class Node {
private:
    std::string attribute;
    std::vector<Connection*> connections;

public:
    Node();
    Node(std::string attribute);

    void set_attribute(std::string attribute);
    std::string get_attribute();

    std::vector<Connection*> get_connections();
    void set_connections(std::vector<Connection*> connections);

    void add_connection(Connection* connection);
    void remove_connection(size_t index);
};

class Tree {
private:
    Node* root;

public:
    Tree();

    Node* get_root();
    void set_root(Node* root);
};

void display(Node* current, int level);

class Connection {
private:
    std::string answer;
    Node* child;

public:
    Connection();
    Connection(std::string answer, Node* child);

    void set_answer(std::string answer);
    std::string get_answer();

    void set_child(Node* child);
    Node* get_child();

};
