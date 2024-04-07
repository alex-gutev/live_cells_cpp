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
since the argument cells are determined at compile-time.

The `sum` cell is a cell like any other. It can be observed by a watch
function or can appear as an argument in a computed cell.

```cpp
auto watcher = live_cells::watch([=] {
    std::cout << sum() << std::endl;
});

a.value(5); // Prints: 7
b.value(4); // Prints: 9
```

Expressions of cells can be arbitrarily complex:

```cpp
auto x = a * b + c / d;
auto y = x < e;
```

\remark To include a constant in a cell expression wrap it in a cell using
`live_cells::value()`.

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
holding boolean-like value:

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

\note The `&&` and `||` operators overloads preserve the
short-circuiting behaviour of the operators. This means the value of a
cell is not referenced if the result of the expression is already
known without it.

```cpp
// cond() is true when a() || b() is true
auto cond = a || b;

// when cond() is true, cell() == c() else cell() == d()
auto cell = live_cells::select(cond, c, d);

a.value(true);
c.value(1);
d.value(2);

std::cout << cell.value() << std::endl; // Prints: 1

a.value(false);
b.value(false);

std::cout << cell.value() << std::endl; // Prints: 2
```

The third argument (if false) of `live_cells::select()` can be omitted, in which case
the cell's value will not be updated if the condition is false:

```cpp
auto cell = live_cells::select(cond, c);

cond.value(true);
a.value(2);

std::cout << cell.value() << std::endl; // Prints 2

cond.value(false);
a.value(4);

std::cout << cell.value() << std::endl; // Prints 2
```

## Aborting a computation

In the previous section we saw that `live_cells::select()` creates a
cell which does not update its argument when the condition cell is
`false` and its only given one argument. Under the hood, `select`
doesn't create some special kind of cell but uses `live_cells::none()`
to abort the computation.

When `live_cells::none()` is called inside a computed cell, the
computation of the cell's value is aborted and its current value is
preserved. This can be used to prevent a cell's value from being
recomputed when a condition is not met:

```cpp
auto a = live_cells::variable(4);
auto b = live_cells::computed([=] {
    if (a() >= 10)
        live_cells::none();
        
    return a();
});

a.value(6);
std::cout << b.value() << std::endl; // Prints 6

a.value(15);
std::cout << b.value() << std::endl; // Prints 6

a.value(8);
std::cout << b.value() << std::endl; // Prints 8
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

## Exception Handling

If an exception is thrown during the computation of a cell's value it
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

str.value = "5";
std::cout << is_valid.value() << std::endl; // Prints true

str.value = "not a number";
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

str.value("3");
std::cout << result.value() << std::endl; // Prints 3

str.value("not a number");
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

auto isValid = on_error(
    n > live_cells::value(0),
    live_cells::value(false)
);
```

\note We used `live_cells::value(0)` and `live_cells::value(false)` to
create constant cells that hold the values `0` and `false`,
respectively.


## Peeking Cells

What if you want to use the value of a cell in a computed cell but
don't want changes to that cell's value triggering a recomputation?
The [`live_cells::peek()`](@ref live_cells::peek_cell::peek) function
allows you to do exactly that.

```cpp
auto a = live_cells::variable(0);
auto b = live_cells::variable(1);

auto c = live_cells::computed([=] {
    return a() + live_cells::peek(b);
});

auto watch = live_cells::watch([=] {
    std::cout << c() << std::endl;
});

a.value(3); // Prints: 4
b.value(5); // Doesn't print anything
a.value(7); // Prints: 13
```

In the above example cell `c` is a computed cell referencing the value
of `a` and *peeks* the value of `b`. Changing the value of `a`
triggers a recomputation of `c`, and hence triggers the watch function
which prints to standard output, but changing the value of `b` doesn't
trigger a recomputation of `c`.

\note [`live_cells::peek()`](@ref live_cells::peek_cell::peek) returns
a cell.

You may be asking why do we need
[live_cells::peek()](#live_cells::peek_cell::peek) here instead of
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
cause issues with other cells. The
[live_cells::peek()](#live_cells::peek_cell::peek) function takes care
of adding an observer to the peeked cell, so that it remains active,
but at the same time prevents the observers, added through
[live_cells::peek()](#live_cells::peek_cell::peek), from being
notified of changes in its value.

## Next

The next [section](3-two-way-data-flow.md) introduces two-way data
flow.