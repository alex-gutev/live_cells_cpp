# 1. Cells

A cell is an object with a value and a set of observers that react to
changes in its value, you'll see exactly what that means in a moment.

There are a number of ways to create cells. The simplest cell is the
constant cell, created with `live_cells::value()`, which holds a
constant value.

```cpp
auto a = live_cells::value(1);
auto b = live_cells::value<std::string>("hello world");
auto c = live_cells::value(some_value);
```

`live_cells::value()` takes the constant value and wraps it in a
constant cell. The values of constant cells never change.

The value of a cell is accessed using the [`value()`](@ref live_cells::constant_cell::value) accessor method.

```cpp
std::cout << a.value() << std::endl; // Prints: 1
std::cout << b.value() << std::endl; // Prints: 'hello world'
std::cout << c.value() << std::endl; // Prints the value of `someValue`
```

## Mutable Cells

Mutable cells, created with `live_cells::variable()`, hold a value
that can be set with the `value()` setter method, which takes the
new value.

```cpp
auto a = live_cells::variable(0);

std::cout << a.value() << std::endl; // Prints: 0

// Set the value of a to 3
a.value(3);
std::cout << a.value() << std::endl; // Prints: 3
```

`live_cells::variable()` takes the initial value of the cell.

## Observing Cells

When the value of a cell changes, its observers are notified of the
change. The simplest way to demonstrate this is to set up a *watch
function* using [`live_cells::watch()`](@ref live_cells::watcher::watch()):

```cpp
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

// Set up a watch function observing cells `a` and `b`
auto watcher = live_cells::watch([=] {
    std::cout << a();
    std::cout << ", ";
    std::cout << b();
});

a.value(5);  // Prints: 5, 1
b.value(10); // Prints: 5, 10
```

[`live_cells::watch()`](@ref live_cells::watcher::watch()) takes a
watch function and registers it to be called when the values of the
cells referenced within it change. In the example above, a watch
function that prints the values of cells `a` and `b` to the console,
is defined. This function is called automatically when the value of
either `a` or `b` changes.

There are a couple of important points to keep in mind when using
[`live_cells::watch()`](@ref live_cells::watcher::watch()):

* The watch function is called once immediately when
  [`live_cells::watch()`](@ref live_cells::watcher::watch()) is
  called, to determine what cells are referenced by it.
  
* [`live_cells::watch()`](@ref live_cells::watcher::watch())
  automatically tracks which cells are referenced within it, and
  registers it to be called when their values change. This works even
  when the cells are referenced conditionally.
  
* Within the watch function, the values of cells have to be referenced
  with the function call operator, rather than the `value()`
  method. The difference between the two is that `value()` only
  references the value, whereas the function call operator also tracks
  it as a referenced cell.

\attention Within a watch function, the value of a cell is referenced
using the function call syntax rather than the `value()` method.

Every call to [`live_cells::watch()`](@ref live_cells::watcher::watch()) adds a new watch function, for
example:

```cpp
auto watcher2 = live_cells::watch([=] {
	std::cout << "A = " << a() << std::endl;
});

// Prints: 20, 10
// Also prints: A = 20
a.value(20);

// Prints: 20, 1
b.value(1);
```

The watch function defined above, `watcher2`, observes the value of
`a` only. Changing the value of `a` results in both watch functions
being called. Changing the value of `b` only results in the first
watch function being called, since the second watch function does not
reference `b` and hence is not observing it.

\remark When you no longer need the watch function to be called, call [`stop()`](@ref live_cells::watcher::stop())
on the "handle" returned by [`live_cells::watch()`](@ref live_cells::watcher::watch())\n
```cpp
watcher1->stop();
```

[`live_cells::watch()`](@ref live_cells::watcher::watch()) returns a
shared pointer (`std::shared_ptr`) holding a handle
(`live_cells::watcher`) to the watch function. This handle provides
the [`stop()`](@ref live_cells::watcher::stop()) method, which stops the watch function from being called
for further changes in the cell values.

```cpp
// Prints: 2, 1
b.value(2);

watcher1->stop();

// Doesn't print anything
b.value(3);
```

The watch function is also stopped automatically when the last
`shared_ptr` point to the handle, is destroyed.

\warning Due to the watch function being stopped automatically on
destruction, you should always assign the handle returned by
[`live_cells::watch()`](@ref live_cells::watcher::watch()) to a
variable, even if you don't intend on using it directly. If you don't
assign it to a variable or store it in a class member, the handle will
be destroyed immediately and the watch will not be called when the
cell values change.\n\n
**Don't do this**:\n
```cpp
// BAD because the watch handle is destroyed immediately,
// due to it not being stored anywhere,
// and thus the watch function is also stopped immediately
live_cells::watch([=] {
	...
});
```


## Computed Cells

A *computed cell* is a cell with a value that is defined as a function
of the values of one or more argument cells. Whenever the value of an
argument cell changes, the value of the computed cell is recomputed.

Compute cells are defined using `live_cells::computed()` which takes the
value computation function of the cell:

```cpp
auto a = live_cells::variable(1);
auto b = live_cells::variable(2)
auto sum = live_cells::computed([=] {
    return a() + b()
});
```

In the above example, `sum` is a computed cell with the value defined
as the sum of cells `a` and `b`. The value of `sum` is recomputed
whenever the values of either `a` or `b` change. This is demonstrated
below:

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << "The sum is ";
    std::cout << sum() << std::endl;
});

a.value(3); // Prints: The sum is 5
b.value(4); // Prints: The sum is 7
```

In this example:

1. A watch function observing the `sum` cell is defined.
2. The value of `a` is set to `3`, which:
   1. Causes the value of `sum` to be recomputed
   2. Calls the watch function defined in 1.
3. The value of `b` is set to `4`, which likewise also results in the
   sum being recomputed and the watch function being called.

## Batch Updates

The values of multiple cells can be set simultaneously in a *batch
update*. The effect of this is that while the values of the cells are
changed as soon as the `value()` setter method is called, the
observers of the cells are only notified after all the cell values
have been set.

Batch updates are performed with `live_cells::batch()`, which takes a
function that is called to set the values of one or more cells:

```cpp
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

auto watcher = live_cells::watch([=] {
    std::cout << a();
    std::cout << ", ";
    std::cout << b();
});

// This only prints: a = 15, b = 3
live_cells::batch([&] {
    a.value(15);
    b.value(3);
});
```

In the example above, the values of `a` and `b` are set to `15` and
`3` respectively, within `live_cells::batch()`. The watch function,
which observes both `a` and `b`, is only called once after the values
of both `a` and `b` are set within `live_cells::batch()`.

As a result the following is printed to the console:

```
a = 0, b = 1
a = 15, b = 3
```

1. `a = 0, b = 1` is printed when the watch function is first defined.
2. `a = 15, b = 3` is printed when the function provided to
   [`live_cells::watch()`](@ref live_cells::watcher::watch()) returns.


\note A watch function is always called once immediately after it is
set up. This is necessary to determine, which cells the watch function
is observing.

Alternatively a batch update can be performed by creating a
`live_cells::batch_update` in a given scope. The batching comes into
effect when the `batch_update` is created and the cell values are
updated when the `batch_update` is destroyed (on leaving the scope).

The following is equivalent to the previous example using
`live_cells::batch()`:

```cpp
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

auto watcher = live_cells::watch([=] {
    std::cout << a();
    std::cout << ", ";
    std::cout << b();
});

{
    live_cells::batch_update b;
    
    a.value(15);
    b.value(3);
} // Prints: a = 15, b = 3
```

## Cell Types

In the examples till this point we've been using `auto` to declare the
variables holding our cells. This is because the actual type varies
depending on the type of cell (constant cell, mutable cell, computed
cell, etc.) and also on the parameters used to create the cell. For
example, the type of a computed cell depends on the value computation
function.

All cell types satisfy the `live_cells::Cell` concept, which specifies
the cell protocol. We've already used two methods specified by the
`Cell` concept, the `value()` getter method and the function call
operator overload.

\note Mutable cells satisfy the `live_cells::MutableCell` concept, which
specifies the mutable cell protocol. `MutableCell` is a superset of
`Cell`, which means that every type that satisfies `MutableCell` also
satisfies `Cell`.

To define a function that takes a cell as an argument, define a
function `template` with the `Cell` concept constrained on the
template parameter type. For example here's a simple function `add`,
which takes two cells and returns a computed cell that computes the
sum of the two cells:

```cpp
template <live_cells::Cell A, live_cells::Cell B>
auto add(const A &a, const B &b) {
    return live_cells::computed([=] {
        return a() + b();
    });
}
```

This ensures that only an object that implements the cell protocol can
be provided for `a` and `b`.

The definition of the `add` function can be simplified using C++20's template
shorthand syntax:

```cpp
auto add (const Cell auto &a, const Cell auto &b) {
    return live_cells::computed([=] {
        return a() + b();
    });
}
```

### Dynamically Typed Cells

The `Cell` concept and `auto` limits you to cells for which the exact
type is known at compile-time. This means you cannot use them to store
cells of unrelated, and unknown, types in a container such as
`std::vector`.

For this use case, the `live_cells::cell` wrapper is provided. `cell`
is a wrapper over a `Cell` that performs type erasure, much like
`std::function`, in which a cell of any type can be stored.

A `cell` wrapper is created by providing a `Cell` to its
constructor. The wrapper exposes the same methods specified by the
`Cell` concept however the [`value()`](@ref live_cells::cell::value())
getter method is a template that has to be invoked with a type
parameter, specifying the type of value to retrieve:

```cpp
// A vector that can hold cells of any type
std::vector<live_cells::cell> my_cells;

// A statically typed mutable cell
auto a = live_cells::variable(0);

// A dynamically typed cell
live_cells::cell ref_a(a);

// `ref_a` can be added to a vector whereas `a` cannot
my_cells.push_back(ref_a);

// Printing the value of `a` via the `cell` wrapper:

// Prints: 0
std::cout << ref_a.value<int>() << std::endl;
```

\attention The template type parameter provided to `value` has to match the type
of the value held in the cell exactly, otherwise an `std::bad_cast`
exception is thrown.


When the value type of the value held in the cell is known ahead of
time, the `live_cells::typed_cell` wrapper can be used which is the
same as `live_cells::cell` but takes the value type as a template
parameter:

```cpp
// A vector that can hold cells of any type that hold an `int`
std::vector<live_cells::typed_cell<int>> my_cells;

// A statically typed mutable cell
auto a = live_cells::variable(0);

// A dynamically typed cell
live_cells::typed_cell<int> ref_a(a);

// `ref_a` can be added to a vector whereas `a` cannot
my_cells.push_back(ref_a);

// Prints: 0
std::cout << ref_a.value() << std::endl;
```

Notice there is no need to provided a value type template parameter to
the `value()` method, because the value type (`int` in this case) is
given in the `typed_cell` template parameter.

### Memory Management

Cells mostly take care of their own memory management, but there are a
few points to keep in mind when using cells:

1. A cell holds a reference to its state. This means, copying a cell
   does not copy the underlying state but merely creates a new
   reference to it:
   
   ```cpp
   auto a = live_cells::variable(0);
   auto b = a;
   
   a.value(10);
   
   // Prints: 10
   std::cout << b.value() << std::endl;
   ```
   
   That's why, as you've probably noticed, the lambda functions
   provided to [`live_cells::watch()`](@ref live_cells::watcher::watch()) and `live_cells::computed()` capture
   cells by value, not by reference.
   
   The cell state holds the cell's value and its observers.
   
2. If a lambda function can escape its scope, capture cells by value
   not by reference. 
   
   Both [`live_cells::watch()`](@ref live_cells::watcher::watch()) and
   `live_cells::computed()` store a copy of the lambda which can
   potentially outlive the scope in which the lambda function is
   defined. If a cell is captured by reference in this case, it ends up
   as a dangling reference.
   
   On the other hand, `live_cells::batch()` neither copies nor stores
   the function provided to it but only calls it immediately. Thus it
   is safe to use capture by reference in this case. However, when in
   doubt capture by value.
   
3. The state of a cell is destroyed when the last cell referencing it
   is destroyed.
   
   ```cpp
   auto a = live_cells::variable(0);
   live_cells::cell ref(a);
   
   { // Create a new scope
       auto b = live_cells::variable(1);
       auto c = live_cells::variable(2);
   
       // Set `ref` to `b`
       ref = live_cells::cell(b);
   } // Th state of `c` is destroyed at this point
   
   // The state of `b` is not destroyed because `ref`
   // is still holding a reference to it
   ```

    In this regard cells function much like `std::shared_ptr`.

4. Do not wrap a cell in `std::shared_ptr`. Whilst not wrong, its
   unnecessary because cells already have a shared pointer to their
   underlying state. Therefore, it's best to simply copy the cell,
   whether statically or dynamically typed.
   
5. Mutable cell values are assigned by value, not by reference. This
   probably goes without saying but its best to state it explicitly
   
   ```cpp
   auto a = live_cells::variable(std::string("hello"));
   
   // The string "bye" is copied into the `std::string` held by
   // the cell.
   a.value("bye");
   ```

## Next

Now that we've covered the basic you can proceed to the next
[section](2-expressions.md), which introduces utilities for
creating cells directly from expressions.
