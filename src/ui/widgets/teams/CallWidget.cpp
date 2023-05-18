#include "CallWidget.hpp"
#include "backend/teams/Connection.hpp"
#include "ui/utils/UiUtils.hpp"

namespace ui::widgets::teams {
CallWidget::CallWidget() {
    prep_widget();
}

void CallWidget::prep_widget() {
    append(label);

    label.get_style_context()->add_provider(get_css_provider(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void CallWidget::set_call(backend::teams::CallEvent&& call) {
    this->call = std::move(call);
    label.set_label("📞   " + (this->call.from.empty() ? this->call.id : this->call.from));

    label.remove_css_class("call-end");
    label.add_css_class("call");

    set_visible(true);
}

void CallWidget::set_call_end(backend::teams::CallEndEvent&& callEnd) {
    this->callEnd = std::move(callEnd);
    label.set_label("☎️   " + (this->callEnd.from.empty() ? this->callEnd.id : this->callEnd.from));

    label.remove_css_class("call");
    label.add_css_class("call-end");
}

//-----------------------------Events:-----------------------------
}  // namespace ui::widgets::teams