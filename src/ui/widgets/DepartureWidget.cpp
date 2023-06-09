#include "DepartureWidget.hpp"
#include "backend/db/Departure.hpp"
#include "spdlog/spdlog.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <date/date.h>
#include <date/tz.h>
#include <gtkmm/box.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/enums.h>
#include <gtkmm/label.h>

namespace ui::widgets {
DepartureWidget::DepartureWidget() {
    prep_widget();
}

void DepartureWidget::prep_widget() {
    set_margin_bottom(5);

    mainBox.set_margin_start(10);
    mainBox.set_margin_end(10);
    set_child(mainBox);

    // Header:
    mainBox.append(headerBox);
    label.set_width_chars(5);
    label.get_style_context()->add_provider(labelProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    label.add_css_class("departure-background");
    headerBox.append(label);
    canceled.set_margin_start(10);
    headerBox.append(canceled);
    destination.set_single_line_mode(true);
    destination.set_ellipsize(Pango::EllipsizeMode::END);
    destination.set_margin_start(10);
    headerBox.append(destination);

    // Info:
    mainBox.append(infoBox);
    infoBox.append(depInfo);

    // Delay Info:
    mainBox.append(delayMsg);
    delayMsg.set_wrap(true);
    delayMsg.set_halign(Gtk::Align::START);
}

void DepartureWidget::update_departure_ui() {
    // Header:
    label.set_label(departure->trainName);
    static const std::string S_BAHN_COLOR = "#408335";
    labelProvider->load_from_data(".departure-background { color: #FFFFFF; background-image: image(" + S_BAHN_COLOR + "); }");
    canceled.set_visible(departure->canceled);

    std::string destinationStr;

    if (departure->destination != departure->destinationScheduled) {
        destinationStr += "<span strikethrough='true' strikethrough_color='#8b0000'>";
        destinationStr += departure->destinationScheduled;
        destinationStr += "</span> ";
    }
    destinationStr += "<b>" + departure->destination + "</b>";

    std::string viaStr;
    for (const backend::db::Stop& stop : departure->nextStops) {
        if (stop.showVia) {
            if (!viaStr.empty()) {
                viaStr += ", ";
            }
            viaStr += stop.name;
        }
    }

    if (!viaStr.empty()) {
        destinationStr += " via " + viaStr;
    }

    destination.set_markup(destinationStr);

    // Info:
    if (!departure->canceled) {
        std::string depInfoStr;

        const date::time_zone* tz = date::current_zone();
        auto depTimeScheduled = date::make_zoned(tz, departure->depTimeScheduled).get_local_time();
        auto depTime = date::make_zoned(tz, departure->depTime).get_local_time();
        auto now = date::make_zoned(tz, std::chrono::system_clock::now()).get_local_time();

        auto diff = depTime - now;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff).count();

        // Platform changes:
        if (departure->platform != departure->platformScheduled) {
            depInfoStr += "<span background='#FFFFFF' foreground='#000000' strikethrough='true' strikethrough_color='#8b0000'>";
            depInfoStr += " " + departure->platformScheduled + " ";
            depInfoStr += "</span> ";

            depInfoStr += "<span background='#0087FF' foreground='#FFFFFF'>";
            depInfoStr += " " + departure->platform + " ";
            depInfoStr += "</span> ";
        } else {
            depInfoStr += "<span background='#FFFFFF' foreground='#000000'>";
            depInfoStr += " " + departure->platform + " ";
            depInfoStr += "</span> ";
        }

        // Departure Time:
        if (departure->depTime != departure->depTimeScheduled) {
            depInfoStr += "<span strikethrough='true' strikethrough_color='#8b0000'>";
            depInfoStr += date::format("%H:%M", depTimeScheduled);
            depInfoStr += "</span> ";

            depInfoStr += "<span font_weight='bold' foreground='";
            if (departure->delay > 0) {
                depInfoStr += "#8b0000";
            } else {
                depInfoStr += "#008b00";
            }
            depInfoStr += "'>";
            depInfoStr += date::format("%H:%M", depTime);
            depInfoStr += " </span> ";

        } else {
            depInfoStr += date::format("%H:%M", depTime) + " ";
        }

        // Departure minutes:
        if (minutes <= 0) {
            depInfoStr += "NOW";
        } else {
            depInfoStr += "in " + std::to_string(minutes) + " min.";
        }

        // Delay:
        if (departure->delay != 0) {
            depInfoStr += " (";
            depInfoStr += "<span font_weight='bold' foreground='";
            if (departure->delay > 0) {
                depInfoStr += "#8b0000";
            } else {
                depInfoStr += "#008b00";
            }
            depInfoStr += "'>";
            depInfoStr += std::to_string(departure->delay);
            depInfoStr += " </span>";
            depInfoStr += " min. delay)";
        }
        depInfo.set_markup(depInfoStr);
    }
    depInfo.set_visible(!departure->canceled);

    // Delay Info:
    if (!departure->infoMessages.empty()) {
        std::string msg = "⚠️ <span foreground='#8b0000'>";

        for (size_t i = 0; i < departure->infoMessages.size(); i++) {
            msg += departure->infoMessages[i];

            if (i < departure->infoMessages.size() - 1) {
                msg += ", ";
            }
        }

        msg += "</span>";
        delayMsg.set_markup(msg);
    }
    delayMsg.set_visible(!departure->infoMessages.empty());
}

void DepartureWidget::set_departure(std::shared_ptr<backend::db::Departure> departure) {
    assert(departure);
    this->departure = std::move(departure);
    update_departure_ui();
}
}  // namespace ui::widgets