# Contributing to bytebeam-esp-sdk

If you have loved using bytebeam-esp-sdk and want to give back, we would love to have you open GitHub issues and PRs for features, bugs and documentation improvements.

Before you start, please make yourself familiar with the architecture of bytebeam-esp-sdk and read the [design docs][design] before making your first contribution to increase it's chances of being adopted. Please follow the [Code of Conduct][coc] when communicating with other members of the community and keep discussions civil, we are excited to have you make your first of many contributiions to this repository, welcome!

## Steps to Contribute

### Getting the project

Go to <https://github.com/bytebeamio/bytebeam-esp-idf-sdk> and fork the project repository.

```bash
# Open the terminal where you want to do the project setup
$ cd path_to_project_setup

# Remove the bytebeam-esp-idf-sdk project if any
$ rmdir /s /q bytebeam-esp-idf-sdk

# Clone your fork
$ git clone git@github.com:<YOU>/bytebeam-esp-idf-sdk.git

# Step into project directory
$ cd bytebeam-esp-idf-sdk

# Create a branch for your changes (say my_topical_branch)
$ git checkout -b my_topical_branch
```

### Project Setup

Before stepping into project setup, launch the esp-idf framework. If you have not installed the esp-idf framework yet then the best place to get started will be the  [Espressif Getting Started Guide][esp-get-started]

```bash
# Make sure idf.py tool is accessible
$ idf.py --version

# Make sure the target is available in the current version of esp-idf
$ idf.py --list-targets

# Step into project directory if not there
$ cd path_to_project_dir

# Step into any example project (say toggle_led)
$ cd examples/toggle_led

# Set the target (say esp32)
$ idf.py set-target esp32

# Configure your project
$ idf.py menuconfig

# Build, Flash and Monitor
$ idf.py -p PORT flash monitor
```

For more information on build system check out [Espressif Build System Guide][esp-build-system]

### Making Changes

Please make sure your changes conform to [Espressif IoT Development Framework Style Guide][esp-code-style]

### Testing & CI

Tests Suite is not yet added to the repo, will add it once we are done with the development cycle of the first release. By the way you can contribute this too :)

## Add yourself to Contributors

Thank you for contributing to bytebeam-esp-sdk, Please feel free to add yourself to [Contributors][contributors]

## License

bytebeam-esp-sdk is licensed under the permissive [Apache License Version 2.0][license] and we accept contributions under the implied notion that they are made in complete renunciation of the contributors any rights or claims to the same after the code has been merged into the codebase.

[license]: LICENSE
[design]: docs/design.md
[coc]: CODE_OF_CONDUCT.md
[esp-get-started]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/get-started/index.html
[esp-build-system]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/api-guides/build-system.html
[esp-code-style]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/contribute/style-guide.html
[contributors]: AUTHORS.md#contributors