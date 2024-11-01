#pragma once

#include <cmath>

namespace geo {

    static constexpr int const& EarthRadius = 6371000;
    static constexpr double const& Delta = 3.1415926535 / 180.;

    struct Coordinates {
        double lat;
        double lng;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}
