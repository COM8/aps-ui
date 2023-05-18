#include "CallWidget.hpp"
#include "backend/teams/Connection.hpp"
#include "logger/Logger.hpp"
#include "spdlog/spdlog.h"
#include "ui/utils/UiUtils.hpp"
#include <chrono>
#include <memory>
#include <date/date.h>
#include <gtkmm/box.h>
#include <gtkmm/enums.h>
#include <sigc++/connection.h>

namespace ui::widgets::teams {
CallWidget::CallWidget() : mainBox(Gtk::Orientation::VERTICAL) {
    prep_widget();
}

void CallWidget::prep_widget() {
    set_child(mainBox);

    mainBox.append(label);
    mainBox.append(descLabel);

    get_style_context()->add_provider(get_css_provider(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    descLabel.add_css_class("dim-label");
    descLabel.set_margin_top(3);
}

void CallWidget::set_call(backend::teams::CallEvent&& call) {
    this->call = std::move(call);

    label.set_label("📞   " + (this->call.from.empty() ? this->call.id : this->call.from));

    descLabel.set_label(get_local_time());

    remove_css_class("call-end");
    add_css_class("call");

    // Cancel the hide timeout:
    if (hideTimeout) {
        hideTimeout->disconnect();
        hideTimeout = nullptr;
    }

    set_visible(true);
}

std::string CallWidget::get_local_time() {
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::chrono::seconds offsetUtc(-timezone);
    std::chrono::hours daylightUtc(daylight);
    tp = tp + offsetUtc + offsetUtc;
    return date::format("%H:%M:%S", std::chrono::floor<std::chrono::seconds>(tp));
}

void CallWidget::set_call_end(backend::teams::CallEndEvent&& callEnd) {
    this->callEnd = std::move(callEnd);

    label.set_label("☎️   " + (this->callEnd.from.empty() ? this->callEnd.id : this->callEnd.from));
    descLabel.set_label(get_local_time() + " - " + (this->callEnd.phrase.empty() ? this->callEnd.reason : this->callEnd.phrase));

    remove_css_class("call");
    add_css_class("call-end");

    // Restart the hide timeout:
    if (hideTimeout) {
        hideTimeout->disconnect();
        hideTimeout = nullptr;
    }

    sigc::connection hideTimeout = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &CallWidget::on_call_ended_timeout), 60);
    this->hideTimeout = std::make_unique<sigc::connection>(std::move(hideTimeout));
}

//-----------------------------Events:-----------------------------
bool CallWidget::on_call_ended_timeout() {
    set_visible(false);
    SPDLOG_INFO("Call ended timeout reached.");
    return false;
}

void CallWidget::on_clicked() {
    SPDLOG_INFO("Call button clicked. Hiding.");
    set_visible(false);
}
}  // namespace ui::widgets::teams