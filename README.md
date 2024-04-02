# Live Cells CPP

> [!CAUTION]
> This software is still in beta, and will likely experience rapid
> changes before the first official release.


This is the C++ port of the [Live
Cells](https://livecells.viditrack.com/) reactive programming library.

## Examples

> [!NOTE]
> This section contains examples demonstrating the main features of
> the library. Head to the
> [documentation](https://alex-gutev.github.io/live_cells_cpp/docs/intro),
> for a short tutorial.

The basic building bloc of Live Cells is the cell, which is an object
with a value and a set of observers, which react to changes in the
value.

Cells are defined as follows:

```c++
auto a = live_cells::variable(0);
auto b = live_cells::variable(0);
```

And are observed as follows:

```c++
auto watcher = live_cells::watch([=] {
	std::cout << a() << ", " << b() << std::endl;
});
```

The *watch function* defined above prints the values of the cells `a`
and `b` to standard output. It is called whenever the value of `a` or
`b` changes.

For example the following code, which sets the values of `a` and `b`:

```c++
// Set a to 1
a.value(1);

// Set b to 2
b.value(2);

// Set a to 3
a.value(3);
```

Results in the following being printed to standard output:

```
1, 0
1, 2
3, 2
```

Cells can also be defined as a function of the values of one or more
cells. For example the following cell is defined as the sum of cells
`a` and `b`:

```c++
auto sum = live_cells::computed([=] {
	return a() + b();
});
```

The value of the `sum` cell is recomputed automatically whenever the
value of either `a` or `b` changes.

This cell can also be defined more succinctly as an expression of cells:

```c++
auto sum = a + b;
```

That looks exactly like a normal variable definition.

This can be used for example to print run a certain function
automatically whenever a certain condition is met. For example let's
print a message to standard output whenever the sum exceeds `100`:

```c++
auto watcher = live_cells::watch([=] {
	if (sum() > 100) {
		std::cout << "Sum exceeds 100!!!\n";
	}
});
```

When the following is executed:

```c++
a.value(90);
b.value(30);
```

The following message is printed to standard output automatically:

```c++
Sum exceeds 100!!!
```

## Quickstart

### Prerequisites

This library requires the following:

* A compiler that supports C++20.

The following dependencies are not required to use the library, but
are required for running the unit tests.

* [Boost Unit Test Framework](https://www.boost.org/) version 1.82

### Installation

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

### Configuring your project

The Live Cells library consists of headers, installed in
`<prefix>/include/live_cells`, and a static library `liblive_cells.a`
installed in `<prefix>/lib`.

:::note

`<prefix>` is `/usr/local`, unless specified otherwise with the
`--prefix` argument to the `configure` script.

:::

To use live cells in your C++ project, you'll need to do the following:

1. Add `<prefix>/include/live_cells` to your include path.
2. Link `<prefix>/lib/liblive_cells.a` into your compiled binary

#### Autotools

Add the following to your `configure.ac`:

```sh title="configure.ac"
AC_LIB_HAVE_LINKFLAGS(live_cells)
```

This searches for the `liblive_cells.a` in the default library search
paths (or the paths specified by the user), adds the necessary linker
options to the variable `LIBLIVE_CELLS` and adds directory where the
header files were installed, to the project's include path.

Add the following to your `Makefile.am`

```makefile title="Makefile.am"
target_LDADD = $(LIBLIVE_CELLS)
```

where `target` is the name of your compilation target.

#### CMake

Add the following to your `CMakeLists.txt` file:

```cmake title="CMakeLists.txt
list(APPEND CMAKE_MODULE_PATH "<prefix>/share/live_cells/cmake)
find_package(live_cells REQUIRED)
```

:::note

Substitute `<prefix>` with the install prefix given to the `configure`
script.

:::

### Using Live Cells

Now that your project is set up, all you need to do is include the
`live_cells.hpp` header file:

```c++
#include <live_cells/live_cells.hpp>
...
```

You're all set to start following the introductory
[tutorial](https://alex-gutev.github.io/live_cells_cpp/docs/category/basics),
