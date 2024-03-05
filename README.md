# Webserver-monitoring-system

## How to make Spiffs work

1. Configure in Platform.ini new Option: 
```
board_build.partitions = partitions.csv
```
2. Open Menu Config and go to **Partition Table**, change it to custom partition
3. To build the Spiffs partition binary from certain folder (here data) add following to the CmakeList.txt in your main folder:
```
spiffs_create_partition_image(spiffs data)
```
4. Use **Build Filesystem Image** and **Upload Filesystem Image** to upload

## How to make CallMeBot API work

1. Open menuconfig go to **Component config**
2. Open **ESP-TLS** and check **Allow potentially insecure options** and **Skip server certificate verification by default** 