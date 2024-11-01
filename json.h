#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    enum class NodeType {
        TNULL,
        TINT,
        TDOUBLE,
        TBOOL,
        TSTRING,
        TARRAY,
        TMAP
    };
    
    Node() {}
    
    template <class T>
    Node(T value) {
        value_ = value;
    }
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    const Value& GetValue() const { return value_; }
    
    bool operator== (const Node& other) const;
    bool operator!= (const Node& other) const;
    NodeType GetType() const;
private:
    struct TypeGetter {
        TypeGetter(NodeType& result) : result_(result){
            //
        }
        void operator() (const std::nullptr_t&) const {
            result_ = NodeType::TNULL;
        }
        void operator() (const int&) const {
            result_ = NodeType::TINT;
        }
        void operator() (const double&) const {
            result_ = NodeType::TDOUBLE;
        }
        void operator() (const bool&) const {
            result_ =  NodeType::TBOOL;
        }
        void operator() (const std::string&) const {
            result_ = NodeType::TSTRING;
        }
        void operator() (const Dict&) const {
            result_ = NodeType::TMAP;
        }
        void operator() (const Array&) const {
            result_ = NodeType::TARRAY;
        }
        
    private:
        NodeType& result_;
    };

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator== (const Document& other);
    bool operator!= (const Document& other);

private:
    Node root_;
};

Document Load(std::istream& input);


// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    std::ostream& PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
        return out;
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const bool& value, const PrintContext& ctx);
void PrintValue(std::nullptr_t, const PrintContext& ctx);

void PrintValue(const std::string& value, const PrintContext& ctx);

void PrintValue(const Array& arr, const PrintContext& ctx);
void PrintValue(const Dict& dict, const PrintContext& ctx);
void PrintValue(const Node& dict, const PrintContext& ctx);

void PrintNode(const Node& node, const PrintContext& ctx);


void Print(const Document& doc, std::ostream& output);


}// namespace json
