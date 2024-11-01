#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {
    void MapRenderer::AddObject(std::unique_ptr<svg::Drawable>&& obj) {
        objects_.push_back(std::move(obj));
    }
    
    const svg::Document MapRenderer::GetDocument() const {
        svg::Document doc;
        for (auto& it : objects_) {
            it->Draw(doc);
        }
        return doc;
    }
}
