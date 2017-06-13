#include <stdlib.h>
#include <stdio.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include "lv2_external_ui.h"
#include "uris.h"

#define TEST_ATOM_UI_URI "http://lv2plug.in/plugins/test_atom#ui"

typedef struct {
  LV2_External_UI_Widget widget;
  LV2_URID_Map* map;
  uris uris;
  LV2UI_Write_Function write;
  LV2UI_Controller controller;
  LV2_Atom_Forge forge;
} test_atom_ui;

static void run(LV2_External_UI_Widget* widget) {
}

static void show(LV2_External_UI_Widget* widget) {
}

static void hide(LV2_External_UI_Widget* widget) {
}

static LV2UI_Handle instantiate(const LV2UI_Descriptor* descriptor,
    const char* plugin_uri, const char* bundle_path,
    LV2UI_Write_Function write_function, LV2UI_Controller controller,
    LV2UI_Widget* widget, const LV2_Feature* const* features) {

  test_atom_ui* ui = malloc(sizeof(test_atom_ui));

  // Scan host features for URID map
  LV2_URID_Map* map = NULL;

  int i;
  for (i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map))
      map = (LV2_URID_Map*)features[i]->data;
  }

  if (!map) {
    fprintf(stderr, "Host does not support urid:map.\n");
    free(ui);
    return NULL;
  }

  // Map URIS
  ui->map = map;
  map_uris(ui->map, &ui->uris);

  ui->write = write_function;
  ui->controller = controller;
  ui->widget.run = run;
  ui->widget.show = show;
  ui->widget.hide = hide;
  *widget = ui;

  lv2_atom_forge_init(&ui->forge, ui->map);

  uint8_t buf[128];
  lv2_atom_forge_set_buffer(&ui->forge, buf, 128);
  LV2_Atom_Forge_Frame obj_frame;
  LV2_Atom* obj = (LV2_Atom*) lv2_atom_forge_object(&ui->forge, &obj_frame, 0, ui->uris.ta_getZones);
  lv2_atom_forge_pop(&ui->forge, &obj_frame);

  ui->write(ui->controller, 0, lv2_atom_total_size(obj), ui->uris.atom_eventTransfer, obj);

  fprintf(stderr, "UI: ui instantiate called\n");

  return ui;
}

static void cleanup(LV2UI_Handle handle) {
  test_atom_ui* ui = (test_atom_ui*) handle;

  fprintf(stderr, "UI: ui closed\n");
  free(ui);
}

static void port_event(LV2UI_Handle handle, uint32_t port_index,
    uint32_t buffer_size, uint32_t format, const void* buffer) {

  test_atom_ui* ui = (test_atom_ui*) handle;
  const LV2_Atom* atom = (const LV2_Atom*) buffer;
  if (format == ui->uris.atom_eventTransfer && (atom->type == ui->uris.atom_Blank || atom->type == ui->uris.atom_Object)) {
    const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;
    if (obj->body.otype == ui->uris.ta_addZone) {
      LV2_Atom* params = NULL;

      lv2_atom_object_get(obj, ui->uris.ta_params, &params, 0);
      fprintf(stderr, "UI: received add zone!! %s\n", (char*)(params + 1));
    }
  }
}

static const LV2UI_Descriptor descriptor = {
  TEST_ATOM_UI_URI,
  instantiate,
  cleanup,
  port_event,
  NULL // extension_data
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
