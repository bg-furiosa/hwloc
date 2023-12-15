#include <hwloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void print_hwloc_bridge_attributes(const struct hwloc_bridge_attr_s *bridge_attr) {
    printf("Upstream Bridge Type: %d\n", bridge_attr->upstream_type);

    if (bridge_attr->upstream_type == HWLOC_OBJ_BRIDGE_PCI) {
        printf("Upstream PCI Domain: %04x\n", bridge_attr->upstream.pci.domain);
        printf("Upstream PCI Bus: %02x\n", bridge_attr->upstream.pci.bus);
        printf("Upstream PCI Device: %02x\n", bridge_attr->upstream.pci.dev);
        printf("Upstream PCI Function: %01x\n", bridge_attr->upstream.pci.func);
    }

    printf("Downstream Bridge Type: %d\n", bridge_attr->downstream_type);
    if (bridge_attr->downstream_type == HWLOC_OBJ_BRIDGE_PCI) {
        printf("Downstream PCI Domain: %u\n", bridge_attr->downstream.pci.domain);
        printf("Downstream PCI Secondary Bus: %02x\n", bridge_attr->downstream.pci.secondary_bus);
        printf("Downstream PCI Subordinate Bus: %02x\n", bridge_attr->downstream.pci.subordinate_bus);
    }

    printf("Bridge Depth: %u\n", bridge_attr->depth);
}

void print_hwloc_pcidev_attributes(const struct hwloc_pcidev_attr_s *pcidev_attr) {
    printf("PCI Device Domain: %04x\n", pcidev_attr->domain);
    printf("PCI Device Bus: %02x\n", pcidev_attr->bus);
    printf("PCI Device Dev: %02x\n", pcidev_attr->dev);
    printf("PCI Device Func: %01x\n", pcidev_attr->func);
    printf("PCI Device Class: %04x\n", pcidev_attr->class_id);
    printf("PCI Device Vendor: %04x\n", pcidev_attr->vendor_id);
    printf("PCI Device Device: %04x\n", pcidev_attr->device_id);
    printf("PCI Device Subvendor: %04x\n", pcidev_attr->subvendor_id);
    printf("PCI Device Subdevice: %04x\n", pcidev_attr->subdevice_id);
    printf("PCI Device Revision: %02x\n", pcidev_attr->revision);
    printf("PCI Device Linkspeed: %f GT/s\n", pcidev_attr->linkspeed);
}

void print_hwloc_obj_attributes(hwloc_obj_t obj) {
    printf("Type: %s\n", hwloc_obj_type_string(obj->type));
    printf("Subtype: %s\n", obj->subtype);
    printf("Name: %s\n", obj->name);
    printf("Depth: %u\n", obj->depth);

    printf("Number of Info Attributes: %u\n", obj->infos_count);
    for (unsigned i = 0; i < obj->infos_count; ++i) {
        printf("Info %u: Key = %s, Value = %s\n", i, obj->infos[i].name, obj->infos[i].value);
    }

    switch (obj->type) {
        case HWLOC_OBJ_PCI_DEVICE: {
            print_hwloc_pcidev_attributes(&obj->attr->pcidev);
            break;
        }
        case HWLOC_OBJ_BRIDGE: {
            print_hwloc_bridge_attributes(&obj->attr->bridge);
            break;
        }
    }

    printf("Logical Index: %u\n", obj->logical_index);
    printf("Number of Children: %u\n", obj->arity);
}

hwloc_obj_t find_host_bridge(hwloc_obj_t pci_device) {
    hwloc_obj_t obj = pci_device;
    while (obj != NULL) {
        if (obj->type == HWLOC_OBJ_BRIDGE && obj->attr->bridge.upstream_type == HWLOC_OBJ_BRIDGE_HOST) {
            return obj;
        }
        obj = obj->parent;
    }
    return NULL;
}

int main (int argc, char *argv[])
{
  if (argc == 1 || argc > 9) {
    return EXIT_FAILURE;
  }

  hwloc_topology_t topology;
  int depth;
  hwloc_obj_t last;
  hwloc_obj_t *closest;
  unsigned found;
  int err;
  unsigned numprocs;
  hwloc_obj_t ancestor;
  hwloc_obj_t host_bridge;

  err = hwloc_topology_init (&topology);
  if (err)
    return EXIT_FAILURE;

  hwloc_topology_set_io_types_filter(topology, HWLOC_TYPE_FILTER_KEEP_IMPORTANT);

  err = hwloc_topology_load (topology);
  if (err)
    return EXIT_FAILURE;

  hwloc_obj_t device_obj_arr[10] = {0,};

  //hwloc_obj_t hwloc_get_pcidev_by_busid(hwloc_topology_t topology, unsigned domain, unsigned bus, unsigned dev, unsigned func)
  for (int i=1; i< argc; i++) {
    device_obj_arr[i] = hwloc_get_pcidev_by_busidstring(topology, argv[i]);
    assert(device_obj_arr[i]);
    printf("PCI device found at %s\n", argv[i]);
    print_hwloc_obj_attributes(device_obj_arr[i]);

    host_bridge = find_host_bridge(device_obj_arr[i]);
    assert(host_bridge);
    printf("\n\nHostBridge info\n");
    print_hwloc_obj_attributes(host_bridge);
    printf("\n\n ------------------- \n\n");
  }

  for (int i = 1; i < argc; i++) {
    for (int j = 1; j< argc; j++) {
      if (i == j) {
        continue;
      }

      // hwloc_obj_t hwloc_get_common_ancestor_obj (hwloc_topology_t topology __hwloc_attribute_unused, hwloc_obj_t obj1, hwloc_obj_t obj2)
      ancestor = hwloc_get_common_ancestor_obj(topology, device_obj_arr[i], device_obj_arr[j]);
      assert(ancestor);
      printf("ancestor found for %s and %s \n", argv[i], argv[j]);
      print_hwloc_obj_attributes(ancestor);
      printf("\n\n ------------------- \n\n");
    }
  }

  hwloc_topology_destroy (topology);

  return EXIT_SUCCESS;
}