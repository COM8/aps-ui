#pragma once

#include "backend/teams/Connection.hpp"
#include "ui/widgets/teams/CallWidget.hpp"
#include <memory>
#include <gtkmm/box.h>

namespace ui::widgets::teams {
class TeamsWidget : public Gtk::Box {
 private:
    std::unique_ptr<backend::teams::Connection> connection{nullptr};

    Glib::Dispatcher disp;
    std::mutex callMutex{};

    CallWidget callWidget{};
    backend::teams::CallEvent call{};

 public:
    TeamsWidget();
    TeamsWidget(TeamsWidget&&) = delete;
    TeamsWidget(const TeamsWidget&) = delete;
    TeamsWidget& operator=(TeamsWidget&&) = delete;
    TeamsWidget& operator=(const TeamsWidget&) = delete;
    ~TeamsWidget() override;

 private:
    void prep_widget();
    void update_ui();

    //-----------------------------Events:-----------------------------
    void on_call_event(const backend::teams::CallEvent& event);
    void on_notification_from_update_thread();
};
}  // namespace ui::widgets::teams