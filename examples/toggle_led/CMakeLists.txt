# The following four lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to wororrectly
cmake_minimum_required(VERSION 3.5)

# (Not part of the boilerplate)
# This example uses an extra component for common functions such as Wi-Fi and Ethernet connection.
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
                         ../../components
                         $ENV{IDF_PATH}/examples/get-started/blink/managed_components/espressif__led_strip)

include($ENV{IDF_PATH}/tools/cmake/project.cmake
                    main/Kconfig.projbuild)
project(bytebeam_toggle_led_example)