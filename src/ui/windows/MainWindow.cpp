#include "MainWindow.hpp"
#include "backend/systemUtils/SystemUtils.hpp"
#include "ui/utils/UiUtils.hpp"
#include <cstdint>
#include <memory>
#include <gdkmm/cursor.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/enums.h>
#include <gtkmm/grid.h>
#include <gtkmm/window.h>
#include <spdlog/spdlog.h>

namespace ui::windows {
MainWindow::MainWindow() : screenBrightnessBtn(32, 255, 1) {
    prep_window();
}

void MainWindow::prep_window() {
    set_title("Aps UI");
    set_default_size(800, 480);

// Make the window look like a debug window for debug builds:
#ifdef DEBUG
    add_css_class("devel");
#endif  // DEBUG

    // Keyboard events:
    Glib::RefPtr<Gtk::EventControllerKey> controller = Gtk::EventControllerKey::create();
    controller->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::on_key_pressed), false);
    add_controller(controller);

    // Content:
    stack = Gtk::make_managed<Gtk::Stack>();
    prep_overview_stack_page(stack);
    prep_lightning_stack_page(stack);
    prep_setting_stack_page(stack);
    set_child(*stack);

    // Header bar:
    Gtk::HeaderBar* headerBar = Gtk::make_managed<Gtk::HeaderBar>();

    inspectorBtn.set_label("🐞");
    inspectorBtn.set_tooltip_text("Inspector");
    inspectorBtn.signal_clicked().connect(&MainWindow::on_inspector_clicked);
    headerBar->pack_end(inspectorBtn);

    Gtk::StackSwitcher* stackSwitcher = Gtk::make_managed<Gtk::StackSwitcher>();
    stackSwitcher->set_stack(*stack);
    headerBar->set_title_widget(*stackSwitcher);
    set_titlebar(*headerBar);

    stack->property_visible_child_name().signal_changed().connect(sigc::mem_fun(*this, &MainWindow::on_page_changed));

    fullscreen();
}

void MainWindow::prep_overview_stack_page(Gtk::Stack* stack) {
    Gtk::Grid* mainGrid = Gtk::make_managed<Gtk::Grid>();
    mainGrid->set_halign(Gtk::Align::FILL);
    mainGrid->set_valign(Gtk::Align::FILL);
    mainGrid->set_vexpand(true);
    mainGrid->set_column_homogeneous(true);
    mainGrid->set_row_homogeneous(true);

    // GitLab:
    mainGrid->attach(plot, 0, 0);

    // Left bottom:
    Gtk::Box* leftBottomBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
    mainGrid->attach(*leftBottomBox, 0, 1);
    leftBottomBox->append(deviceStatus);
    leftBottomBox->set_vexpand(true);
    leftBottomBox->set_valign(Gtk::Align::FILL);
    deviceStatus.set_valign(Gtk::Align::END);
    leftBottomBox->set_hexpand(true);
    leftBottomBox->set_halign(Gtk::Align::CENTER);

    // Quick actions:
    leftBottomBox->append(quickActionsBox);
    quickActionsBox.set_halign(Gtk::Align::CENTER);
    quickActionsBox.set_valign(Gtk::Align::END);
    quickActionsBox.set_vexpand(true);
    quickActionsBox.set_margin_bottom(10);
    quickActionsBox.append(fullscreenBtn);
    fullscreenBtn.set_icon_name("window-grow-symbolic");
    fullscreenBtn.set_tooltip_text("Toggle fullscreen");
    fullscreenBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_toggle_full_screen_clicked));
    quickActionsBox.append(cursorBtn);
    cursorBtn.set_icon_name("mouse-wireless-disabled-filled-symbolic");
    cursorBtn.set_tooltip_text("Toggle cursor visibility");
    cursorBtn.set_margin_start(10);
    cursorBtn.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_toggle_cursor_clicked));
    quickActionsBox.append(screenSaverBtn);
    screenSaverBtn.set_icon_name("screen-privacy-symbolic");
    screenSaverBtn.set_tooltip_text("Enable screen saver");
    screenSaverBtn.set_margin_start(10);
    screenSaverBtn.signal_clicked().connect(sigc::ptr_fun(&MainWindow::on_screen_saver_clicked));
    quickActionsBox.append(screenBrightnessBtn);
    screenBrightnessBtn.set_icons({"display-brightness-high-symbolic"});
    screenBrightnessBtn.set_tooltip_text("Change the screen brightness");
    screenBrightnessBtn.set_margin_start(10);
    screenBrightnessBtn.set_value(static_cast<double>(backend::systemUtils::get_screen_brightness()));
    screenBrightnessBtn.signal_value_changed().connect(sigc::ptr_fun(&MainWindow::on_screen_brightness_value_changed));

    // DB:
    mainGrid->attach(db, 1, 0);

    // Weather:
    mainGrid->attach(weather, 1, 1);

    stack->add(*mainGrid, "overview", "Overview");
}

void MainWindow::prep_lightning_stack_page(Gtk::Stack* stack) {
    stack->add(lightning, "lightning", "Lightning");
}

void MainWindow::prep_setting_stack_page(Gtk::Stack* stack) {
    stack->add(settings, "settings", "Settings");
}

void MainWindow::hide_cursor() {
    set_cursor(Gdk::Cursor::create("none"));
    cursorBtn.set_icon_name("mouse-wireless-filled-symbolic");
    cursorHidden = true;
    SPDLOG_INFO("Cursor hidden.");
}

void MainWindow::show_cursor() {
    set_cursor();
    cursorBtn.set_icon_name("mouse-wireless-disabled-filled-symbolic");
    cursorHidden = false;
    SPDLOG_INFO("Cursor shown.");
}

//-----------------------------Events:-----------------------------
void MainWindow::on_inspector_clicked() {
    gtk_window_set_interactive_debugging(true);
}

void MainWindow::on_full_screen_clicked() { fullscreen(); }

void MainWindow::on_toggle_full_screen_clicked() {
    if (property_fullscreened().get_value()) {
        unfullscreen();
    } else {
        fullscreen();
    }
};

void MainWindow::on_toggle_cursor_clicked() {
    if (cursorHidden) {
        show_cursor();
    } else {
        hide_cursor();
    }
};

void MainWindow::on_screen_saver_clicked() {
    backend::systemUtils::activate_screensaver();
};

void MainWindow::on_screen_brightness_value_changed(double value) {
    if (value < 32) {
        value = 32;
    } else if (value > 255) {
        value = 255;
    }
    backend::systemUtils::set_screen_brightness(static_cast<uint8_t>(value));
}

bool MainWindow::on_key_pressed(guint keyVal, guint /*keyCode*/, Gdk::ModifierType /*modifier*/) {
    if (keyVal == GDK_KEY_Escape && property_fullscreened().get_value()) {
        unfullscreen();
        maximize();
        show_cursor();
        return true;
    }
    if (keyVal == GDK_KEY_F11) {
        if (property_fullscreened().get_value()) {
            unfullscreen();
            maximize();
            show_cursor();
        } else {
            fullscreen();
            hide_cursor();
        }
        return true;
    }
    if (keyVal == GDK_KEY_F12) {
        if (cursorHidden) {
            show_cursor();
        } else {
            hide_cursor();
        }
        return true;
    }
    return false;
}

void MainWindow::on_full_screen_changed() {
    if (property_fullscreened().get_value()) {
        fullscreenBtn.set_icon_name("window-shrink-symbolic");
    } else {
        fullscreenBtn.set_icon_name("window-grow-symbolic");
    }
}

void MainWindow::on_page_changed() {
    lightning.set_is_being_displayed(stack->get_visible_child_name() == "lightning");
}
}  // namespace ui::windows
