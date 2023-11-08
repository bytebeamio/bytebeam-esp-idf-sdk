@echo off

cd build
python %IDF_PATH%/components/esptool_py/esptool/esptool.py -p %1 write_flash @flash_project_args
cd ..
python %IDF_PATH%/tools/idf_monitor.py -p %1 build/hello_cmake.elf