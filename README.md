# rxdataToJSON

Converts the rxdata format used by RPG Maker XP to JSON

Also extracts the Scripts from Scripts.rxdata

## How to Compile/Install

First, dependencies.  
You'll need a C++ Compiler(`MSVC` for Windows or `GCC`/`Clang` for Unix) & [`CMake`](https://cmake.org). I won't be teaching you how to install these, so have fun!

Next, you'll need the actual source code on your computer.
You can obtain this either by downloading the ZIP file on Github or using `Git`.

Next, open a terminal instance in the root folder of the source, aka the one with this `README` file.
In here, you'll want to run the following commands
```sh
cmake -DCMAKE_BUILD_TYPE=Release .
cmake --build .
```

And that's it! The program should now be in the newly generated folder 'rxdataToJSON'.

## Usage

This is a terminal application, meaning it is meant to run in the terminal. Don't forget to add `.exe` if you're on Windows.
```sh
./rxdataToJSON [pathToFile]
```

The scripts from `Scripts.rxdata` should generate in the working directory of your terminal instance, so most likely the same folder as the executable if you used the exact command above.  
The other `.rxdata` JSONs should be next to the `.rxdata` file itself named as `[script].rxdata.json`
