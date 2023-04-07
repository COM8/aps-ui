#pragma once

#include <array>
#include <tuple>
#include <vector>
#include <gtkmm/drawingarea.h>

namespace ui::widgets::gitlab {
class PlotWidget : public Gtk::DrawingArea {
 private:
    const Gdk::RGBA GRID_COLOR{"#575757"};
    std::array<std::tuple<std::vector<double>, Gdk::RGBA>, 3> points{
        std::make_tuple<std::vector<double>, Gdk::RGBA>({}, Gdk::RGBA{"#a51d2d"}),
        std::make_tuple<std::vector<double>, Gdk::RGBA>({}, Gdk::RGBA{"#24634c"}),
        std::make_tuple<std::vector<double>, Gdk::RGBA>({}, Gdk::RGBA{"#1d3f91"})};

 public:
    PlotWidget();
    PlotWidget(PlotWidget&&) = delete;
    PlotWidget(const PlotWidget&) = delete;
    PlotWidget& operator=(PlotWidget&&) = delete;
    PlotWidget& operator=(const PlotWidget&) = delete;
    ~PlotWidget() override = default;

 private:
    void prep_widget();
    void generate_data();
    static void draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const std::vector<double>& points, double maxVal, const Gdk::RGBA& color);
    void draw_grid(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height);

    //-----------------------------Events:-----------------------------
    void on_draw_handler(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height);
};
}  // namespace ui::widgets::gitlab