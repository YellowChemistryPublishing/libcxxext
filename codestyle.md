# Style Guide

## Eliminating Error Prone Practice

CMake settings have configured compilation under gcc to use `-Wall -Wextra -Wpedantic -Werror`. To suppress false positives, see the useful macros in the `CompilerWarnings.h`
header.

You may not `throw` anything for the purpose of error checking. An exception is granted for fatal program conditions, such as that of a contract violation. In the case that a throw
expression is required, you are forbidden from using the `throw` keyword directly, please see `_throw(expr)`. Correctly written production code is banned from executing a
`_throw(expr)` expression, except for, once again, fatal error handling.

You _should_ not `catch(...)` anything, with the possible exception of hardware/system related exceptions, for example `std::bad_alloc`, for producing debugging information.

Due to partial requirements to support exceptions, your code must expect that control flow may return to the caller at any time. Hence, you must make sure that your code _never_
abandons unmanaged resources, i.e. (wrong) `spinLock.lock(); ... /* _throw(expr) */ ... spinLock.unlock();`. The natural corollary of this is that all synchronisation mechanisms
(i.e. `sys::SpinLock`) must never have `.lock()` or `.unlock()` invoked directly-- you should use some lock guard instead.

No C-style casts! Shortened macros for all casts are provided for convenience, please use those! (i.e. `_as(T, expr)` equiv. `static_cast<T>(expr)`.) Alternatively, for many
builtin types, prefer invoking their constructors explicitly. (i.e. `int(2.0f)`.)

Please don't let constructors silently fail! Instead, include the static member function `static Result<T> ctor(...)`, and include in the body of the analogous constructor
`_assert_ctor_can_fail()`.

## Informed Annotations

Documenting functions using the doxygen syntax in comments is highly recommended for production code. (Non-Enum) Types must be additionally annotated with
``/// @note Pass `byval`.``, ``/// @note Pass `byref`.``, or ``/// @note Pass `byptr`.`` indicating whether a type is best passed as `T`, `(const) T&`, or `(const) T (const)*`.
Types that act as static classes must be annotated with `/// @note Static class.`. The only mandatory field for production code is an `@attention Lifetime assumptions!` clause on
functions, with a `cpp` block highlighting the lifetime requirements of non-trivial parameters.

Any error checking passed up to a caller must be administered with short-circuiting guards. `_(co)retif(val, cond)` will return `val` iff. `cond` evaluates to `true`.
`_res_mov(co)ret(out, res_xval)` will use in assignment the `Result<...>` object from `res_xval`, returning its error via `Result<...>::err()` in the error case, or otherwise,
moving its result value into `out` via `Result<...>::move()`. When inside a coroutine, `co_await result` is semantically identical to `_res_mov_co_return(out, res_xval)`.

Runtime domain/input validation may be achieved through contracts, with `_contract_assert(cond)`.

For C++ initialization:

- Use list-initialization with a braced-init-list (`T t { v1, v2, ... }`) for aggregate initialization or to initialize a container with a list of values.
    - Don't forget to prefer designated initializers where applicable! (`T t { .m = ..., ... }`)
- Use direct initialization (`T t(...)`) where construction implies a specific workload or side-effect, and you want to call a specific constructor.
- (Most frequently) Use copy-initialization (`T t = ...`) for simple initialization from a single value.
- Use list-initialization with braces (`T t {...}`) in any other case, where none of the above would compile (i.e. member variables).

Parameters of type array-as-pointer must be passed by the array notation `... func(T arr[])`, rather than `... func(T* arr)`.

Where applicable and valid, `_restrict` must apply to pointers.

If you are accessing members, you _must_ use `this->...`!

Overriden virtual member functions must be annotated with the `override` specifier.

All functions, where correct to, must be annotated with `_pure_const` (`[[gnu::const]]`), or, where otherwise correct to, `_pure` (`[[gnu::pure]]`). Don't forget `[[nodiscard]]`
too!

Please use the following specifier order: `static`|`thread_local`|`extern`&emsp; `constexpr`|`consteval`|`constinit`|`inline`&emsp; `explicit`&emsp;`virtual`&emsp;`cv-pre`&emsp;
`T`|`auto`&emsp;`decl...`&emsp;`cvref-post`&emsp; `noexcept`&emsp;`override`|`final`

Where template parameters have restricted domain or constraints, they must be specified with a `requires` clause.

When using features from the C standard library, you must include the equivalent `<cheader>` and qualify relevant identifiers with `std::`. The only exception is for fixed width
integer types and standard integer aliases, i.e. `uint_least32_t`, `size_t`.

## What Integer to Use

- `ixx` or `uxx` in most cases.
- `ssz` when referring to sizes and lengths.
- `(u)int_leastxx_t`, `size_t`, `ptrdiff_t` if you specifically need the underlying integer type at least as wide.
- `int` if you legitimately don't need to care.

## Naming

### `C++`

- For macros, `_leading_underscore_snake_case`.
- For concepts and template parameters, `PascalCase`.
- For functions, member variables, global variables, enum types, and types, `snake_case`.
- For _everything_ else, `camelCase`.

### Files and Directories

- Scripts, test files, and directories are named in `snake_case`.
- Prefer `.h` / `.cpp` / (occasionally) `.inl` for C++ files.
- C++ headers and corresponding implementations are named in `PascalCase`.
- The entrypoint to a C++ program is enclosed in `main.cpp`.

## Formatting

The standard formatter expected for `C/C++` is `clang-format`.

The standard formatter expected for configuration and markup languages is `prettier-vscode`.

When deciding what casing to use:

- For macros, `_leading_underscore_snake_case`.
- For concepts and template parameters, `PascalCase`.
- For functions, member variables, global variables, enum types, and types, `snake_case`.
- For _everything_ else, `camelCase`.

Single statement bodies of control flow (i.e. after an `if`, `else`, `for`, etc.) must omit curly braces.

In addition, boolean checks must be as concise as possible, i.e. prefer `if (res)` over `if (_as(bool, res) == true)`.

## Library Checklist

If you check all these boxes, your code is probably sufficiently well thought out.

- Functions and variables marked with `noexcept`, `const`, `_pure`, `_pure_const`, `_restrict`, `[[nodiscard]]` where applicable.
- `constexpr` anything that moves.
- You are using `result<...>` for error handling.
- An exception thrown at any point in my code would not cause a memory or resource leak (or, it is sufficiently justified that one must not occur).
- For custom types, a function named `swap` is implemented in the same scope, and used in the type's move constructor.
    - (Please do the `friend constexpr void swap(T& a, T& b) { using std::swap; ... }` trick please.)
- When applicable, a type has a member function with declaration `sz hash_code()` is provided.

## Testing

When naming a test, describe in a _gramatically correct_ sentence what the test does, and the specific function, functions, or type that it tests. For example,
`` Byte manipulation works as expected. | `sys::s16fb2(...)`, `sys::hbfs16(...)`, `sys::lbfs16(...)` ``.

Use sections where appropriate. Name them as only a short description.
