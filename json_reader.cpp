#include "json_reader.h"
#include "json.h"

#include <algorithm>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
using namespace json;

json::Document JsonReader::ApplyCommands([[maybe_unused]] transport::TransportCatalogue& catalogue) const {
    for (const auto& cmd : commands_.stop_requests) {
        catalogue.AddStop(cmd.name, cmd.place);        
    }
    for (const auto& cmd : commands_.stop_requests) {
        for (const auto& to : cmd.road_distances) {
            catalogue.AddDistance(cmd.name, to.stop, to.distance);
        }       
    }
    for (const auto& cmd : commands_.bus_requests) {
        catalogue.AddBus(cmd.name, cmd.stops);        
    }
    Array ans;
    for (const auto& cmd : commands_.stat_requests) {
        json::Dict result;
        switch (cmd.type) {
            case StatType::Bus: {
                const auto bus = catalogue.GetBus(cmd.name);
                const auto stat = catalogue.GetStat(bus);
                result["request_id"] = cmd.id;
                if (!stat) {
                    result["error_message"] = "not found";
                } else {
                    result["curvature"] = stat->curvature;
                    result["route_length"] = stat->dist;
                    result["stop_count"] = static_cast<int>(stat->stops_count);
                    result["unique_stop_count"] = static_cast<int>(stat->unique_stops);
                }
                break;
            }
            case StatType::Stop: {
                const auto buses4stop = catalogue.GetBusses4Stop(cmd.name);
                result["request_id"] = cmd.id;
                if (!buses4stop) {
                    result["error_message"] = "not found";
                } else {
                    Array buses;
                    for (const auto& bus : *buses4stop) {
                        buses.push_back(std::string(bus));
                    }
                    result["buses"] = buses;
                }
                break;
            }
        }
        ans.push_back(result);
    }
    return Document(ans);
}

void JsonReader::ParseCommands(std::istream& in) {
    Document doc = json::Load(in);
    const auto& root = doc.GetRoot().AsMap();
    
    for (auto ptr = root.find("base_requests"); ptr != root.end(); ptr = root.end()) {
        const auto& requests = ptr->second.AsArray();
        for (const auto& r : requests) {
            const auto& base_request = r.AsMap();
            std::string type = base_request.at("type").AsString();
            std::string name = base_request.at("name").AsString();
            if (type == "Stop") {
                StopRequest ans;
                ans.name = name;
                ans.place = {base_request.at("latitude").AsDouble(), base_request.at("longitude").AsDouble()};
                if (base_request.count("road_distances")) {
                    for (const auto& [id, dist] : base_request.at("road_distances").AsMap()) {
                        ans.road_distances.push_back({id, dist.AsInt()});
                    }
                }
                commands_.stop_requests.push_back(ans);
            } else if (type == "Bus") {
                BusRequest ans;
                ans.name = name;
                const auto& stops = base_request.at("stops").AsArray();          
                for (const auto& stop : stops) {
                    ans.stops.push_back(commands_.AddId(stop.AsString()));
                }
                ans.is_roundtrip = base_request.at("is_roundtrip").AsBool();
                if (!ans.is_roundtrip) {
                    std::vector<std::string_view> return_stops;
                    for (auto it = ans.stops.rbegin(); it != ans.stops.rend();++it) {
                        if (it == ans.stops.rbegin()) {
                            continue;
                        }
                        return_stops.push_back(*it);
                    }
                    for (const auto& s : return_stops) {
                        ans.stops.push_back(s);
                    }
                }
                commands_.bus_requests.push_back(ans);
            }
        }
    }
    for (auto ptr = root.find("stat_requests"); ptr != root.end(); ptr = root.end()) {
        const auto& requests = ptr->second.AsArray();
        for (const auto& req : requests) {
            StatRequest ans;
            const auto& r = req.AsMap();
            ans.id = r.at("id").AsInt();
            std::string type = r.at("type").AsString();
            if (type == "Bus") {
                ans.type = StatType::Bus;
            } else if (type == "Stop") {
                ans.type = StatType::Stop;
            }
            ans.name = r.at("name").AsString();
            commands_.stat_requests.push_back(ans);
        }
    }
}
