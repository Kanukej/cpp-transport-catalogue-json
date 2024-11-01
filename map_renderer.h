#pragma once

#include "svg.h"

#include <memory>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {
    
    class MapRenderer {
    public:
        void AddObject(std::unique_ptr<svg::Drawable>&& obj);
        const svg::Document GetDocument() const;
    private:
        std::vector<std::unique_ptr<svg::Drawable>> objects_;
    };
    
}
