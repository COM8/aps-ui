#include "CallWidget.hpp"
#include "backend/teams/Connection.hpp"

namespace ui::widgets::teams {
CallWidget::CallWidget() {
    prep_widget();
}

void CallWidget::prep_widget() {
    append(label);

    label.get_style_context()->add_provider(styleProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    styleProvider->load_from_data(".call-style { color: #FFFFFF; background-image: image(#337700); }");
    label.add_css_class("call-style");
}

void CallWidget::set_call(backend::teams::CallEvent&& call) {
    this->call = std::move(call);
    update_ui();
}

void CallWidget::update_ui() {
    label.set_label(call.from.empty() ? call.id : call.from);
}

//-----------------------------Events:-----------------------------
}  // namespace ui::widgets::teams