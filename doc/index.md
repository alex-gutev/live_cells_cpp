# Getting Started

## Prerequisites

This library requires the following:

* A compiler that supports C++20.

The following dependencies are not required to use the library, but
are required for running the unit tests.

* [Boost Unit Test Framework](https://www.boost.org/) version 1.82

## Installation

To begin using Live Cells:

1. Download the [latest release](https://github.com/alex-gutev/live_cells_cpp/releases/latest).

2. Unpack the tar archive to a directory of your choice.

3. Install the library by running the following commands in the
   directory where you unpacked the tar archive:

   ```sh
   mkdir build; cd build
   ../configure
   make install
   ```
   
	This will install Live Cells at `/usr/local`. You can choose an
    alternative prefix with:
   
   ```sh
   ../configure --prefix=/path/to/prefix
   ```

Live Cells should now be installed at `/usr/local` or the prefix you've
given to `configure`.

## Configuring your project

The Live Cells library consists of headers, installed in
`<prefix>/include/live_cells`, and a static library `liblive_cells.a`
installed in `<prefix>/lib`.


\note `<prefix>` is `/usr/local`, unless specified otherwise with the
`--prefix` argument to the `configure` script.

To use live cells in your C++ project, you'll need to do the following:

1. Add `<prefix>/include/live_cells` to your include path.
2. Link `<prefix>/lib/liblive_cells.a` into your compiled binary

### Autotools

Add the following to your `configure.ac`:

```sh
AC_LIB_HAVE_LINKFLAGS(live_cells)
```

This searches for the `liblive_cells.a` in the default library search
paths (or the paths specified by the user), adds the necessary linker
options to the variable `LIBLIVE_CELLS` and adds directory where the
header files were installed, to the project's include path.

Add the following to your `Makefile.am`

```makefile
target_LDADD = $(LIBLIVE_CELLS)
```

where `target` is the name of your compilation target.

### CMake

Add the following to your `CMakeLists.txt` file:

```cmake
list(APPEND CMAKE_MODULE_PATH "<prefix>/share/live_cells/cmake)
find_package(live_cells REQUIRED)
```

\note Substitute `<prefix>` with the install prefix given to the `configure`
script.

## Using Live Cells

Now that your project is set up, all you need to do is include the
`live_cells.hpp` header file:

```cpp
#include <live_cells/live_cells.hpp>
...
```

You're all set to start following the introductory
[tutorial](1-cells.md).

