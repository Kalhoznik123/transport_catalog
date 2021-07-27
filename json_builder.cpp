#include "json_builder.h"


json::Builder &json::Builder::EndArray() {
  if (!nodes_stack_.back()->IsArray() || nodes_stack_.empty()) {
    throw std::logic_error("EndArray закрывает не массив."s);
  }
  nodes_stack_.pop_back();

  return *this;
}

json::Node json::Builder::Build() {
  if (root_.IsNull()) {
    throw std::logic_error("Вызов Build при неготовом объекте."s);
  }
  if(nodes_stack_.size()>1 && (nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict())){
    throw std::logic_error("Вызов Build при неготовом объекте."s);

  }
  return root_;
}

json::Builder::Builder():nodes_stack_(1,&root_){

}

json::KeyValueContext json::Builder::Key(std::string key) {
  if (root_.IsNull() || nodes_stack_.empty()) {
    throw std::logic_error("Вызов любого метода, кроме Build, при готовом объекте."s);
  }
  if (key_ || !nodes_stack_.back()->IsDict()) {
    throw std::logic_error("Вызов метода Key снаружи словаря или сразу после другого Key."s);
  }
  key_ = move(key);

  return *this;
}

json::Builder &json::Builder::Value(json::Node::Value value){

  const  auto back_item = nodes_stack_.back();

  if(back_item->IsNull()){

    back_item->GetValue() = std::move(value);

  }else if(back_item->IsArray()){
    Array& arr = std::get<Array>(back_item->GetValue());

     arr.push_back(std::visit(NodeGetter{},std::move(value)));
  }else if(back_item->IsDict()){
    if (!key_) {
      throw std::logic_error("Попытка добавить элемент в словарь без ключа."s);
    }
    Dict & dict = std::get<Dict>(back_item->GetValue());

    dict.emplace(*key_, std::visit(NodeGetter{},std::move(value)));
    key_ = std::nullopt;
  }else{
    throw std::logic_error("Вызов любого метода, кроме Build, при готовом объекте."s);
  }

  return *this;
}

json::DictItemContext json::Builder::StartDict() {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Вызов любого метода, кроме Build, при готовом объекте."s);
  }

  const auto back_element = nodes_stack_.back();
  if (back_element->IsNull()) {
    *back_element = Dict();
    nodes_stack_.push_back(nodes_stack_.back());
  }else if(back_element->IsArray()){
    Array & arr = std::get<Array>(back_element->GetValue());
    arr.emplace_back(json::Dict());
    nodes_stack_.push_back(&arr.back());

  }else if (back_element->IsDict()){
    if (!key_) {
      throw std::logic_error("Попытка добавить элемент в словарь без ключа."s);
    }
    Dict & dict = std::get<Dict>(back_element->GetValue());

    const auto dictionary = dict.emplace(*key_,json::Dict());
    nodes_stack_.push_back(&dictionary.first->second);
    key_ = std::nullopt;
  }
  return *this;
}


json::ArrayItemContext json::Builder::StartArray() {
  if (nodes_stack_.empty()) {
    throw std::logic_error("Вызов любого метода, кроме Build, при готовом объекте."s);
  }

  const auto back_element = nodes_stack_.back();
  if (back_element->IsNull()) {
    *back_element = Array();
    nodes_stack_.push_back(nodes_stack_.back());
  } else if(back_element->IsArray()){
    Array & arr = std::get<Array>(back_element->GetValue());
    arr.emplace_back(json::Array());
    nodes_stack_.push_back(&arr.back());

  }else if (back_element->IsDict()){
    if (!key_) {
      throw std::logic_error("Попытка добавить элемент в словарь без ключа."s);
    }
    Dict & dict = std::get<Dict>(back_element->GetValue());

    const auto dictionary = dict.emplace(*key_,json::Array());
    nodes_stack_.push_back(&dictionary.first->second);
    key_ = std::nullopt;
  }
  return *this;
}

json::Builder &json::Builder::EndDict() {
  if (!nodes_stack_.back()->IsDict() || nodes_stack_.empty()) {
    throw std::logic_error("EndDict закрывает не словарь."s);
  }
  nodes_stack_.pop_back();

  return *this;
}


// ------ DictItemContext -------------
json::DictItemContext::DictItemContext(Builder& builder) : builder_(builder) {}
json::KeyValueContext json::DictItemContext::Key(std::string key) {
  return builder_.Key(std::move(key));
}
json::Builder& json::DictItemContext::EndDict() {
  return builder_.EndDict();
}

// ------ ArrayItemContext ------------
json::ArrayItemContext::ArrayItemContext(Builder& builder) : builder_(builder) {}
json::ArrayItemContext json::ArrayItemContext::Value(Node::Value value) {
  builder_.Value(std::move(value));
  return ArrayItemContext{builder_};
}
json::DictItemContext json::ArrayItemContext::StartDict() {
  return builder_.StartDict();
}
json::ArrayItemContext json::ArrayItemContext::StartArray() {
  return builder_.StartArray();
}
json::Builder& json::ArrayItemContext::EndArray() {
  return builder_.EndArray();
}

// ------ KeyValueContext -------------
json::KeyValueContext::KeyValueContext(Builder& builder) : builder_(builder) {}
json::DictItemContext json::KeyValueContext::Value(Node::Value value) {
  builder_.Value(std::move(value));
  return DictItemContext{builder_};
}
json::DictItemContext json::KeyValueContext::StartDict() {
  return builder_.StartDict();
}
json::ArrayItemContext json::KeyValueContext::StartArray() {
  return builder_.StartArray();
}
