#include "TeamsWidget.hpp"
#include "backend/storage/Settings.hpp"
#include "backend/teams/Connection.hpp"
#include "ui/widgets/teams/CallWidget.hpp"
#include <memory>
#include <string>
#include <gtkmm/enums.h>

namespace ui::widgets::teams {
TeamsWidget::TeamsWidget() : Gtk::Box(Gtk::Orientation::HORIZONTAL) {
    prep_widget();

    callDisp.connect(sigc::mem_fun(*this, &TeamsWidget::on_call_notification_from_ws));
    callEndDisp.connect(sigc::mem_fun(*this, &TeamsWidget::on_call_end_notification_from_ws));

    // Start connection:
    backend::storage::Settings* settings = backend::storage::get_settings_instance();
    connection = std::make_unique<backend::teams::Connection>(std::string{settings->data.teamsHookEndpoint}, std::string{settings->data.teamsHookAuthentication});
    connection->start();
    connection->onCallEvent.append([this](const backend::teams::CallEvent& event) { this->on_call_event(event); });
    connection->onCallEndEvent.append([this](const backend::teams::CallEndEvent& event) { this->on_call_end_event(event); });
}

TeamsWidget::~TeamsWidget() {
    connection->stop();
}

void TeamsWidget::prep_widget() {
    set_margin(6);
    set_margin_end(3);

    append(callWidget);
    callWidget.set_visible(false);
    callWidget.set_halign(Gtk::Align::CENTER);
    callWidget.set_vexpand(true);
    callWidget.set_valign(Gtk::Align::CENTER);
}

//-----------------------------Events:-----------------------------
void TeamsWidget::on_call_event(const backend::teams::CallEvent& event) {
    callMutex.lock();
    call = event;
    callMutex.unlock();
    callDisp.emit();
}

void TeamsWidget::on_call_end_event(const backend::teams::CallEndEvent& event) {
    callMutex.lock();
    callEnd = event;
    callMutex.unlock();
    callEndDisp.emit();
}

void TeamsWidget::on_call_notification_from_ws() {
    callMutex.lock();
    callWidget.set_call(backend::teams::CallEvent(call));
    callMutex.unlock();
}

void TeamsWidget::on_call_end_notification_from_ws() {
    callMutex.lock();
    callWidget.set_call_end(backend::teams::CallEndEvent(callEnd));
    callMutex.unlock();
}
}  // namespace ui::widgets::teams