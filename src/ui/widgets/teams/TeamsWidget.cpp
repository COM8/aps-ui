#include "TeamsWidget.hpp"
#include "backend/storage/Settings.hpp"
#include "backend/teams/Connection.hpp"
#include <memory>
#include <string>

namespace ui::widgets::teams {
TeamsWidget::TeamsWidget() {
    prep_widget();

    // Start connection:
    backend::storage::Settings* settings = backend::storage::get_settings_instance();
    connection = std::make_unique<backend::teams::Connection>(std::string{settings->data.teamsHookEndpoint}, std::string{settings->data.teamsHookAuthentication});
    connection->start();
    connection->onCallEvent.append([this](const backend::teams::CallEvent& event) { this->on_call_event(event); });
}

TeamsWidget::~TeamsWidget() {
    connection->stop();
}

void TeamsWidget::prep_widget() {
    set_overflow(Gtk::Overflow::HIDDEN);
    set_margin(6);
    set_margin_end(3);
}

//-----------------------------Events:-----------------------------
void TeamsWidget::on_call_event(const backend::teams::CallEvent& /*event*/) {
}
}  // namespace ui::widgets::teams