#include <sstream>
#include <cstring>
#include <cctype>

#include "json.h"

using namespace std;

namespace json {

namespace {

    
using Number = std::variant<int, double>;


Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw json::ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw json::ParsingError("A digit is expected"s);
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
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw json::ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw json::ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw json::ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw json::ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c != ']') {
        throw ParsingError("");
    }
    return Node(move(result));
}
    
bool CheckString(const std::string& str, istream& input) {
    for (const char ch : str) {
        if (ch != static_cast<char>(input.get())) {
            return false;
        }
    }
    return true;
}
    
bool CheckSep(char sep) {
    if (std::isspace(sep) ||  sep == ',' ||  sep == '}'||  sep == ']'){
        return true;
    }
    return false;
}
    
bool CheckSep(istream& input) {
    char sep = static_cast<char>(input.get());
    if (!std::isprint(sep) || std::isblank(sep)) {
        return true;
    }
    input.putback(sep);
    return CheckSep(sep);
}
    
Node LoadNull(istream& input) {
    if (!CheckString("null"s, input) || !CheckSep(input)) {
        throw ParsingError("");
    }
    
    return Node{};
}
    
Node LoadFalse(istream& input) {
    if(!CheckString("false"s, input) || !CheckSep(input)){
        throw ParsingError("");
    }
    return Node{false};
}
    
Node LoadTrue(istream& input) {
    if(!CheckString("true"s, input) || !CheckSep(input)){
        throw ParsingError("");
    }
    return Node{true};
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}') {
        throw ParsingError("");
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;
    
    while (std::isspace(c) && input >> c) {
        //
    }
    
    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't') {
        input.putback(c);
        return LoadTrue(input);
    } else if(c == 'f') {
        input.putback(c);
        return LoadFalse(input);
    } else {
        input.putback(c);
        const auto number = LoadNumber(input);
        const int* int_val = std::get_if<int>(&number);
        if (int_val != nullptr) {
            return Node(*int_val);
        }
        const double* dbl_val = std::get_if<double>(&number);
        if (dbl_val != nullptr) {
            return Node(*dbl_val);
        }
        throw ParsingError("");
    }
}
}
    
Node::NodeType Node::GetType() const {
    Node::NodeType result = Node::NodeType::TNULL;
    Node::TypeGetter getter(result);
    std::visit(getter, value_);
    return result;
}
bool Node::IsInt() const {
    return GetType() == NodeType::TINT;
}
bool Node::IsDouble() const {
    return GetType() == NodeType::TINT || GetType() == NodeType::TDOUBLE;
}
bool Node::IsPureDouble() const {
    return GetType() == NodeType::TDOUBLE;
}
bool Node::IsBool() const {
    return GetType() == NodeType::TBOOL;
}
bool Node::IsString() const {
    return GetType() == NodeType::TSTRING;
}
bool Node::IsNull() const {
    return GetType() == NodeType::TNULL;
}
bool Node::IsArray() const {
    return GetType() == NodeType::TARRAY;
}
bool Node::IsMap() const {
    return GetType() == NodeType::TMAP;
}

int Node::AsInt() const {
    try {
        return std::get<int>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
bool Node::AsBool() const {
    try {
        return std::get<bool>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
double Node::AsDouble() const {
    try {
        return std::get<double>(value_);
    } catch (const std::bad_variant_access& ex) {
        //
    }    
    try {
        return std::get<int>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
const std::string& Node::AsString() const {
    try {
        return std::get<std::string>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
const Array& Node::AsArray() const {
    try {
        return std::get<Array>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
const Dict& Node::AsMap() const {
    try {
        return std::get<Dict>(value_);
    } catch (const std::bad_variant_access& ex) {
        throw std::logic_error("");
    }
}
    

bool Node::operator== (const Node& other) const {
    if (GetType() != other.GetType()) {
        return false;
    }
    return value_ == other.GetValue();
}
bool Node::operator!= (const Node& other) const {    
    if (GetType() != other.GetType()) {
        return true;
    }
    return value_ != other.GetValue();
}
  /*  
bool operator== (const Node& left, const Node& right) {
    if (left.GetType() != right.GetType()) {
        return false;
    }
    return left.GetValue() == right.GetValue();
}
bool operator!= (const Node& left, const Node& right) {    
    if (left.GetType() != right.GetType()) {
        return true;
    }
    return left.GetValue() != right.GetValue();
}
*/
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const bool& value, const PrintContext& ctx) {
    if (value) {
        ctx.out << "true"sv;
    } else {
        ctx.out << "false"sv;
    }
}

void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(const std::string& value, const PrintContext& ctx) {
    ctx.out << "\""sv;
    for (const char c : value) {
        if (c == '\\') {
            ctx.out << "\\\\";
        } else if (c == '"') {
            ctx.out << "\\\"";
        } else if (c == '\r') {
            ctx.out << "\\r";
        } else if (c == '\n') {
            ctx.out << "\\n";
        } else if (c == '\t') {
            ctx.out << "\\t";
        } else {
            ctx.out << c;
        }
    }
    ctx.out << "\""sv;
}

void PrintValue(const Array& arr, const PrintContext& ctx) {
    ctx.out << "["sv << std::endl;
    bool comma = false;
    for (const auto& val : arr) {
        if (comma) {
            ctx.out << ","sv << std::endl;
        } else {
            comma = true;
        }
        ctx.Indented().PrintIndent();
        PrintValue(val, ctx);
    }
    ctx.out << std::endl;
    ctx.PrintIndent() << "]"sv;
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    ctx.out << "{"sv << std::endl;
    bool comma = false;
    for (const auto& [key, val] : dict) {
        if (comma) {
            ctx.out << ","sv << std::endl;
        } else {
            comma = true;
        }
        ctx.Indented().PrintIndent() << "\"" << key << "\" : "sv;
        PrintValue(val, ctx);
    }
    ctx.out << std::endl;
    ctx.PrintIndent() << "}"sv;
}
    
void PrintValue(const Node& node, const PrintContext& ctx) {
    PrintNode(node, ctx.Indented());
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}


void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext {output});
}  // namespace json


bool Document::operator== (const Document& other) {
    return GetRoot() == other.GetRoot();
}

bool Document::operator!= (const Document& other) {
    return GetRoot() != other.GetRoot();
}
}  // namespace json
