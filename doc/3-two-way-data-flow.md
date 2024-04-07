# 3. Two-Way Data Flow

In all the examples we've seen till this point, data always flows in a
single direction. Now we'll introduce two-way data flow which allows
data to flow in both directions between a pair of cells.

## Mutable Computed Cells

A *mutable computed cell* is a cell which ordinarily functions like a
normal computed cell, created with `live_cells::computed()`, but can
also have its value set directly as though it is a mutable cell. When
the value of a mutable computed cell is set, it *reverses* the
computation by setting the argument cells to a value such that when
the mutable computed cell is recomputed, the same value will be
produced as the value that was set. This allows data to flow in two
directions, whereas `live_cells::computed()` only allows data to flow
in a single direction.

Mutable computed cells are created using
`live_cells::mutable_computed()`, which takes the computation function
and reverse computation function. The computation function computes
the cell's value as a function of argument cells, like
`live_cells::computed()`. The reverse computation function *reverses*
the computation by assigning a value to the argument cells. It is
given the value that was assigned to the cell.

Example:

```cpp
auto a = live_cells::variable(0);

auto str_a = live_cells::mutable_computed([=] {
    return std::to_string(a())
}, [=] (value) {
  a.value(std::stoi(value));
});
```

The mutable computed cell `str_a` converts the value of its argument
cell `a` to a string. When the value of `str_a` is set:

1. An `int` is parsed from the value.
2. The value of `a` is set to the parsed `int` value.

```cpp
str_a.value('100');
std::cout << (a.value() + 1) << std::endl; // Prints: 101
```

This definition is useful for obtaining string input from the user and
converting it to an integer. The string input, can be assigned
directly to the `str_a` cell, and the parsed integer can be observed
by observing `a`.

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << (a.value() + 1) << std::endl;
});

// Prints: 16
str_a.value(15);
```

The value of this is that data can also flow from `a` to `str_a`:

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << str_a.value() << std::endl;
});

// Prints: 7
a.value(7);
```

## Multiple Arguments

Mutable computed cells can be defined as a function of more than one
argument cell. For example we can define a `sum` cell, that ordinarily
computes the sum of two numbers held in cells `a` and `b`. When `sum`
is assigned a value, `a` and `b` are set to one half of the sum that
was assigned.

```cpp
auto a = live_cells::variable(0.0);
auto b = live_cells::variable(0.0);

auto sum = live_cells::mutable_computed([=] {
    return a() + b();
}, [=] (sum) {
    auto half = sum / 2;

    a.value(half);
    b.value(half);
});
```

The reverse computation function assigns the sum divided by two to
both cells `a` and `b`.

For example setting `sum` to `6` results in `a` and `b` both being set
to `3`:

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << a() << " + " << b();
    std::cout << " = " << sum() << std::endl;
});

// Prints: 3 + 3 = 6
sum.value(6);
```

The value of `a` and `b` can still be set in which case, `sum`
computes the sum of the values that were set:

```cpp
// Prints: 4 + 3 =7
a.value(4);

// Prints: 4 + 10 = 14
b.value(10);
```
