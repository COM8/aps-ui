#pragma once

#include "backend/teams/Connection.hpp"
#include <memory>
#include <glibmm/dispatcher.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <sigc++/connection.h>

namespace ui::widgets::teams {
class ActivityWidget : public Gtk::Button {
 private:
    backend::teams::ActivityEvent activity{};

    Gtk::Label label{};

 public:
    ActivityWidget();
    ActivityWidget(ActivityWidget&&) = default;
    ActivityWidget(const ActivityWidget&) = delete;
    ActivityWidget& operator=(ActivityWidget&&) = default;
    ActivityWidget& operator=(const ActivityWidget&) = delete;
    ~ActivityWidget() override = default;

    void set_activity(backend::teams::ActivityEvent&& activity);

 private:
    void prep_widget();

    //-----------------------------Events:-----------------------------
    void on_clicked() override;
};
}  // namespace ui::widgets::teams