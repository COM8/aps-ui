#pragma once

#include <array>
#include <cstddef>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <gdkmm/rgba.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/drawingarea.h>

namespace ui::widgets::gitlab {
class PlotWidget : public Gtk::DrawingArea {
 private:
    bool shouldRun{false};
    std::unique_ptr<std::thread> updateThread{nullptr};
    Glib::Dispatcher disp;

    static const std::array<Gdk::RGBA, 10> COLORS;
    size_t colorIndex{0};

    static const Gdk::RGBA GRID_COLOR;

    static constexpr size_t MAX_POINT_COUNT = static_cast<size_t>(60 * 5);
    size_t curIndex{0};
    size_t curSize{0};
    using pointArrType_t = std::array<size_t, MAX_POINT_COUNT>;
    size_t runnerCount{0};
    static constexpr double MAX_HEIGHT_CORRECTION = 0.97;

    std::unordered_map<std::string, std::tuple<pointArrType_t, Gdk::RGBA>> points{};
    std::mutex pointsMutex{};

 public:
    PlotWidget();
    PlotWidget(PlotWidget&&) = delete;
    PlotWidget(const PlotWidget&) = delete;
    PlotWidget& operator=(PlotWidget&&) = delete;
    PlotWidget& operator=(const PlotWidget&) = delete;
    ~PlotWidget() override;

 private:
    void prep_widget();
    Gdk::RGBA get_color();
    void draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const pointArrType_t& points, pointArrType_t& curOffset, const Gdk::RGBA& color) const;
    void draw_grid(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height) const;

    void start_thread();
    void stop_thread();

    void update_data();
    void thread_run();

    //-----------------------------Events:-----------------------------
    void on_draw_handler(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height);
    void on_notification_from_update_thread();
};
}  // namespace ui::widgets::gitlab