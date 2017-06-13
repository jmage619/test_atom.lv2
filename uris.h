#ifndef URIS_H
#define URIS_H

#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#define TEST_ATOM__getZones "http://lv2plug.in/plugins/test_atom#getZones"
#define TEST_ATOM__addZone "http://lv2plug.in/plugins/test_atom#addZone"
#define TEST_ATOM__params "http://lv2plug.in/plugins/test_atom#params"

typedef struct {
  LV2_URID atom_eventTransfer;
  LV2_URID atom_Blank;
  LV2_URID atom_Object;
  LV2_URID ta_getZones;
  LV2_URID ta_addZone;
  LV2_URID ta_params;
} uris;

static inline void map_uris(LV2_URID_Map* map, uris* uris) {
  uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
  uris->atom_Blank = map->map(map->handle, LV2_ATOM__Blank);
  uris->atom_Object = map->map(map->handle, LV2_ATOM__Object);
  uris->ta_getZones = map->map(map->handle, TEST_ATOM__getZones);
  uris->ta_addZone = map->map(map->handle, TEST_ATOM__addZone);
  uris->ta_params = map->map(map->handle, TEST_ATOM__params);
}

#endif
