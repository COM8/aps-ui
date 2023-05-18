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
    backend::teams::CallEndEvent callEnd;

    Gtk::Label label{};
    Gtk::Label descLabel{};

 public:
    CallWidget();
    CallWidget(CallWidget&&) = default;
    CallWidget(const CallWidget&) = delete;
    CallWidget& operator=(CallWidget&&) = default;
    CallWidget& operator=(const CallWidget&) = delete;
    ~CallWidget() override = default;

    void set_call(backend::teams::CallEvent&& call);
    void set_call_end(backend::teams::CallEndEvent&& callEnd);

 private:
    void prep_widget();
    static std::string get_local_time();
    //-----------------------------Events:-----------------------------
};
}  // namespace ui::widgets::teams