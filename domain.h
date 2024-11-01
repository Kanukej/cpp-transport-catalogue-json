#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <forward_list>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

enum class StatType {
    Bus,
    Stop
};

struct Dist2Stop {
    std::string_view stop;
    int distance;
};

struct StopDistance {
    std::string stop;
    int distance;
};

struct StopRequest {
    std::string name;
    geo::Coordinates place;
    std::vector<StopDistance> road_distances;
};

struct BusRequest {
    std::string name;
    std::vector<std::string_view> stops;
    bool is_roundtrip;
};

struct StatRequest {
    int id;
    StatType type;
    std::string name;
};

struct Commands {
    std::vector<StopRequest> stop_requests;
    std::vector<BusRequest> bus_requests;
    std::vector<StatRequest> stat_requests;
    std::string_view AddId(const std::string& id);
private:
    std::forward_list<std::string> ids_;
};
