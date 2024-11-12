#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#include "n-array-tree.hpp"

typedef std::vector<std::string> Header;
typedef std::vector<std::vector<std::string>> Values;
typedef std::pair<size_t, double> MaxGain;
typedef std::map<std::string, std::pair<int, int>> AttributeValues; // Attribute response, positive outcomes, negative outcomes
typedef std::pair<Header, Values> CSVData;

std::string training_file = "TrainingData.csv";
std::string testing_file = "TestingData.csv";

double entropy(double p, double n);
double gain(int attribute_index, CSVData data, double total_entropy);
MaxGain find_max_gain(CSVData training_data, double p, double n);
AttributeValues get_attribute_values(CSVData data, int attribute_index);
CSVData get_attribute_rows(CSVData data, int attribute_index, std::string attribute_value);
CSVData remove_attribute(CSVData data, size_t index);
double test_tree(Tree &tree, CSVData* data);
Node* construct_tree(Node* current, CSVData training_data, double p, double n);
void load(std::string filename, CSVData &data);

int main() {
    Tree tree;
    CSVData training_data;
    CSVData testing_data;

    load(training_file, training_data);
    load(testing_file, testing_data);

    /* Gather Initial Data */
    double p = 0;
    double n = 0;

    for (size_t i = 0; i < training_data.second.size(); i++) {
        if (training_data.second[i][training_data.second[i].size() - 1].compare("Yes") == 0) {
            p++;
        } else {
            n++;
        }
    }

    /* Data Construction and Testing */
    construct_tree(tree.get_root(), training_data, p, n);

    display(tree.get_root(), 0);

    std::cout << "\nAccuracy: " << test_tree(tree, &testing_data) << "%" << std::endl;

    /* File Output */
    std::remove("TestingDataOutput.csv");
    std::ofstream file("TestingDataOutput.csv");

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file TestingDataOutput.csv" << std::endl;
        return 1;
    }

    for (size_t i = 0; i < testing_data.first.size(); i++) {
        file << testing_data.first[i];

        if (i < testing_data.first.size() - 1) {
            file << ",";
        }
    }

    file << std::endl;

    for (size_t i = 0; i < testing_data.second.size(); i++) {
        for (size_t j = 0; j < testing_data.second[i].size(); j++) {
            file << testing_data.second[i][j];

            if (j < testing_data.second[i].size() - 1) {
                file << ",";
            }
        }

        file << std::endl;
    }

    file.close();

    return 0;
}

/**
 * @brief Recursively construct the decision tree
 * 
 * @param current Current node
 * @param data Data to construct the tree with
 * @param p Positive outcomes
 * @param n Negative outcomes
 * 
 * @return Node*
 */
Node* construct_tree(Node* current, CSVData data, double p, double n) { // Fuck recursion >:(
    MaxGain max_gain = find_max_gain(data, p, n);

    // Set the attribute of the current node to the attribute with the maximum gain
    current->set_attribute(data.first[max_gain.first]);

    // Get the attribute values for the attribute with the highest gain
    AttributeValues attribute_values = get_attribute_values(data, max_gain.first);

    // Loop through the attribute values, recursively calling construct_tree on attribute values with ambiguous responses
    for (auto const& [key, val] : attribute_values) {
        Node* child = new Node(); // <- Fuck you, you bitch. You've caused me unknowning dismay (THIS TOOK ME TWO DAYS TO DEBUG)

        // Create a connection for each attribute value
        Connection* connection = new Connection(key, nullptr);

        // Add the connection to the current node
        current->add_connection(connection);

        // Add the response as a child node
        if (val.first == 0 || val.second == 0) {
            // If there are no positive or negative outcomes, create a leaf node
            if (val.first == 0) {
                connection->set_child(new Node("No"));
            } else {
                connection->set_child(new Node("Yes"));
            }
        } else {
            // Get the rows that contain that response
            CSVData new_data = get_attribute_rows(data, max_gain.first, key);

            // Remove the attribute from the data
            new_data = remove_attribute(new_data, max_gain.first);

            // Construct the tree recursively
            connection->set_child(construct_tree(child, new_data, val.first, val.second));
        }
    }

    return current;
}

/**
 * @brief Test the decision tree with the testing data and append the outcome to the testing data
 * 
 * @param tree Tree to test
 * @param data Data to test the tree with
 * 
 * @return double 
 */
double test_tree(Tree &tree, CSVData* data) {
    int counter = 0;
    double correct = 0;

    data->first.push_back("PredictedWillWait");

    // Loop through each row in the testing data
    for (auto& data_row : data->second) {
        Node* current = tree.get_root();
        size_t last = data_row.size() - 1;

        // Loop until we reach a leaf node
        while(current->get_connections().size() > 0) {
            std::string attribute = current->get_attribute();

            // Find the attribute in the testing data
            auto it = std::find(data->first.begin(), data->first.end(), attribute);
            std::string response = data_row[std::distance(data->first.begin(), it)]; // It should always be found, so no need to check

            // Compare the response to the child nodes
            for (size_t j = 0; j < current->get_connections().size(); j++) {
                Connection* connection = current->get_connections()[j];

                if (connection->get_answer().compare(response) == 0) {
                    current = connection->get_child();
                    break;
                }
            }

            // Sometimes the decision tree doesn't have a match due to how the data is split
            if (current->get_attribute().compare(attribute) == 0) {
                break;
            }
        }

        // Check if the outcome is correct
        if (current->get_attribute().compare(data_row[last]) == 0) {
            correct++;
        }

        // Append the outcome to the row
        data_row.push_back(current->get_attribute());

        counter++;
    }

    return (correct / data->second.size()) * 100;
}

/**
 * @brief Calculate the gain of a given attribute
 * 
 * @param attribute_index Index of the attribute
 * @param data Data to calculate the gain for
 * @param total_entropy Total entropy of the whole data set
 * @return double 
 */
double gain(int attribute_index, CSVData data, double total_entropy) {
    AttributeValues attribute_values; 
    double remainder = 0;
    double p = 0;
    double n = 0;

    // Loop through the data and count the number of positive and negative outcomes for each attribute value
    attribute_values = get_attribute_values(data, attribute_index);

    // Calculate the remainder
    for (auto const& [key, val] : attribute_values) {
        p = val.first;
        n = val.second;

        remainder += ((p + n) / data.second.size()) * entropy(p, n);
    }

    return total_entropy - remainder;
}

/**
 * @brief Calculate the entropy of a set of data
 * 
 * @param p Number of positive outcomes
 * @param n Number of negative outcomes
 * @return double 
 */
double entropy(double p, double n) {
    double total = p + n;

    if (total == 0) return 0;

    double p_ratio = p / total;

    if (p_ratio == 1 || p_ratio == 0) return 0;

    double p_log = p_ratio * log2(p_ratio);

    return -(p_log + (1 - p_ratio) * log2(1 - p_ratio));
}

/**
 * @brief Find the attribute with the maximum gain
 * 
 * @param data Data to find the maximum gain for
 * @param p Number of positive outcomes for the whole data set
 * @param n Number of negative outcomes for the whole data set
 * @return MaxGain 
 */
MaxGain find_max_gain(CSVData data, double p, double n) {
    // Find the attribute with the maximum gain
    double attr_gain = 0;
    
    size_t max_gain_index = 1; // Ignore the first attribute (ID)
    double max_gain = 0;

    for (size_t i = 1; i < data.first.size() - 1; i++) { // Ignore the first and last attribute (ID/outcome)
        attr_gain = gain(i, data, entropy(p, n));

        if (attr_gain > max_gain) {
            max_gain = attr_gain;
            max_gain_index = i;
        }
    }

    return std::make_pair(max_gain_index, max_gain);
}

AttributeValues get_attribute_values(CSVData data, int attribute_index) {
    AttributeValues attribute_values = {};

    for (size_t j = 0; j < data.second.size(); j++) {
        if (data.second[j][data.second[j].size() - 1].compare("Yes") == 0) {
            attribute_values[data.second[j][attribute_index]].first++;
        } else {
            attribute_values[data.second[j][attribute_index]].second++;
        }
    }

    return attribute_values;
}

/**
 * @brief Get the rows with a specific attribute value
 * 
 * @param data 
 * @param attribute_index 
 * @param attribute_value 
 * @return CSVData 
 */
CSVData get_attribute_rows(CSVData data, int attribute_index, std::string attribute_value) {
    // Create a new data set
    CSVData new_data = std::make_pair(data.first, std::vector<std::vector<std::string>>());

    for (size_t i = 0; i < data.second.size(); i++) {
        if (data.second[i][attribute_index].compare(attribute_value) == 0) {
            new_data.second.push_back(data.second[i]);
        }
    }

    return new_data;
}

/**
 * @brief Remove an attribute from the header and data
 * 
 * @param data 
 * @param index 
 */
CSVData remove_attribute(CSVData data, size_t index) {
    CSVData new_data = data;

    new_data.first.erase(new_data.first.begin() + index);

    for (size_t i = 0; i < new_data.second.size(); i++) {
        new_data.second[i].erase(new_data.second[i].begin() + index);
    }

    return new_data;
}

/**
 * @brief Load a CSV file
 * 
 * @param filename 
 * @param header 
 * @param data 
 */
void load(std::string filename, CSVData &data) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Get the first line
    
    // Split the line by commas
    std::stringstream ss(line);
    std::string token;

    // Load the header
    while (std::getline(ss, token, ',')) {
        // Null terminate the string
        token.erase(token.find_last_not_of(" \n\r\t") + 1);

        data.first.push_back(token);
    }

    // Load the data
    while (std::getline(file, line)) {
        std::vector<std::string> decision;

        // Split the line by commas
        std::stringstream ss(line);
        std::string token;

        for (size_t i = 0; i < data.first.size(); i++) {
            std::getline(ss, token, ',');

            // Null terminate the string
            token.erase(token.find_last_not_of(" \n\r\t") + 1);

            decision.push_back(token);
        }

        data.second.push_back(decision);
    }

    file.close();
}
