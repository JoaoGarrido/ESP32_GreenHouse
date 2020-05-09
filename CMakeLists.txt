cmake_minimum_required(VERSION 3.5.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ESP32_GreenHouseControl)
#target_add_binary_data(ESP32_GreenHouseControl.elf "main/mqtt_eclipse_org.pem" TEXT)