#pragma once

#include <array>
#include <cstddef>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <glibmm/dispatcher.h>
#include <gtkmm/drawingarea.h>

namespace ui::widgets::gitlab {
class PlotWidget : public Gtk::DrawingArea {
 private:
    bool shouldRun{false};
    std::unique_ptr<std::thread> updateThread{nullptr};
    Glib::Dispatcher disp;

    const Gdk::RGBA GRID_COLOR{"#575757"};

    static const size_t MAX_POINT_COUNT = static_cast<size_t>(60 * 5);
    size_t curIndex{0};
    size_t curSize{0};
    using pointArrType_t = std::array<size_t, MAX_POINT_COUNT>;

    std::unordered_map<std::string, std::tuple<pointArrType_t, Gdk::RGBA>> points{};

    // std::array<std::tuple<pointArrType_t, Gdk::RGBA>, 5> points{
    //     std::make_tuple<pointArrType_t, Gdk::RGBA>({}, Gdk::RGBA{"#a51d2d"}),
    //     std::make_tuple<pointArrType_t, Gdk::RGBA>({}, Gdk::RGBA{"#24634c"}),
    //     std::make_tuple<pointArrType_t, Gdk::RGBA>({}, Gdk::RGBA{"#1d3f91"}),
    //     std::make_tuple<pointArrType_t, Gdk::RGBA>({}, Gdk::RGBA{"#acb323"}),
    //     std::make_tuple<pointArrType_t, Gdk::RGBA>({}, Gdk::RGBA{"#a923b3"})};
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
    void draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const pointArrType_t& points, size_t maxVal, const Gdk::RGBA& color) const;
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