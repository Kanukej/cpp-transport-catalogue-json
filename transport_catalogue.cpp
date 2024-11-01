#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"
#include <sstream>

using namespace transport;
using namespace geo;

std::string_view TransportCatalogue::AddId(const std::string_view id) {
    ids_.push_front(std::string {id});
    return {ids_.front().begin(), ids_.front().end()};
}

void TransportCatalogue::AddStop(const std::string_view id, const Coordinates place) {
    std::string_view stop_id = AddId(id);
    stops_.insert({stop_id, {stop_id, place, {}}});
    busses4stop_.insert({stop_id, {}});
}

void TransportCatalogue::AddBus(const std::string_view id, const std::vector<std::string_view> stops) {
    std::string_view bus_id = AddId(id);
    std::vector<std::string_view> stops_ids;
    stops_ids.reserve(stops.size());
    for (const auto& stop : stops) {
        stops_ids.push_back(stops_.at(stop).id);
        busses4stop_[stops_ids.back()].insert(bus_id);
    }
    busses_.insert({bus_id, {bus_id, std::move(stops_ids)}});
}

void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, const int dist) {
    stops_.at(from).distances[stops_.at(to).id] = dist;
}

const BusDescription* TransportCatalogue::GetBus(const std::string_view id) const {
    auto bus_ptr = busses_.find(id);
    if (bus_ptr != busses_.end()) {
        return &bus_ptr->second;
    }
    return nullptr;
}

const std::optional<RouteStatistics> TransportCatalogue::GetStat(const BusDescription* bus) const {
    if (bus) {
        double route_length = 0.0;
        int route_dist = 0;
        std::unordered_set<std::string_view> unique_stops;
        std::optional<StopDescription> prev_stop;
        for (const std::string_view& s : bus->stops) {
            const auto stop = stops_.at(s);
            unique_stops.insert(s);
            if (prev_stop) {
                const auto& distances = prev_stop->distances;
                auto dist_ptr = distances.find(s);
                if (dist_ptr != distances.end()) {
                    route_dist += dist_ptr->second;
                } else if (stop.distances.end() != (dist_ptr = stop.distances.find(prev_stop->id))) {
                    route_dist += dist_ptr->second;
                } else {
                    std::stringstream ss;
                    ss << "distance between stop " << s << " and stop " << prev_stop->id << " not found in base";
                    throw std::out_of_range(ss.str());
                }
                route_length += ComputeDistance(prev_stop->place, stop.place);
            }
            prev_stop = stop;
        }
        return RouteStatistics {route_dist, bus->stops.size(), unique_stops.size(), route_dist / route_length};
    }
    return std::nullopt;
}

const std::set<BusPtr>* TransportCatalogue::GetBusses4Stop(const std::string_view id) const {
    auto busses4stop_ptr = busses4stop_.find(id);
    if (busses4stop_ptr != busses4stop_.end()) {
        return &busses4stop_ptr->second;
    }
    return nullptr;
}
