// NOLINTBEGIN(misc-include-cleaner)

#include <module/sys>

#include <concepts>
#include <functional>
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

int free_function(int, float) { return 0; }
void void_function() { }

// ================================================================================
// Signed integer types. | `sys::IBuiltinIntegerSigned`
// ================================================================================

// Accepts signed integer types.
static_assert(sys::IBuiltinIntegerSigned<signed char>);
static_assert(sys::IBuiltinIntegerSigned<signed short>);
static_assert(sys::IBuiltinIntegerSigned<signed int>);
static_assert(sys::IBuiltinIntegerSigned<signed long>);
static_assert(sys::IBuiltinIntegerSigned<signed long long>);

// Rejects unsigned integer types.
static_assert(!sys::IBuiltinIntegerSigned<unsigned char>);
static_assert(!sys::IBuiltinIntegerSigned<unsigned short>);
static_assert(!sys::IBuiltinIntegerSigned<unsigned int>);
static_assert(!sys::IBuiltinIntegerSigned<unsigned long>);
static_assert(!sys::IBuiltinIntegerSigned<unsigned long long>);

// Rejects non-integer types.
static_assert(!sys::IBuiltinIntegerSigned<float>);
static_assert(!sys::IBuiltinIntegerSigned<double>);
static_assert(!sys::IBuiltinIntegerSigned<char>);
static_assert(!sys::IBuiltinIntegerSigned<bool>);
static_assert(!sys::IBuiltinIntegerSigned<void>);
static_assert(!sys::IBuiltinIntegerSigned<std::string>);

// ================================================================================
// Unsigned integer types. | `sys::IBuiltinIntegerUnsigned`
// ================================================================================

// Accepts unsigned integer types.
static_assert(sys::IBuiltinIntegerUnsigned<unsigned char>);
static_assert(sys::IBuiltinIntegerUnsigned<unsigned short>);
static_assert(sys::IBuiltinIntegerUnsigned<unsigned int>);
static_assert(sys::IBuiltinIntegerUnsigned<unsigned long>);
static_assert(sys::IBuiltinIntegerUnsigned<unsigned long long>);

// Rejects signed integer types.
static_assert(!sys::IBuiltinIntegerUnsigned<signed char>);
static_assert(!sys::IBuiltinIntegerUnsigned<signed short>);
static_assert(!sys::IBuiltinIntegerUnsigned<signed int>);
static_assert(!sys::IBuiltinIntegerUnsigned<signed long>);
static_assert(!sys::IBuiltinIntegerUnsigned<signed long long>);

// Rejects non-integer types.
static_assert(!sys::IBuiltinIntegerUnsigned<float>);
static_assert(!sys::IBuiltinIntegerUnsigned<double>);
static_assert(!sys::IBuiltinIntegerUnsigned<bool>);
static_assert(!sys::IBuiltinIntegerUnsigned<std::string>);

// ================================================================================
// All built-in integer types. | `sys::IBuiltinInteger`
// ================================================================================

// Accepts all built-in integer types.
static_assert(sys::IBuiltinInteger<signed char>);
static_assert(sys::IBuiltinInteger<unsigned char>);
static_assert(sys::IBuiltinInteger<signed int>);
static_assert(sys::IBuiltinInteger<unsigned int>);
static_assert(sys::IBuiltinInteger<signed long long>);
static_assert(sys::IBuiltinInteger<unsigned long long>);

// Rejects floating-point types.
static_assert(!sys::IBuiltinInteger<float>);
static_assert(!sys::IBuiltinInteger<double>);
static_assert(!sys::IBuiltinInteger<long double>);

// Rejects other types.
static_assert(!sys::IBuiltinInteger<bool>);
static_assert(!sys::IBuiltinInteger<char>);
static_assert(!sys::IBuiltinInteger<std::string>);

// ================================================================================
// Floating-point types. | `sys::IBuiltinFloatingPoint`
// ================================================================================

// Accepts floating-point types.
static_assert(sys::IBuiltinFloatingPoint<float>);
static_assert(sys::IBuiltinFloatingPoint<double>);
static_assert(sys::IBuiltinFloatingPoint<long double>);

// Rejects integer types.
static_assert(!sys::IBuiltinFloatingPoint<int>);
static_assert(!sys::IBuiltinFloatingPoint<unsigned int>);
static_assert(!sys::IBuiltinFloatingPoint<long long>);

// Rejects other types.
static_assert(!sys::IBuiltinFloatingPoint<bool>);
static_assert(!sys::IBuiltinFloatingPoint<char>);
static_assert(!sys::IBuiltinFloatingPoint<std::string>);

// ================================================================================
// All numeric types. | `sys::IBuiltinNumeric`
// ================================================================================

// Accepts all numeric types.
static_assert(sys::IBuiltinNumeric<signed int>);
static_assert(sys::IBuiltinNumeric<unsigned long>);
static_assert(sys::IBuiltinNumeric<float>);
static_assert(sys::IBuiltinNumeric<double>);
static_assert(sys::IBuiltinNumeric<long double>);

// Rejects non-numeric types.
static_assert(!sys::IBuiltinNumeric<bool>);
static_assert(!sys::IBuiltinNumeric<char>);
static_assert(!sys::IBuiltinNumeric<std::string>);
static_assert(!sys::IBuiltinNumeric<void*>);

// ================================================================================
// Integer range containment. | `sys::IBuiltinIntegerCanHold`
// ================================================================================

// Larger signed can hold smaller signed.
static_assert(sys::IBuiltinIntegerCanHold<signed long long, signed char>);
static_assert(sys::IBuiltinIntegerCanHold<signed long long, signed short>);
static_assert(sys::IBuiltinIntegerCanHold<signed long long, signed int>);

// Larger unsigned can hold smaller unsigned.
static_assert(sys::IBuiltinIntegerCanHold<unsigned long long, unsigned char>);
static_assert(sys::IBuiltinIntegerCanHold<unsigned long long, unsigned short>);
static_assert(sys::IBuiltinIntegerCanHold<unsigned long long, unsigned int>);

// Same type can hold itself.
static_assert(sys::IBuiltinIntegerCanHold<signed int, signed int>);
static_assert(sys::IBuiltinIntegerCanHold<unsigned int, unsigned int>);

// Smaller cannot hold larger.
static_assert(!sys::IBuiltinIntegerCanHold<signed char, signed int>);
static_assert(!sys::IBuiltinIntegerCanHold<unsigned char, unsigned int>);

// Unsigned cannot hold signed due to negative range.
static_assert(!sys::IBuiltinIntegerCanHold<unsigned int, signed int>);
static_assert(!sys::IBuiltinIntegerCanHold<unsigned long long, signed char>);

// Signed cannot hold unsigned due to upper range.
static_assert(!sys::IBuiltinIntegerCanHold<signed int, unsigned int>);

// ================================================================================
// Character types. | `sys::ICharacter`
// ================================================================================

// Accepts character types.
static_assert(sys::ICharacter<char>);
static_assert(sys::ICharacter<wchar_t>);
static_assert(sys::ICharacter<char8_t>);
static_assert(sys::ICharacter<char16_t>);
static_assert(sys::ICharacter<char32_t>);

// Rejects non-character types.
static_assert(!sys::ICharacter<int>);
static_assert(!sys::ICharacter<signed char>);
static_assert(!sys::ICharacter<unsigned char>);
static_assert(!sys::ICharacter<std::string>);
static_assert(!sys::ICharacter<bool>);

// ================================================================================
// Functor signature matching. | `sys::IFunc`
// ================================================================================

// Matches function pointers with correct signature.
static_assert(sys::IFunc<decltype(&free_function), int(int, float)>);
static_assert(sys::IFunc<decltype(&void_function), void()>);

// Matches `std::function` with correct signature.
static_assert(sys::IFunc<std::function<int(int, float)>, int(int, float)>);

// Rejects incompatible signatures.
static_assert(!sys::IFunc<decltype(&free_function), void()>);
static_assert(!sys::IFunc<decltype(&free_function), int(int)>);

// ================================================================================
// Iterable types. | `sys::IEnumerable`
// ================================================================================

// Accepts arrays.
static_assert(sys::IEnumerable<int[5]>);
static_assert(sys::IEnumerable<char[10]>);

// Accepts standard containers.
static_assert(sys::IEnumerable<std::vector<int>>);
static_assert(sys::IEnumerable<std::string>);

// Accepts containers with element type constraint.
static_assert(sys::IEnumerable<std::vector<int>, int>);
static_assert(sys::IEnumerable<std::string, char>);

// Rejects containers with wrong element type.
static_assert(!sys::IEnumerable<std::vector<int>, float>);
static_assert(!sys::IEnumerable<std::string, int>);

// Rejects non-iterable types.
static_assert(!sys::IEnumerable<int>);
static_assert(!sys::IEnumerable<float>);
static_assert(!sys::IEnumerable<void*>);

// ================================================================================
// Types with `size()`. | `sys::ISizeable`
// ================================================================================

// Accepts containers with `size()`.
static_assert(sys::ISizeable<std::vector<int>>);
static_assert(sys::ISizeable<std::string>);

// Accepts with specific size type.
static_assert(sys::ISizeable<std::vector<int>, std::size_t>);
static_assert(sys::ISizeable<std::string, std::size_t>);

// Rejects with wrong size type.
static_assert(!sys::ISizeable<std::vector<int>, int>);

// Rejects types without `size()`.
static_assert(!sys::ISizeable<int>);
static_assert(!sys::ISizeable<float>);

// NOLINTEND(misc-include-cleaner)
