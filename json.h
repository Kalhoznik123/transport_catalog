#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cassert>
#include <sstream>



namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

class Node : public NodeValue {
public:
    Node() = default;

    Node(Array array) : NodeValue(array) {}
    Node(Dict map) : NodeValue(map) {}
    Node(int value) : NodeValue(value) {}
    Node(bool value) : NodeValue(value) {}
    Node(double value) : NodeValue(value) {}
    Node(std::string value) : NodeValue(value) {}
    Node(std::nullptr_t) : NodeValue() {}


    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const NodeValue& GetValue() const;

private:

};

class Document;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using Number = std::variant<int, double>;

Document Load(std::istream& input);

Number LoadNumber(std::istream& input);

void Print(const Document& doc, std::ostream& output);

struct OutNode {
    std::ostream& out;

    void operator()(std::nullptr_t) const;
    void operator()(Array array) const;
    void operator()(Dict map) const;
    void operator()(bool value) const;
    void operator()(int value) const;
    void operator()(double value) const;
    void operator()(const std::string& str) const;
};


class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

}  // namespace json
