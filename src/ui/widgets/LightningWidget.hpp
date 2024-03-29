#pragma once

#include "backend/lightning/Lightning.hpp"
#include <vector>
#include <gtkmm/drawingarea.h>
#include <gtkmm/image.h>
#include <gtkmm/overlay.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#include <shumate/shumate.h>
#pragma GCC diagnostic pop

namespace ui::widgets {
class LightningWidget : public Gtk::Overlay {
 private:
    backend::lightning::Lightning lightning;

    ShumateMarker* marker{nullptr};
    ShumateMarkerLayer* markerLayer{nullptr};
    Gtk::Image image;
    Gtk::DrawingArea drawingArea;

    bool newLightning{false};

 public:
    LightningWidget(const backend::lightning::Lightning& lightning, ShumateMarkerLayer* markerLayer);
    LightningWidget(LightningWidget&&) = default;
    LightningWidget(const LightningWidget&) = delete;
    LightningWidget& operator=(LightningWidget&&) = default;
    LightningWidget& operator=(const LightningWidget&) = delete;
    ~LightningWidget() override = default;

    void prep_widget();

    [[nodiscard]] const backend::lightning::Lightning& get_lightning() const;
    void remove();
    void update();

    //-----------------------------Events:-----------------------------
    void on_draw_handler(const Cairo::RefPtr<Cairo::Context>& ctx, int width, int height);
};
}  // namespace ui::widgets