#include "PlotWidget.hpp"
#include "backend/gitlab/GitLabHelper.hpp"
#include "backend/storage/Settings.hpp"
#include "logger/Logger.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <random>
#include <vector>
#include <cairo.h>
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
    ctx->save();

    ctx->set_line_width(2);
    ctx->set_source_rgba(GRID_COLOR.get_red(), GRID_COLOR.get_green(), GRID_COLOR.get_blue(), GRID_COLOR.get_alpha());

    // Rows:
    const double rowHeight = (static_cast<double>(height) * MAX_HEIGHT_CORRECTION) / static_cast<double>(runnerCount);
    for (size_t i = 1; i <= runnerCount; i++) {
        ctx->move_to(0, height - static_cast<double>(i) * rowHeight);
        ctx->line_to(width, height - static_cast<double>(i) * rowHeight);
        ctx->stroke();

        ctx->set_font_size(rowHeight * 0.75);
        ctx->move_to(10, height - ((static_cast<double>(i - 1)) * rowHeight) - (rowHeight * 0.25));
        ctx->text_path(std::to_string(i));
    }

    // Columns:
    const size_t colWidth = 75;
    size_t colCount = static_cast<size_t>(width) / colWidth;
    for (size_t i = 1; i < colCount + 2; i++) {
        ctx->move_to(static_cast<double>(i * colWidth), 0);
        ctx->line_to(static_cast<double>(i * colWidth), height);
        ctx->stroke();
    }

    ctx->restore();
}

void PlotWidget::draw_data(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height, const pointArrType_t& points, pointArrType_t& curOffset, const Gdk::RGBA& color) const {
    if (curSize <= 0 || points.empty()) {
        return;
    }

    ctx->save();
    ctx->set_operator(Cairo::Context::Operator::SOURCE);

    double maxHeight = static_cast<double>(height) * MAX_HEIGHT_CORRECTION;
    double yStep = maxHeight / static_cast<double>(runnerCount);
    double xStep = static_cast<double>(width) / static_cast<double>(points.size());

    // Line:
    ctx->begin_new_path();
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
    ctx->set_line_width(2);
    ctx->set_line_join(Cairo::Context::LineJoin::ROUND);
    ctx->move_to(0, height);

    size_t index = 0;
    double x = 0;
    for (size_t i = 0; i < curSize; i++) {
        index = (curIndex + i) % MAX_POINT_COUNT;

        x = curSize >= MAX_POINT_COUNT && i + 1 >= curSize ? width : static_cast<double>(i) * xStep;
        const double y = static_cast<double>(curOffset[index]) * yStep;

        if (i == 0) {
            ctx->move_to(x, height - y);
        } else {
            ctx->line_to(x, height - y);
        }

        curOffset[index] -= points[index];
    }

    if (curSize < MAX_POINT_COUNT) {
        ctx->line_to(x, height);
    }
    ctx->stroke_preserve();
    ctx->line_to(x, height);

    // Polygon:
    ctx->line_to(0, height);
    ctx->close_path();
    ctx->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha() * 0.3);
    ctx->fill();

    ctx->restore();
}

Gdk::RGBA PlotWidget::random_color() {
    static const std::array<char, 16> HEX_CHARS{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'E', 'F'};

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<size_t> normalBrightness(0, 15);
    static std::uniform_int_distribution<size_t> highBrightness(8, 15);
    static std::uniform_int_distribution<size_t> isBrightComponent(0, 2);

    std::string tmp = "#";
    size_t brightPart = isBrightComponent(gen);
    for (size_t i = 0; i < 3; i++) {
        if (i == brightPart) {
            tmp += HEX_CHARS[highBrightness(gen)];
            tmp += HEX_CHARS[highBrightness(gen)];
        } else {
            tmp += HEX_CHARS[normalBrightness(gen)];
            tmp += HEX_CHARS[normalBrightness(gen)];
        }
    }
    tmp += "FF";  // Disable alpha
    return Gdk::RGBA{std::move(tmp)};
}

void PlotWidget::update_data() {
    const backend::storage::Settings* settings = backend::storage::get_settings_instance();
    const backend::gitlab::GitLabStats stats = backend::gitlab::request_stats(settings->data.gitlabRunnerUrl);

    pointsMutex.lock();

    runnerCount = stats.runnerCount;

    // Update index:
    if (curSize <= 0) {
        curSize = 1;
        curIndex = 0;
    } else if (curSize >= MAX_POINT_COUNT) {
        curSize = MAX_POINT_COUNT;
        curIndex = (curIndex + 1) % MAX_POINT_COUNT;
    } else {
        curSize++;
    }

    // Update all existing points with 0
    size_t index = curSize <= 1 ? 0 : (curIndex + curSize - 1) % MAX_POINT_COUNT;
    for (auto& entry : points) {
        std::tuple<pointArrType_t, Gdk::RGBA>& p = entry.second;
        pointArrType_t& arr = std::get<0>(p);
        arr[index] = 0;
    }

    // Insert and update all new data points:
    for (const auto& entry : stats.status) {
        if (points.contains(entry.first)) {
            std::get<0>(points[entry.first])[index] = entry.second;
        } else {
            std::get<0>(points[entry.first])[index] = entry.second;
            std::get<1>(points[entry.first]) = random_color();
        }
    }

    pointsMutex.unlock();
    disp.emit();
}

void PlotWidget::thread_run() {
    SPDLOG_INFO("GitLab thread started.");
    while (shouldRun) {
        update_data();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
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
    pointsMutex.lock();
    // Grid:
    draw_grid(ctx, width, height);
    ctx->push_group();

    // Points:
    pointArrType_t curOffset{};
    curOffset.fill(runnerCount);
    for (const auto& entry : points) {
        const std::tuple<pointArrType_t, Gdk::RGBA>& p = entry.second;
        draw_data(ctx, width, height, std::get<0>(p), curOffset, std::get<1>(p));
    }
    ctx->pop_group_to_source();
    ctx->paint();
    pointsMutex.unlock();
}

void PlotWidget::on_notification_from_update_thread() {
    queue_draw();
}
}  // namespace ui::widgets::gitlab