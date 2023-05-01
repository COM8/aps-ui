#pragma once

#include "backend/teams/Connection.hpp"
#include <memory>
#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>

namespace ui::widgets::teams {
class TeamsWidget : public Gtk::Box {
 private:
    std::unique_ptr<backend::teams::Connection> connection{nullptr};

 public:
    TeamsWidget();
    TeamsWidget(TeamsWidget&&) = default;
    TeamsWidget(const TeamsWidget&) = delete;
    TeamsWidget& operator=(TeamsWidget&&) = default;
    TeamsWidget& operator=(const TeamsWidget&) = delete;
    ~TeamsWidget() override;

 private:
    void prep_widget();

    //-----------------------------Events:-----------------------------
    void on_call_event(const backend::teams::CallEvent& event);
};
}  // namespace ui::widgets::teams