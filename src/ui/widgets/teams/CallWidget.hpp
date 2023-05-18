#pragma once

#include "backend/teams/Connection.hpp"
#include <memory>
#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>

namespace ui::widgets::teams {
class CallWidget : public Gtk::Box {
 private:
    backend::teams::CallEvent call;

    Gtk::Label label{};

 public:
    CallWidget();
    CallWidget(CallWidget&&) = default;
    CallWidget(const CallWidget&) = delete;
    CallWidget& operator=(CallWidget&&) = default;
    CallWidget& operator=(const CallWidget&) = delete;
    ~CallWidget() override = default;

    void set_call(backend::teams::CallEvent&& call);

 private:
    void prep_widget();
    void update_ui();

    //-----------------------------Events:-----------------------------
};
}  // namespace ui::widgets::teams