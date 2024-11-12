#include "n-array-tree.hpp"

Node::Node() {
    this->attribute = "";
    this->connections = std::vector<Connection*>();
}

Node::Node(std::string attribute) {
    this->attribute = attribute;
    this->connections = std::vector<Connection*>();
}

void Node::set_attribute(std::string attribute) {
    this->attribute = attribute;
}

std::string Node::get_attribute() {
    return this->attribute;
}

std::vector<Connection*> Node::get_connections() {
    return this->connections;
}

void Node::set_connections(std::vector<Connection*> connections) {
    this->connections = connections;
}

void Node::add_connection(Connection* connection) {
    this->connections.push_back(connection);
}

void Node::remove_connection(size_t index) {
    this->connections.erase(this->connections.begin() + index);
}

Connection::Connection() {
    this->answer = "";
    this->child = nullptr;
}

Connection::Connection(std::string answer, Node* child) {
    this->answer = answer;
    this->child = child;
}

void Connection::set_answer(std::string answer) {
    this->answer = answer;
}

std::string Connection::get_answer() {
    return this->answer;
}

void Connection::set_child(Node* child) {
    this->child = child;
}

Node* Connection::get_child() {
    return this->child;
}

Tree::Tree() {
    this->root = new Node();
}

Node* Tree::get_root() {
    return this->root;
}

void Tree::set_root(Node* root) {
    this->root = root;
}

void display(Node* current, int level) {
    std::vector<Connection*> connections = current->get_connections();

    // Indent based on the depth
    for (int j = 0; j < level; j++) {
        std::cout << "  ";
    }

    if (level > 0) {
        std::cout << "- ";
    }

    // Print the attribute
    std::cout << current->get_attribute() << "?" << std::endl;

    // Loop through each connection
    for (size_t i = 0; i < connections.size(); i++) {
        // Indent based on the depth
        for (int j = 0; j < level; j++) {
            std::cout << "  ";
        }

        if (level > 0) {
            std::cout << "  ";
        }

        // Print the connection
        std::cout << "- " << connections[i]->get_answer() << ": ";

        if (connections[i]->get_child()->get_connections().size() > 0) {
            std::cout << std::endl;

            display(connections[i]->get_child(), level + 2);
        } else {
            // Print the attribute
            std::cout << connections[i]->get_child()->get_attribute() << std::endl;
        }


    }

}