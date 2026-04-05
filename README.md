## Setup

Requires third-party dependencies (like glfw) and they can be installed using vcpkg. Vcpkg can be installed by running these commands in windows powershell:

- ```git clone https://github.com/microsoft/vcpkg``` 
- ```cd C:\dev\vcpkg```
- ```.\bootstrap-vcpkg.bat``` 
- ```.\vcpkg install glfw3:x64-windows``` 

In the root folder of the project, setup the config using the following generic CMake command. Note that the command uses the default vcpkg path, it may differ on your machine.

- ```cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake```

If this doesnt run properly, try the specified command below:

- ```cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake```

## Requirements

- CMake

## Build and Run

VS Code Debug shortcut (usually F5) in root folder to build + run

Alternatively, ```cmake --build build --config Debug``` and run the exe in build/Debug

## License

This project is licensed under the MIT License.

## Third-Party Libraries

This project uses:

- Dear ImGui (MIT License)
- GLFW (zlib/libpng License)

See the `/third-party_licenses` folder for full license texts.