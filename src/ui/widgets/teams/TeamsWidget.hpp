#pragma once

#include "backend/teams/Connection.hpp"
#include "ui/widgets/teams/ActivityWidget.hpp"
#include "ui/widgets/teams/CallWidget.hpp"
#include <memory>
#include <gtkmm/box.h>

namespace ui::widgets::teams {
class TeamsWidget : public Gtk::Box {
 private:
    std::unique_ptr<backend::teams::Connection> connection{nullptr};

    Glib::Dispatcher callDisp;
    Glib::Dispatcher callEndDisp;
    Glib::Dispatcher activityDisp;
    std::mutex callMutex{};
    std::mutex activityMutex{};

    CallWidget callWidget{};
    ActivityWidget activityWidget{};
    backend::teams::CallEvent call{};
    backend::teams::CallEndEvent callEnd{};
    backend::teams::ActivityEvent activity{};

 public:
    TeamsWidget();
    TeamsWidget(TeamsWidget&&) = delete;
    TeamsWidget(const TeamsWidget&) = delete;
    TeamsWidget& operator=(TeamsWidget&&) = delete;
    TeamsWidget& operator=(const TeamsWidget&) = delete;
    ~TeamsWidget() override;

 private:
    void prep_widget();

    //-----------------------------Events:-----------------------------
    void on_call_event(const backend::teams::CallEvent& event);
    void on_call_end_event(const backend::teams::CallEndEvent& event);
    void on_activity_end_event(const backend::teams::ActivityEvent& event);
    void on_call_notification_from_ws();
    void on_call_end_notification_from_ws();
    void on_activity_end_notification_from_ws();
};
}  // namespace ui::widgets::teams