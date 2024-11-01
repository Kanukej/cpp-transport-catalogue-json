#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

class JsonReader {
public:
    JsonReader() = default;
    
    void ParseCommands(std::istream& in);
    
    json::Document ApplyCommands([[maybe_unused]] transport::TransportCatalogue& catalogue) const;
private:
    Commands commands_;
};
