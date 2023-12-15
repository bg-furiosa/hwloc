# hwloc-print

This program takes list of pci devices bdf as arguments and print pci device informations and find common ancestors.

## Prerequisite

```Bash
sudo apt install hwloc libhwloc-dev
```

## Build and execution
```Bash
gcc hwloc.c -o hwloc -lhwloc

./hwloc {SPACE_SEPERATED_DEVICES_BDF}

#./hwloc 0000:4f:00.0 0000:52:00.0 0000:56:00.0 0000:57:00.0 0000:ce:00.0 0000:d1:00.0 0000:d5:00.0 0000:d6:00.0
```
