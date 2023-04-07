#include "PlotWidget.hpp"
#include <algorithm>
#include <random>
#include <vector>
#include <cairomm/context.h>
#include <gtkmm/enums.h>

namespace ui::widgets::gitlab {
PlotWidget::PlotWidget() {
    prep_widget();
    generate_data();
}

void PlotWidget::prep_widget() {
    set_draw_func(sigc::mem_fun(*this, &PlotWidget::on_draw_handler));
    set_can_target(false);
    set_can_focus(false);

    set_hexpand();
    set_vexpand();
    set_overflow(Gtk::Overflow::HIDDEN);
    set_margin(5);
    add_css_class("card");
}

void PlotWidget::generate_data() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-3, 3);

    for (std::tuple<std::vector<double>, Gdk::RGBA>& p : points) {
        std::vector<double>& vec = std::get<0>(p);
        vec.resize(2 * 60 * 60);
        double d = 25 * (2 + dis(gen));
        for (double& point : vec) {
            d += dis(gen);
            if (d < 0) {
                d = 0;
            }
            point = d;
        }
    }
}

void PlotWidget::draw_grid(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height) {
    ctx->set_line_width(2);
    ctx->set_source_rgba(GRID_COLOR.get_red(), GRID_COLOR.get_green(), GRID_COLOR.get_blue(), GRID_COLOR.get_alpha());

    // Rows:
    const size_t rowHeight = 50;
    const size_t rowCount = static_cast<size_t>(height) / rowHeight;
    for (size_t i = 1; i < rowCount + 1; i++) {
        ctx->move_to(0, static_cast<double>(i * rowHeight));
        ctx->line_to(width, static_cast<double>(i * rowHeight));
        ctx->stroke();
    }

    // Columns:
    const size_t colWidth = 75;
    size_t colCount = static_cast<size_t>(width) / colWidth;
    for (size_t i = 1; i < colCount + 2; i++) {
        ctx->move_to(static_cast<double>(i * colWidth), 0);
        ctx->line_to(static_cast<double>(i * colWidth), height);
        ctx->stroke();
    }
}

void PlotWidget::draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const std::vector<double>& points, double maxVal, const Gdk::RGBA& color) {
    double increment = static_cast<double>(width) / static_cast<double>(points.size());
    double maxHeight = static_cast<double>(height) * 0.95;

    // Line:
    ctx->set_line_width(2);
    ctx->set_line_join(Cairo::Context::LineJoin::ROUND);
    ctx->move_to(0, height - (points[0] / maxVal * maxHeight));

    for (size_t i = 1; i < points.size(); i++) {
        const double x = i >= points.size() - 1 ? width : static_cast<double>(i) * increment;
        const double y = points[i] / maxVal * maxHeight;
        ctx->line_to(x, height - y);
    }
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
    ctx->stroke_preserve();

    // Polygon:
    ctx->line_to(width, height);
    ctx->line_to(0, height);
    ctx->close_path();
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha() * 0.3);
    ctx->fill();
}

//-----------------------------Events:-----------------------------
void PlotWidget::on_draw_handler(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height) {
    double maxVal = 0;
    for (const std::tuple<std::vector<double>, Gdk::RGBA>& p : points) {
        const std::vector<double>& vec = std::get<0>(p);
        const double tmp = *std::max_element(vec.begin(), vec.end());
        if (maxVal < tmp) {
            maxVal = tmp;
        }
    }

    draw_grid(ctx, width, height);

    for (const std::tuple<std::vector<double>, Gdk::RGBA>& p : points) {
        draw_data(ctx, width, height, std::get<0>(p), maxVal, std::get<1>(p));
    }
}
}  // namespace ui::widgets::gitlab