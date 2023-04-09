#include "PlotWidget.hpp"
#include "backend/gitlab/GitLabHelper.hpp"
#include "backend/storage/Settings.hpp"
#include "logger/Logger.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <random>
#include <vector>
#include <cairomm/context.h>
#include <gtkmm/enums.h>

namespace ui::widgets::gitlab {
PlotWidget::PlotWidget() {
    prep_widget();
    disp.connect(sigc::mem_fun(*this, &PlotWidget::on_notification_from_update_thread));
    start_thread();
}

PlotWidget::~PlotWidget() {
    if (shouldRun) {
        stop_thread();
    }
}

void PlotWidget::prep_widget() {
    set_draw_func(sigc::mem_fun(*this, &PlotWidget::on_draw_handler));
    set_can_target(false);
    set_can_focus(false);

    set_hexpand();
    set_vexpand();
    set_overflow(Gtk::Overflow::HIDDEN);
    set_margin(6);
    set_margin_end(3);
    add_css_class("card");
}

void PlotWidget::draw_grid(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height) const {
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

void PlotWidget::draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const pointArrType_t& points, double maxVal, const Gdk::RGBA& color) const {
    if (curSize <= 0) {
        return;
    }

    double increment = static_cast<double>(width) / static_cast<double>(points.size());
    double maxHeight = static_cast<double>(height) * 0.95;

    // Line:
    ctx->set_line_width(2);
    ctx->set_line_join(Cairo::Context::LineJoin::ROUND);
    ctx->move_to(0, height - (points[curIndex] / maxVal * maxHeight));

    size_t index = 0;
    double x = 0;
    for (size_t i = 1; i < curSize; i++) {
        index = (curIndex + i) % MAX_POINT_COUNT;
        x = curSize >= MAX_POINT_COUNT && i + 1 >= curSize ? width : static_cast<double>(i) * increment;
        const double y = points[index] / maxVal * maxHeight;
        ctx->line_to(x, height - y);
    }
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
    ctx->stroke_preserve();

    // Polygon:
    ctx->line_to(x, height);
    ctx->line_to(0, height);
    ctx->close_path();
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha() * 0.3);
    ctx->fill();
}

void PlotWidget::update_data() {
    const backend::storage::Settings* settings = backend::storage::get_settings_instance();
    backend::gitlab::request_stats(settings->data.gitlabRunnerUrl);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(-1, 1);

    pointsMutex.lock();

    size_t prevIndex = 0;
    if (curSize <= 0) {
        curSize = 1;
        curIndex = 0;
    } else if (curSize >= MAX_POINT_COUNT) {
        curSize = MAX_POINT_COUNT;
        prevIndex = (curIndex - 1 + curSize) % MAX_POINT_COUNT;
        curIndex = (curIndex + 1) % MAX_POINT_COUNT;
    } else {
        prevIndex = curSize - 1;
        curSize++;
    }

    for (std::tuple<pointArrType_t, Gdk::RGBA>& p : points) {
        pointArrType_t& arr = std::get<0>(p);

        if (curSize <= 1) {
            arr[0] = 50 * (1 + dis(gen));
        } else {
            size_t index = (curIndex + curSize - 1) % MAX_POINT_COUNT;
            arr[index] = arr[prevIndex] + dis(gen);
            if (arr[index] < 0) {
                arr[index] = 0;
            }
        }
    }
    pointsMutex.unlock();
    disp.emit();
}

void PlotWidget::thread_run() {
    SPDLOG_INFO("GitLab thread started.");
    while (shouldRun) {
        update_data();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    SPDLOG_INFO("GitLab thread stoped.");
}

void PlotWidget::start_thread() {
    assert(!updateThread);
    assert(!shouldRun);
    shouldRun = true;
    updateThread = std::make_unique<std::thread>(&PlotWidget::thread_run, this);
}

void PlotWidget::stop_thread() {
    assert(updateThread);
    assert(shouldRun);
    shouldRun = false;
    updateThread->join();
    updateThread = nullptr;
}

//-----------------------------Events:-----------------------------
void PlotWidget::on_draw_handler(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height) {
    // Grid:
    draw_grid(ctx, width, height);

    // Points:
    pointsMutex.lock();
    double maxVal = 0;
    for (const std::tuple<pointArrType_t, Gdk::RGBA>& p : points) {
        const pointArrType_t& vec = std::get<0>(p);
        const double tmp = *std::max_element(vec.begin(), vec.end());
        if (maxVal < tmp) {
            maxVal = tmp;
        }
    }

    for (const std::tuple<pointArrType_t, Gdk::RGBA>& p : points) {
        draw_data(ctx, width, height, std::get<0>(p), maxVal, std::get<1>(p));
    }
    pointsMutex.unlock();
}

void PlotWidget::on_notification_from_update_thread() {
    queue_draw();
}
}  // namespace ui::widgets::gitlab