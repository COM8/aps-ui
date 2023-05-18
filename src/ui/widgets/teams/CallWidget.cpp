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
    label.add_css_class("call");
}

void CallWidget::set_call(backend::teams::CallEvent&& call) {
    this->call = std::move(call);
    update_ui();
}

void CallWidget::update_ui() {
    label.set_label("📞   " + (call.from.empty() ? call.id : call.from));
}

//-----------------------------Events:-----------------------------
}  // namespace ui::widgets::teams