# 0.2.4-beta - 2023-05-13

* Fix bugs in examples of `live_cells::select()` in documentation.

# 0.2.3-beta - 2023-05-09

* Fix bug in `live_cells::none()` example in documentation.

# 0.2.2-beta - 2023-04-28

* Fix link errors when building unit tests.

# 0.2.1-beta - 2023-04-21

* Fix potential bug caused by adding an observer to a cell while it is
  notifying its observers.

# 0.2.0-beta - 2023-04-18

New features:

* The values of mutable cells can now be set directly with the
  assignment operator:
  
  ```cpp
  auto a = live_cells::variable(0);
  
  std::cout << a.value() << std::endl; // 0
  
  a = 10;
  std::cout << a.value() << std::end; // 10
  ```
  
  The values can also be changed using the `++`, `--`, `+=`, `-=`,
  etc. operators.
  
* Reactive cell pipelines with the `|` operator:

  The namespace `live_cells::ops` provides functions that can be
  applied on cells using the `|` operator:
  
  The following:
  
  ```cpp
  auto cell = live_cells::peek(
      live_cells::on_error(
          live_cells::select(cond, a, b),
          c
      )
  )
  ```
  
  Can now be written as the following:
  
  ```cpp
  auto cell = cond
      | live_cells::ops::select(a, b)
      | live_cells::ops::on_error(c)
      | live_cells::ops::peek;
  ```
  
Bug fixes:

* Fixed bug: `MutableCell` concept not being satisfied by any mutable
  cell.

# 0.1.4-beta - 2024-04-10

* Add function call operator to `cell` and `typed_cell` containers.
  
  This allows runtime dependency tracking with dynamically typed
  cells:
  
  ```cpp
  auto a1 = live_cells::variable(0);
  auto a2 = live_cells::typed_cell<int>(a1);
  
  auto f = live_cells::computed([=] {
      return a2() + 1
  });
  ```

# 0.1.3-beta - 2024-04-08

* Switch to Apache 2.0 license

# 0.1.2-beta - 2024-04-08

* Change license to LGPL

# 0.1.1-beta - 2024-04-07

* Improve documentation

# 0.1.0-beta

Initial Release. The following features are ported for Live Cells
Dart:

* Mutable Cells
* Computed Cells
* Mutable Computed Cells
* Batch Updates
* Watch Functions
* Peek Cells
