# Contributing to esp-bytebeam-sdk

If you have loved using esp-bytebeam-sdk and want to give back, we would love to have you open GitHub issues and PRs for features, bugs and documentation improvements.

Before you start, please make yourself familiar with the architecture of esp-bytebeam-sdk and read the [design docs][design] before making your first contribution to increase it's chances of being adopted. Please follow the [Code of Conduct][coc] when communicating with other members of the community and keep discussions civil, we are excited to have you make your first of many contributiions to this repository, welcome!

## Steps to Contribute

### Getting the code

Go to <https://github.com/bytebeamio/esp-bytebeam-sdk> and fork the project repository.

```bash
# Clone your fork
$ git clone git@github.com:<YOU>/esp-bytebeam-sdk.git

# Enter the project directory
$ cd esp-bytebeam-sdk

# Create a branch for your changes
$ git checkout -b my_topical_branch
```

### Project Setup

Before stepping into project setup, launch the esp-idf framework. If you have not installed the esp-idf framework yet then the best place to get started will be the  [Espressif Getting Started Guide][esp-get-started]

```bash
# Make sure idf.py tool is accessible
$ idf.py --version

# Make sure the target is available in the current version of esp-idf
$ idf.py --list-targets

# Step into any example project (say toggle_led)
$ cd examples/toggle_led

# Set the target
$ idf.py set-target esp-board

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

Thank you for contributing to esp-bytebeam-sdk, Please feel free to add yourself to [Contributors][contributors]

## License

esp-bytebeam-sdk is licensed under the permissive [Apache License Version 2.0][license] and we accept contributions under the implied notion that they are made in complete renunciation of the contributors any rights or claims to the same after the code has been merged into the codebase.

[license]: LICENSE
[design]: docs/design.md
[coc]: docs/CoC.md
[esp-get-started]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/get-started/index.html
[esp-build-system]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/api-guides/build-system.html
[esp-code-style]: https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32/contribute/style-guide.html
[contributors]: AUTHORS.md#contributors