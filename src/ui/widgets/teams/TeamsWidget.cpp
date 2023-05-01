#include "TeamsWidget.hpp"
#include "backend/storage/Settings.hpp"
#include "backend/teams/Connection.hpp"
#include "ui/widgets/teams/CallWidget.hpp"
#include <memory>
#include <string>

namespace ui::widgets::teams {
TeamsWidget::TeamsWidget() {
    prep_widget();

    disp.connect(sigc::mem_fun(*this, &TeamsWidget::on_notification_from_update_thread));

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
    set_margin(6);
    set_margin_end(3);
    append(callWidget);
    callWidget.set_visible(false);
}

void TeamsWidget::update_ui() {
    callMutex.lock();
    callWidget.set_call(backend::teams::CallEvent(call));
    callMutex.unlock();
    callWidget.set_visible(true);
}

//-----------------------------Events:-----------------------------
void TeamsWidget::on_call_event(const backend::teams::CallEvent& event) {
    callMutex.lock();
    call = event;
    callMutex.unlock();
    disp.emit();
}

void TeamsWidget::on_notification_from_update_thread() {
    update_ui();
}
}  // namespace ui::widgets::teams