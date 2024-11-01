#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

#include <iostream>

using namespace std;
using namespace transport;
using namespace json;

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */
    TransportCatalogue db;
    JsonReader reader;
    reader.ParseCommands(cin);
    const auto ans = reader.ApplyCommands(db);
    Print(ans, cout);
}
