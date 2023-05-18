#include "CallWidget.hpp"
#include "backend/teams/Connection.hpp"
#include "ui/utils/UiUtils.hpp"
#include <chrono>
#include <date/date.h>
#include <gtkmm/box.h>
#include <gtkmm/enums.h>

namespace ui::widgets::teams {
CallWidget::CallWidget() : Gtk::Box(Gtk::Orientation::VERTICAL) {
    prep_widget();
}

void CallWidget::prep_widget() {
    append(label);
    append(descLabel);

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
}

//-----------------------------Events:-----------------------------
}  // namespace ui::widgets::teams