#include "ActivityWidget.hpp"
#include "backend/teams/Connection.hpp"
#include "logger/Logger.hpp"
#include "spdlog/spdlog.h"
#include "ui/utils/UiUtils.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <date/date.h>
#include <gtkmm/box.h>
#include <gtkmm/enums.h>
#include <sigc++/connection.h>

namespace ui::widgets::teams {
ActivityWidget::ActivityWidget() {
    prep_widget();
}

void ActivityWidget::prep_widget() {
    get_style_context()->add_provider(get_css_provider(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    add_css_class("activity");
}

void ActivityWidget::set_activity(backend::teams::ActivityEvent&& activity) {
    this->activity = activity;
    set_label("🔔   " + std::to_string(activity.count));
    set_visible(activity.count > 0);
}

//-----------------------------Events:-----------------------------
void ActivityWidget::on_clicked() {
    SPDLOG_INFO("Activity button clicked. Hiding.");
    set_visible(false);
}
}  // namespace ui::widgets::teams