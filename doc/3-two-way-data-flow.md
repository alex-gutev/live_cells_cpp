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
str_a = '100';
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
str_a = '15';
```

The value of this is that data can also flow from `a` to `str_a`:

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << str_a.value() << std::endl;
});

// Prints: 7
a = 7;
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

    a = half;
    b = half;
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
sum = 6;
```

The value of `a` and `b` can still be set in which case, `sum`
computes the sum of the values that were set:

```cpp
// Prints: 4 + 3 = 7
a = 4;

// Prints: 4 + 10 = 14
b = 10;
```

## Error Handling

One thing missing from the string to integer conversion cell, defined
earlier, is error handling. Ordinarily, exceptions thrown inside the
value computation function of a cell are rethrown when the cell's
value is referenced. However, exceptions thrown in the reverse
computation function of a mutable computed cell are not forwarded back
to the argument cells.

For example when the string to integer conversion cell `str_a`,
defined earlier, is assigned a string that does not represent a valid
integer, the assignment fails silently:

```cpp
str_a = "5";
std::cout << a << std::endl; // Prints: 5

str_a = "not a number";
std::cout << a << std::endl; // Prints: 5
```

### Maybe Values

`live_cells::maybe` is a container that either contains a value, or an
exception that was thrown during the computation of a value. This can
be used to catch exceptions thrown inside the reverse computation
function of a mutable computed cell.

`live_cells::maybe_wrap` calls the function passed to it and wraps the
return value, or the exception that was thrown, in a
[`maybe`](@ref live_cells::maybe):

```cpp
std::string str = "1";

auto maybe_int = live_cells::maybe_wrap([&] {
    return std::stoi("1");
});
```

The [`unwrap`](@ref live_cells::maybe::unwrap) method returns the
value held in the [`maybe`](@ref live_cells::maybe). If the
[`maybe`](@ref live_cells::maybe) holds an exception, it is rethrown.

```cpp
auto maybe_int = live_cells::maybe_wrap([&] {
    return std::stoi("1");
});

std::cout << maybe_int.unwrap() << std::endl; // Prints: 1

auto maybe_error = live_cells::maybe_wrap([&] {
    return std::stoi("not a number");
});

// Throws an 'std::invalid_argument' exception
std::cout << maybe_error.unwrap() << std::endl;
```

### Maybe Cells

`live_cells::maybe_cell` creates a cell that wraps the value of its
argument cell in a [`maybe`](@ref live_cells::maybe), as if by
`live_cells::maybe_wrap`.

The following definition:

```cpp
auto maybe_a = live_cells::maybe_cell(a)
```

is equivalent to:

```cpp
auto maybe_a = live_cells::computed([=] {
    return live_cells::maybe_wrap(a());
});
```

If the cell given to `maybe_cell` is mutable, `maybe_cell` creates a
mutable computed cell, which when assigned a maybe, sets the value of
its argument cell to the value held in the maybe.

\note The constructor of [`maybe`](@ref live_cells::maybe) creates a
[`maybe`](@ref live_cells::maybe) that holds the value given to it.

```cpp
auto a = live_cells::variable(0);
auto maybe_a = live_cells::maybe_cell(a);

auto w = live_cells::watch([=] {
    std::cout << "a = " << a() << std::endl;
});

// Prints 5
maybe_a = live_cells::maybe(5);
```

When a [`maybe`](@ref live_cells::maybe) holding an exception is
assigned to the *maybe cell*, the value of the argument cell is not
changed.

```cpp
maybe_a = live_cells::maybe_wrap([] {
    throw some_exception();
});

// Prints 5
std::cout << a.value() << std::endl;

// Throws 'some_exception'
std::cout << maybe_a.value().unwrap() << std::endl;
```

Exceptions thrown in the reverse computation function of a mutable
computed cell can be handled by wrapping the exception in a
[`maybe`](@ref live_cells::maybe) and assigning it to the argument
cell:

```cpp
auto a = live_cells::variable(0);
auto maybe_a = live_cells::maybe_cell(a);

auto str_a = live_cells::mutable_computed([=] {
    return std::to_string(maybe_a().unwrap());
}, [=] (auto str) {
    maybe_a = live_cells::maybe_wrap([&] {
        return std::stoi(str);
    });
});
```

The following creates a cell `is_valid` that is true if `str_a` was
set to a string containing a valid integer or false otherwise.

\note [`error()`](@ref live_cells::maybe::error) returns an
`std::exception_ptr` pointing to the exception that was thrown. If the
maybe does not hold an exception, the null exception pointer is
returned.

```cpp
auto is_valid = live_cells::computed([=] {
    if (maybe_a().error()) {
        return false;
    }
    
    return true;
});
```

To put this all together consider the following watch function:

```cpp
auto w1 = live_cells::watch([=] {
    if (is_valid()) {
        std::cout << "a = " << a() << std::endl;
    }
    else {
        std::cout << "'" << str_a() < "' is not a valid integer\n";
    }
});
```

When the value of `a` is set, its value is printed to standard output.

```cpp
a = 1;
a = 2;
a = 3;
```

The code above results in the following being printed:

```
a = 1
a = 2
a = 3
```

When the value of `str_a` is set, either the parsed integer value is
printed or an error message is printed if an integer could not be
parsed from the string assigned to `str_a`.

```cpp
str_a = '4';
str_a = 'NaN';
a = 5;
```

The code above results in the following being printed:

```
a = 4
'NaN' is not a valid integer
a = 5
```
