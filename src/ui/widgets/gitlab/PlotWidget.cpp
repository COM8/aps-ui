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
#include <string>
#include <vector>
#include <cairo.h>
#include <cairomm/context.h>
#include <gdkmm/rgba.h>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/enums.h>
#include <gtkmm/label.h>

namespace ui::widgets::gitlab {
// NOLINTNEXTLINE(cert-err58-cpp)
const std::array<Gdk::RGBA, 10> PlotWidget::COLORS{Gdk::RGBA("#ffbe0b"), Gdk::RGBA("#606c38"), Gdk::RGBA("#780000"), Gdk::RGBA("#003566"), Gdk::RGBA("#9f86c0"), Gdk::RGBA("#fb5607"), Gdk::RGBA("#ff006e"), Gdk::RGBA("#8338ec"), Gdk::RGBA("#3a86ff"), Gdk::RGBA("#669bbc")};
// NOLINTNEXTLINE(cert-err58-cpp)
const Gdk::RGBA PlotWidget::GRID_COLOR{"#575757"};

PlotWidget::PlotWidget() : Gtk::Box(Gtk::Orientation::VERTICAL) {
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
    set_hexpand();
    set_vexpand();
    set_margin(6);
    set_margin_end(3);

    drawingArea.set_draw_func(sigc::mem_fun(*this, &PlotWidget::on_draw_handler));
    drawingArea.set_can_target(false);
    drawingArea.set_can_focus(false);

    drawingArea.set_hexpand();
    drawingArea.set_vexpand();
    drawingArea.set_overflow(Gtk::Overflow::HIDDEN);
    drawingArea.add_css_class("card");
    append(drawingArea);

    append(flowBox);
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

Gdk::RGBA PlotWidget::get_color() {
    const size_t index = colorIndex++;
    if (colorIndex >= COLORS.size()) {
        colorIndex = 0;
    }
    return COLORS[index];
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
            std::get<1>(points[entry.first]) = get_color();

            // Inform the UI thread that there is a new point source:
            newPoints.emplace_back(entry.first);
        }
    }

    pointsMutex.unlock();
    disp.emit();
}

void PlotWidget::thread_run() {
    SPDLOG_INFO("GitLab thread started.");
    while (shouldRun) {
        update_data();
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
    drawingArea.queue_draw();

    pointsMutex.lock();
    // Insert new labels:
    for (const std::string& name : newPoints) {
        // Create new label:
        labels.emplace_back(name + ": " + std::to_string(std::get<0>(points[name])[curIndex]));
        labels.back().set_margin(1);

        // Apply pill shape:
        const Gdk::RGBA color = std::get<1>(points[name]);
        Glib::RefPtr<Gtk::CssProvider> pillCssProvider = Gtk::CssProvider::create();
        pillCssProvider->load_from_data(".pill { border-radius: 9999px; padding: 2px 4px; border: 2px solid " + color.to_string() + "; background-color: alpha(" + color.to_string() + ", 0.3); }");
        labels.back().get_style_context()->add_provider(pillCssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        labels.back().add_css_class("pill");

        // Insert in flow box:
        flowBox.insert(labels.back(), -1);
    }
    newPoints.clear();

    // Update labels.
    // This is inefficient and should be replace at some point in the future with a more efficient one.
    for (const auto& point : points) {
        for (Gtk::Label& label : labels) {
            if (label.get_label().find(point.first) != std::string::npos) {
                const size_t count = std::get<0>(point.second)[curIndex];
                label.set_label(point.first + ": " + std::to_string(count));
                break;
            }
        }
    }

    pointsMutex.unlock();
}
}  // namespace ui::widgets::gitlab