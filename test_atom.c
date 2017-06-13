#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>

#include "uris.h"

#define TEST_ATOM_URI "http://lv2plug.in/plugins/test_atom"

enum {
  TEST_ATOM_CONTROL = 0,
  TEST_ATOM_NOTIFY = 1,
  TEST_ATOM_OUT_L = 2,
  TEST_ATOM_OUT_R = 3
};

typedef struct {
  const LV2_Atom_Sequence* control_port;
  LV2_URID_Map* map;
  uris uris;
  LV2_Atom_Forge forge;
  LV2_Atom_Forge_Frame seq_frame;
  LV2_Atom_Sequence* notify_port;
} test_plugin;

static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double sample_rate, const char* bundle_path,
    const LV2_Feature* const* features) {
  test_plugin* plugin = malloc(sizeof(test_plugin));
  
  // Scan host features for URID map
  LV2_URID_Map* map = NULL;

  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map))
      map = (LV2_URID_Map*) features[i]->data;
  }
  if (!map) {
    fprintf(stderr, "Host does not support urid:map.\n");
    free(plugin);
    return NULL;
  }

  // Map URIS
  plugin->map = map;
  map_uris(plugin->map, &plugin->uris);
  lv2_atom_forge_init(&plugin->forge, plugin->map);

  return plugin;
}

static void activate(LV2_Handle instance) {
  fprintf(stderr, "plugin activated!\n");
}

static void deactivate(LV2_Handle instance) {
  fprintf(stderr, "plugin deactivated!\n");
}

static void cleanup(LV2_Handle instance) {
  test_plugin* plugin = (test_plugin*) instance;
  free(plugin);
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
  test_plugin* plugin = (test_plugin*) instance;

  switch (port) {
    case TEST_ATOM_CONTROL:
      plugin->control_port = (const LV2_Atom_Sequence*) data;
      break;
    case TEST_ATOM_NOTIFY:
      plugin->notify_port = (LV2_Atom_Sequence*) data;
      break;
    default:
      break;
  }
}

static void run(LV2_Handle instance, uint32_t n_samples) {
  test_plugin* plugin = (test_plugin*) instance;

  // Set up forge to write directly to notify output port.
  const uint32_t notify_capacity = plugin->notify_port->atom.size;
  lv2_atom_forge_set_buffer(&plugin->forge, (uint8_t*) plugin->notify_port,
    notify_capacity);

  //LV2_Atom_Forge_Frame seq_frame;
  lv2_atom_forge_sequence_head(&plugin->forge, &plugin->seq_frame, 0);

  LV2_ATOM_SEQUENCE_FOREACH(plugin->control_port, ev) {
    if (ev->body.type == plugin->uris.atom_Object) {
      const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
      if (obj->body.otype == plugin->uris.ta_getZones) {
        fprintf(stderr, "PLUGIN: get zones received!!\n");
        int i;
        for (i = 0; i < 10; ++i) {
          lv2_atom_forge_frame_time(&plugin->forge, 0);
          LV2_Atom_Forge_Frame obj_frame;
          LV2_Atom* obj = (LV2_Atom*)lv2_atom_forge_object(&plugin->forge, &obj_frame, 0, plugin->uris.ta_addZone);
          lv2_atom_forge_key(&plugin->forge, plugin->uris.ta_params);
          char buf[432];
          sprintf(buf, "zone %i", i);
          lv2_atom_forge_string(&plugin->forge, buf, 432);
          lv2_atom_forge_pop(&plugin->forge, &obj_frame);

          fprintf(stderr, "PLUGIN: sent addZone %i; obj size: %i\n", i, lv2_atom_total_size(obj));
        }
      }
    }
  }
}

static const LV2_Descriptor descriptor = {
  TEST_ATOM_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  NULL // extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  switch (index) {
    case 0:
      return &descriptor;
    default:
      return NULL;
  }
}
