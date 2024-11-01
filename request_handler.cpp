#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

using namespace transport;


RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {
    //
}

std::optional<RouteStatistics> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    const auto bus = db_.GetBus(bus_name);
    return db_.GetStat(bus);    
}

const std::set<BusPtr>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetBusses4Stop(stop_name);    
}

const svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetDocument();
}
