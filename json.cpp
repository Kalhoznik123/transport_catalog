#include "json.h"
#include <cmath>
#include <set>
using namespace std;

namespace json {

namespace {



}  // namespace

bool Node::IsNull() const{
    // проверем элемент на нулёвость
    return (std::get_if<std::nullptr_t>(this) != nullptr);
}

bool Node::IsArray() const {
    return (std::get_if<Array>(this) != nullptr);
}

bool Node::IsMap() const {
    return (std::get_if<Dict>(this) != nullptr);
}

bool Node::IsBool() const {
    return (std::get_if<bool>(this) != nullptr);
}

bool Node::IsInt() const {
    return (std::get_if<int>(this) != nullptr);
}

bool Node::IsDouble() const {
    // целые числа являются подмножеством чисел с плавающей запятой
    return (std::get_if<double>(this) != nullptr || std::get_if<int>(this) != nullptr);
}

bool Node::IsPureDouble() const {
    if (std::get_if<double>(this) == nullptr) {
        return false;
    }
    return true;
}

bool Node::IsString() const {
    return (std::get_if<std::string>(this) != nullptr);
}

const Array& Node::AsArray() const {
    if ( std::get_if<Array>(this) == nullptr ) {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
    return *std::get_if<Array>(this);
}

const Dict& Node::AsMap() const {
    if ( std::get_if<Dict>(this) == nullptr ) {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
    return *std::get_if<Dict>(this);
}

int Node::AsInt() const {
    if ( std::get_if<int>(this) == nullptr ) {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
    return *std::get_if<int>(this);
}

const string& Node::AsString() const {
    if ( std::get_if<std::string>(this) == nullptr ) {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
    return *std::get_if<std::string>(this);
}

bool Node::AsBool() const {
    if ( std::get_if<bool>(this) == nullptr ) {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
    return *std::get_if<bool>(this);
}

double Node::AsDouble() const {
    if (std::get_if<double>(this) != nullptr) {
        return *std::get_if<double>(this);
    }

    else if (std::get_if<int>(this) != nullptr) {
        return static_cast<double>(*std::get_if<int>(this));
    } else {
        using namespace std::string_literals;
        throw std::logic_error("Неверный тип данных"s);
    }
}

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    char c = ' ';
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') {
        using namespace std::string_literals;
        throw ParsingError("Неверный массив на входе"s);
    }
    return Node(result);
}

Node LoadBool(std::istream& input) {
    std::string value;
    for (char c; input >> c;) {
        value += c;
        if (value == "true"s) {
            return Node(true);
        }
        if (value == "false"s) {
            return Node(false);
        }
        if (value.size() == 5) {
            break;
        }
    }
    using namespace std::string_literals;
    throw ParsingError("Неверный булевый тип"s);
    return Node();
}

Node LoadNull(std::istream& input) {
    std::string value;
    for (char c; input >> c;) {
        value += c;
        if (value == "null"s) {
            return Node(nullptr);
        }
        if (value.size() == 4) {
            break;
        }
    }
    throw ParsingError("Неверный тип на входе"s);
    return Node();
}

Node NodeNumber(const Number& number) {
    if (std::get_if<int>(&number) != nullptr) {
        return Node(*std::get_if<int>(&number));
    }
    else {
        return Node(*std::get_if<double>(&number));
    }
}

Node LoadString(std::istream& input) {
    input >> std::noskipws;
    std::string line;
    char c;
    char prev = '\\';
    while (input >> c) {
        if (c == '"' && prev != '\\') {
            // окончание строки
            break;
        }
        prev = c;
        // если нашли символ экранирования, то смотрим, что за ним
        if (c == '\\') {
            char temp;
            input >> temp;
            // опрелелили следующий символ за чертой
            switch (temp) {
            case 'n':
                line += '\n';
                break;
            case 'r':
                line += '\r';
                break;
            case 't':
                line += '\t';
                break;
            case '\"':
                line += '\"';
                break;
            default:
                line += temp;
                break;
            }
        }
        else {
           line += c;
        }
    }
    input >> std::skipws;
    if (line.size() == 0 || c != '"') {
        throw ParsingError("Пустая строка на входе"s);
    }
    return Node(move(line));
}

Node LoadDict(std::istream& input) {
    Dict result;
    char c = ' ';
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}') {
        throw ParsingError("Неверный словарь на входе"s);
    }
    return Node(move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        // bool value
        input.putback(c);
        return LoadBool(input);
    } else if (c == 'n') {
        // null value
        input.putback(c);
        return LoadNull(input);
    } else {
        // digit value
        input.putback(c);
        return NodeNumber(LoadNumber(input));
    }
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

const NodeValue& Node::GetValue() const {
    return *this;
}

void Print(const Document& doc, std::ostream& output) {
    std::visit(OutNode{output}, doc.GetRoot().GetValue());
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return root_ != other.root_;
}

void OutNode::operator()(std::nullptr_t) const {
    using namespace std::string_view_literals;
    out << "null"sv;
}

void OutNode::operator()(Array array) const {
    // example
    // "[1, 1.23, \"Hello\"]"s
    // выводим вектор поэлементно
    using namespace std::string_view_literals;
    out << "["sv;
    for (size_t i = 0; i < array.size(); ++i) {
        std::visit(OutNode{out},array[i].GetValue());
        if (i+1 != array.size()) {
            out << ","sv;
        }
    }
    out << "]"s;
}

void OutNode::operator()(Dict map) const {
    // example
    // "{ \"key1\": \"value1\", \"key2\": 42 }"s
    // выводим словарь поэлементно
    using namespace std::string_view_literals;
    int flag = 0;
    out << "{"sv;
    for (const auto& [key, node] : map) {
        if (flag == 1) {
            out << ","sv;
        }
        out << "\""sv << key << "\":"sv;
        std::visit(OutNode{out},node.GetValue());
        out << ""sv;
        flag = 1;
    }
    out << "}"sv;
}

void OutNode::operator()(bool value) const {
    out << std::boolalpha << value;
}

void OutNode::operator()(int value) const {
    out << value;
}

void OutNode::operator()(double value) const {
    out << value;
}

void OutNode::operator()(const std::string& str) const {
    using namespace std::string_view_literals;
    std::set<char> escape{'\"','\\','\t','\n','\r'};
    out << "\"";
    for (const char& c : str) {
        // если мы имеем escape-последовательность, то треба добавить косую черту
        if (escape.count(c)) {
            switch (c) {
            case '\"':
                out << '\\';
                out << '\"';
                break;
            case '\r':
                out << '\\';
                out << 'r';
                break;
            case '\t':
                out << '\\';
                out << 't';
                break;
            case '\n':
                out << '\\';
                out << 'n';
                break;
            case '\\':
                out << '\\';
                out << '\\';
                break;
            }
        }
        else {
            out << c;
        }
    }
    out << '\"';
}

}  // namespace json
