# 2. Cell Expressions

This library provides a number of tools for building expressions of
cells without requiring a computed cell to be created explicitly using
`live_cells::computed()`.

## Arithmetic

The arithmetic and relational (`<`, `<=`, `>`, `>=`) operators, when
applied to cells holding numeric values, return cells which compute the
result of the expression.

This allows a computation to be defined directly as an expression of
cells. For example the following cell computes the sum of two cells:

```cpp
auto a = live_cells::variable(1);
auto b = live_cells::variable(2);

auto sum = a + b;
```

\note This definition of the sum cell is not only simpler than the
definition using `live_cells::computed()` but is also more efficient
since the argument cells are determined at compile-time. In-fact it is
equivalent to the definition seen in \ref lightweight-computed-cells
"Lightweight Computed Cells":\n
```cpp
auto sum = live_cells::computed(a, b, [] (auto a, auto b) {
    return a + b;
});
```

The `sum` cell is a cell like any other. It can be observed by a watch
function or can appear as an argument in a computed cell.

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << sum() << std::endl;
});

a = 5; // Prints: 7
b = 4; // Prints: 9
```

Expressions of cells can be arbitrarily complex:

```cpp
auto x = a * b + c / d;
auto y = x < e;
```

\remark Values appearing in a cell expression, e.g. `a + 1`, are
converted to constant cells, as if by `live_cells::value()`.

## Equality

Every cell overloads the `==` and `!=` operators, which return cells
that compare whether the values of the cells are equal or not equal,
respectively.

```cpp
auto eq = a == b;  // eq() == true when a() == b()
auto neq = a != b; // neq() == true when a() != b()
```

## Logic and selection

The following operator overloads and functions are provided for cells
holding boolean-like values:

<dl>
<dt>`&&`</dt>
<dd>Creates a cell with a value that is the logical and of two cells</dd>
<dt>`||`</dt>
<dd>Creates a cell with a value that is the logical or of two cells</dd>
<dt>`!`</dt>
<dd>Creates a cell with a value which is the logical not of a cell</dd>
<dt>[`select`](@ref live_cells::select)</dt>
<dd>Creates a cell which selects between the values of two cells based on a condition</dd>
</dl>

\note The `&&` and `||` operator overloads preserve the
short-circuiting behaviour of the operators. This means the value of a
cell is not referenced if the result of the expression is already
known without it.

```cpp
auto a = live_cells::variable(false);
auto b = live_cells::variable(false);

auto c = live_cells::variable(1);
auto d = live_cells::variable(2);

// cond() is true when a() || b() is true
auto cond = a || b;

// when cond() is true, cell() == c() else cell() == d()
auto cell = live_cells::select(cond, c, d);

auto watcher = live_cells::watch([=] {
    std::cout << cell() << std::endl;
});

a = true;  // Prints: 1
a = false; // Prints: 2
```

The third argument (if false) of `live_cells::select()` can be omitted, in which case
the cell's value will not be updated if the condition is false:

```cpp
auto cond = live_cells::variable(false);
auto a = live_cells::variable(1);

auto cell = live_cells::select(cond, a);

auto watcher = live_cells::watch([=] {
    std::cout << cell() << std::endl;
});

cond = true;  // Prints: 1
a = 2;        // Prints: 2

cond = false; // Prints: 2
a = 4;        // Prints: 2
```

## Aborting a computation

The computation of a computed cell's value can be aborted using
`live_cells::none()`. When `live_cells::none()` is called inside a
computed cell, the value computation function is exited and the cell's
current value is preserved. This can be used to prevent a cell's value
from being recomputed when a condition is not met:

```cpp
auto a = live_cells::variable(4);
auto b = live_cells::computed([=] {
    if (a() >= 10)
        live_cells::none();
        
    return a();
});

auto watcher = live_cells::watch([=] {
    std::cout << b() << std::endl;
});

a = 6;  // Prints 6
a = 15; // Prints 6
a = 8;  // Prints 8
```

\note The initial value of every computed cell is the default
constructed value. Thus if `live_cells::none()` is called during the
first call to the cell's computation function, the default value for
the cell's value type is retained.

\attention The value of a computed cell is only computed if it is
actually referenced. `live_cells::none()` only preserves the current
value of the cell, but this might not be the latest value of the cell
if the cell's value is only referenced conditionally. A good rule of
thumb is to use `live_cells::none()` only to prevent a cell from
holding an invalid value.

\warning Do no use `live_cells::none()` inside a lightweight computed
cell, unless the cell is immediately wrapped with
`live_cells::store`. This is because `live_cells::none()` requires
caching of the cell's previous value and lightweight computed cells do
not cache their values.

## Exception handling

If an exception is thrown during the computation of a cell's value, it
is rethrown when the value is referenced. This allows exceptions to be
handled using `try` and `catch` inside computed cells:

```cpp
auto str = live_cells::variable(std::string("0"));

auto n = live_cells::computed([=] {
    return std::stoi(str());
});

auto is_valid = live_cells::computed([=] {
  try {
    return n() > 0;
  }
  catch (...) {
    return false;
  }
});

std::cout << is_valid.value() << std::endl; // Prints false

str = "5";
std::cout << is_valid.value() << std::endl; // Prints true

str = "not a number";
std::cout << is_valid.value() << std::endl; // Prints false
```

The `live_cells::on_error()` function creates a cell that selects the
value of another cell when an exception is thrown.

```cpp
auto str = live_cells::variable(std::string("0"));

auto m = live_cells::variable(2);
auto n = live_cells::computed([=] {
    return std::stoi(str());
});

// Equal to n(). If n() throws, equal to m();
auto result = live_cells::on_error(n, m);

str = "3";
std::cout << result.value() << std::endl; // Prints 3

str = "not a number";
std::cout << result.value() << std::endl; // Prints 2
```

A template overload of `on_error` is provided, which takes an
exception type as a template type parameter. When this overload is
used only exceptions of the given type are handled.

```cpp
// Only handles std::invalid_argument exceptions
auto result = live_cells::on_error<std::invalid_argument>(n, m);
```

The validation logic in the previous example can be implemented more
succinctly using:

```cpp
auto str = live_cells::variable("0");
auto n = live_cells::computed([=] {
    return std::stoi(str());
});

auto is_valid = on_error(
    n > 0,
    live_cells::value(false)
);
```

\note We used `live_cells::value(false)` to create a constant cell
that holds the value `false`.


## Previous values

The previous value of a cell can be accessed using `live_cells::previous`:

```cpp
auto a = live_cells::variable(1);
auto prev = live_cells::previous(a);

auto sum = a + prev;

auto watch = live_cells::watcher([=] {
    int prev_value = prev();
    
    std::cout << "\n";
    std::cout << "A = " << a() << std::endl;
    std::cout << "Prev = " << prev_value << std::endl;
    std::cout << "Sum = " << sum() << std::endl;
});

a = 2;
a = 5;
```

This results in the following being printed to standard output:

```text
A = 2
Prev = 1
Sum = 3

A = 5
Prev = 2
Sum = 7
```

`live_cells::previous` returns a cell that can be used like any other
cell. When `previous` is called multiple times on the same argument
cell, the same cell is returned.

\attention
* On creation `prev` does not hold a value. Accessing it will throw a
  `live_cells::uninitialized_cell_error` exception.
* `prev` must have at least one observer in order for it to keep track
  of the previous value of `a`.

## Peeking cells

What if you want to use the value of a cell in a computed cell but
don't want changes to that cell's value triggering a recomputation?
The `live_cells::peek()` function allows you to do exactly that.

```cpp
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

auto c = live_cells::computed([=] {
    return a() + live_cells::peek(b);
});

auto watch = live_cells::watch([=] {
    std::cout << c() << std::endl;
});

a = 3; // Prints: 4
b = 5; // Doesn't print anything
a = 7; // Prints: 13
```

In the above example cell `c` is a computed cell that references the
value of `a` and *peeks* the value of `b`. Changing the value of `a`
triggers a recomputation of `c`, and hence triggers the watch function
which prints to standard output. However, changing the value of `b`
doesn't trigger a recomputation of `c`, and hence the watch function
is not called.

\note `live_cells::peek()` returns a cell.

You may be asking why do we need `live_cells::peek()` here instead of
just accessing the value of `b` directly using `b.value()`. Something
we've glossed over till this point is the lifecycle of cells. Cells
are only active while they are actually observed, and are activated
when the first observer is added. While active, cells react to changes
in their argument cells. When the last observer is removed, cells are
deactivated and stop observing their argument cells. When a new
observer is added, they are reactivated again. Essentially, this means
that the value of a cell may no longer be current if it doesn't have
at least one observer. For a computed cell this is not a problem,
since when it is inactive it computes its value on demand, but it may
cause issues with other cells. The `live_cells::peek()` function takes
care of adding an observer to the peeked cell, so that it remains
active, but at the same time prevents the observers, added through
`live_cells::peek()`, from being notified of changes in its value.

## Pipe operator

The `live_cells::ops` namespace provides a collection of operators
which can be applied on cells using the `|` operator.

For example the [`select`](@ref live_cells::select) function,
introduced earlier, can also be applied on the condition cell with the
following:

```cpp
auto cell = cond | live_cells::ops::select(c, d);
```

which is equivalent to:

```cpp
auto cell = live_cells::select(cond, c, d);
```

The `live_cells::ops` package also provides a variant of
[`on_error`](@ref live_cells::on_error), 
[`peek`](@ref live_cells::peek) and [`previous`](@ref live_cells::previous) that can
be used with the pipe operator. This allows for a reactive pipeline to
be built without nested function calls.

```cpp
auto cell = cond 
    | live_cells::ops::select(a, b) 
    | live_cells::ops::on_error(c) 
    | live_cells::ops::previous
    | live_cells::ops::peek;
```

This is equivalent to:

```cpp
auto cell = live_cells::peek(
    live_cells::previous(
        live_cells::on_error(
            live_cells::select(cond, a, b),
            c
        )
    )
);
```

The `live_cells::ops::store` can be used to wrap a cell using
`live_cells::store` (introduced in \ref lightweight-computed-cells
"Lightweight Computed Cells"):

```cpp
auto cached_sum = sum | live_cells::ops::store;
```

This is equivalent to:

```cpp
auto cached_sum = live_cells::store(sum)
```

`live_cells::ops::cache` is like `live_cells::ops::store` however it
also passes the `live_cells::changes_only` argument:

```cpp
auto cached_sum = sum | live_cells::ops::cache;
```

This is equivalent to:

```cpp
auto cached_sum = live_cells::store(live_cells::changes_only, sum)
```

## Next

The next [section](3-two-way-data-flow.md) introduces two-way data
flow.
