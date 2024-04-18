# Live Cells C++

Live Cells is a reactive programming library for C++ ported from [Live
Cells](https://livecells.viditrack.com) for Dart.

## Features

### Declarative

* A declarative reactive programming library for C++
* Define `cells` for your data
* Express complex logic as functions of cells

```cpp
// Define cells
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

// Define a function of a cell
auto sum = live_cells::computed([=] {
    return a() + b();
});
```

### Reactive

* Define observers on your cells
* Observers called automatically when cell values change
* Observers are self-subscribing
* React to any event or data change with minimal boilerplate

```cpp
auto watcher = live_cells::watch([=] {
    if (sum() > 100) {
        std::cout << "Sum exceeded 100!!\\n";
    }
});

a = 20;
b = 90; // Prints: Sum exceeded 100!!!
```

### Simple, powerful and maintainable

* You only need to learn one basic building block &mdash; the cell
* No need to worry about adding and removing event listeners or callbacks
* No need to implement complicated interfaces or subclasses
* Powerful enough to express any application logic
* Express your application logic without it buried in layers of event
  handling and state synchronization code
  
```cpp
auto a = live_cells::variable(...);
auto b = live_cells::computed(...);
auto w = live_cells::watch(...);

// That's all you have to learn
```

---

# Getting Started

\attention This software is still in the beta phase and may experience
rapid and drastic changes between releases.

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
list(APPEND CMAKE_MODULE_PATH "<prefix>/share/live_cells/cmake")
find_package(live_cells REQUIRED)
```

\note Substitute `<prefix>` with the install prefix given to the `configure`
script.


Add `${LIVE_CELLS_LIBRARY}` to your compilation target with the
following:

```cmake
target_link_libraries(<target> PRIVATE ${LIVE_CELLS_LIBRARY})
```

\note Substitute `<target>` with your compilation target

## Using Live Cells

Now that your project is set up, all you need to do is include the
`live_cells.hpp` header file:

```cpp
#include <live_cells/live_cells.hpp>
...
```

You're all set to start following the introductory
[tutorial](1-cells.md).
