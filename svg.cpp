#include "svg.h"

#define _USE_MATH_DEFINES 
#include <cmath>

using namespace std;

namespace {
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline.SetFillColor("red"s).SetStrokeColor("black"s);
}
}

namespace svg {

using namespace std::literals;
using namespace shapes;
    
std::ostream& operator << (std::ostream &os, const StrokeLineCap &cap) {
    switch(cap) {
        case StrokeLineCap::BUTT : return os << "butt"; 
        case StrokeLineCap::ROUND : return os << "round";  
        case StrokeLineCap::SQUARE : return os << "square";
        default: return os;
    }
    return os;
}
    
std::ostream& operator << (std::ostream &os, const StrokeLineJoin &join)
{
    switch(join) {
        case StrokeLineJoin::ARCS : return os << "arcs"; 
        case StrokeLineJoin::BEVEL : return os << "bevel";  
        case StrokeLineJoin::MITER : return os << "miter";
        case StrokeLineJoin::MITER_CLIP : return os << "miter-clip";  
        case StrokeLineJoin::ROUND : return os << "round";
            
        default: return os;
    }
    return os;
}

    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    PathProps::WriteProps(out);
    out << "/>"sv;
}
    
// ----------Document-----------------
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}
    
void Document::Render(std::ostream& out) const {
    RenderContext context(out);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        out << "  "sv;
        obj->Render(context);
    }
    out << "</svg>"sv << std::endl;
}
    
// ----------Text---------------------

Text& Text::SetPosition(Point pos) {
    anchor_point_ = pos;
    return *this;
}
    
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
    
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}
    
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";    
    PathProps::WriteProps(out);
    out << " x=\""sv << anchor_point_.x << "\" y=\""sv << anchor_point_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">";
    for (const auto& ch : data_) {
        if (ch == '\"') {
            out << "&quot;";
        } else if (ch == '\'') {
            out << "&apos;";
        } else if (ch == '<') {
            out << "&lt;";
        } else if (ch == '>') {
            out << "&gt;";
        } else if (ch == '&') {
            out << "&amp;";
        } else {
            out << ch;
        }
    }
    out << "</text>"sv;
}
    
// ----------Polyline------------------
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool need_space = false;
    for (const auto& p : points_) {
        if (need_space) {
            out << ' ';
        } else {
            need_space = true;
        }
        out << p.x << ',' << p.y;
    }
    out << "\"";
    PathProps::WriteProps(out);
    out << "/>"sv;
}

template<class T>
void PathProps<T>::WriteProps(std::ostream& out) const {
            if (fill_) {
                out << " fill=\""sv << *fill_ << "\""sv;
            }
            if (stroke_) {
                out << " stroke=\""sv << *stroke_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }
    
void Star::Draw(svg::ObjectContainer& container) const {
    container.Add(::CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
}
    
void Snowman::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Circle().SetCenter({center_.x, center_.y + 5 * rad_}).SetRadius(2.0 * rad_).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
    container.Add(svg::Circle().SetCenter({center_.x, center_.y + 2 * rad_}).SetRadius(1.5 * rad_).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
    container.Add(svg::Circle().SetCenter(center_).SetRadius(rad_).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
}
    
void Triangle::Draw(svg::ObjectContainer& container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

}  // namespace svg
