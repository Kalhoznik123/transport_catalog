#pragma once
#include "json.h"
#include <vector>
#include <optional>
#include <string>
#include <stdexcept>

namespace  json {
class DictItemContext;
class ArrayItemContext;
class KeyValueContext;

using namespace std::string_literals;



class Builder{

private:
  struct NodeGetter {
    Node operator() (std::nullptr_t) const {
      return Node();
    }
    Node operator() (std::string value) const {
      return Node(std::move(value));
    }
    Node operator() (bool value) const {
      return Node(value);
    }
    Node operator() (int value) const {
      return Node(value);
    }
    Node operator() (double value) const {
      return Node(value);
    }
    Node operator() (Array value) const {
      return Node(std::move(value));
    }
    Node operator() (Dict value) const {
      return Node(std::move(value));
    }
  };

public:
  Builder();
  KeyValueContext Key(std::string key);
  Builder& Value (Node::Value value);
  DictItemContext  StartDict();
  ArrayItemContext StartArray();
  Builder& EndDict();
  Builder& EndArray();
  Node Build();

private:

  Node root_;
  std::optional<std::string> key_;
  std::vector<Node*> nodes_stack_;
};




class ArrayItemContext {
public:
  ArrayItemContext(Builder& builder);
  ArrayItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
  Builder& EndArray();
private:
  Builder& builder_;
};

class KeyValueContext {
public:
  KeyValueContext(Builder& builder);
  DictItemContext Value(Node::Value value);
  DictItemContext StartDict();
  ArrayItemContext StartArray();
private:
  Builder& builder_;
};

class DictItemContext {
public:
  DictItemContext(Builder& builder);
  KeyValueContext Key(std::string key);
  Builder& EndDict();
private:
  Builder& builder_;
};


}
