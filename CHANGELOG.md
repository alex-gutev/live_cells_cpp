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
