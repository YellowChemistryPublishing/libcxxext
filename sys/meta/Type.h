#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility> // NOLINT(misc-include-cleaner)

#include <meta/Properties.h>

/// @def _decltype_of(var)
/// @ingroup sys
/// @brief `std::remove_cvref_t<decltype(var)>`
#define _decltype_of(var) std::remove_cvref_t<decltype(var)>

/// @def _forward(var)
/// @ingroup sys
/// @brief `std::forward<decltype(var)>(var)`
#define _forward(var) std::forward<decltype(var)>(var)

namespace sys::meta
{
    /// @ingroup sys
    /// @brief Metadata for parameter pack `Pack...`.
    template <typename... Pack>
    struct /* [[sys::static]] */ parameter_pack final : meta_type
    {
        using tuple = std::tuple<Pack...>;

        template <size_t Index>
        using at = std::tuple_element_t<Index, tuple>;

        /// @brief Whether parameter pack contains `T`.
        template <typename T>
        static consteval bool contains()
        {
            return (std::same_as<T, Pack> || ...);
        }
    };

    /// @ingroup sys
    /// @brief Metadata for some type.
    template <typename T>
    struct /* [[sys::static]] */ type final : is_unqualified_prop<std::same_as<T, std::remove_cvref_t<T>>>,
                                              is_const_prop<std::is_const_v<T>>,
                                              is_volatile_prop<std::is_volatile_v<T>>,
                                              is_lvalue_prop<std::is_lvalue_reference_v<T>>,
                                              is_rvalue_prop<std::is_rvalue_reference_v<T>>,
                                              is_ref_prop<std::is_reference_v<T>>,
                                              is_array_prop<std::is_array_v<T>>
    {
    private:
        template <typename, template <typename...> class>
        struct is_templated_from final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename...> class Template, typename... Args>
        struct is_templated_from<Template<Args...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto...> class>
        struct is_templated_from_auto final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto...> class Template, auto... Args>
        struct is_templated_from_auto<Template<Args...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, typename, typename, auto...> class>
        struct is_templated_from_special_0001 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, typename, typename, auto...> class Template, typename T0, typename T1, typename T2, auto... Ts>
        struct is_templated_from_special_0001<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, typename, auto, typename...> class>
        struct is_templated_from_special_0010 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, typename, auto, typename...> class Template, typename T0, typename T1, auto T2, typename... Ts>
        struct is_templated_from_special_0010<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, typename, auto, auto...> class>
        struct is_templated_from_special_0011 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, typename, auto, auto...> class Template, typename T0, typename T1, auto T2, auto... Ts>
        struct is_templated_from_special_0011<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, auto, typename, typename...> class>
        struct is_templated_from_special_0100 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, auto, typename, typename...> class Template, typename T0, auto T1, typename T2, typename... Ts>
        struct is_templated_from_special_0100<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, auto, typename, auto...> class>
        struct is_templated_from_special_0101 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, auto, typename, auto...> class Template, typename T0, auto T1, typename T2, auto... Ts>
        struct is_templated_from_special_0101<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, auto, auto, typename...> class>
        struct is_templated_from_special_0110 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, auto, auto, typename...> class Template, typename T0, auto T1, auto T2, typename... Ts>
        struct is_templated_from_special_0110<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <typename, auto, auto, auto...> class>
        struct is_templated_from_special_0111 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <typename, auto, auto, auto...> class Template, typename T0, auto T1, auto T2, auto... Ts>
        struct is_templated_from_special_0111<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, typename, typename, typename...> class>
        struct is_templated_from_special_1000 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, typename, typename, typename...> class Template, auto T0, typename T1, typename T2, typename... Ts>
        struct is_templated_from_special_1000<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, typename, typename, auto...> class>
        struct is_templated_from_special_1001 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, typename, typename, auto...> class Template, auto T0, typename T1, typename T2, auto... Ts>
        struct is_templated_from_special_1001<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, typename, auto, typename...> class>
        struct is_templated_from_special_1010 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, typename, auto, typename...> class Template, auto T0, typename T1, auto T2, typename... Ts>
        struct is_templated_from_special_1010<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, typename, auto, auto...> class>
        struct is_templated_from_special_1011 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, typename, auto, auto...> class Template, auto T0, typename T1, auto T2, auto... Ts>
        struct is_templated_from_special_1011<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, auto, typename, typename...> class>
        struct is_templated_from_special_1100 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, auto, typename, typename...> class Template, auto T0, auto T1, typename T2, typename... Ts>
        struct is_templated_from_special_1100<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, auto, typename, auto...> class>
        struct is_templated_from_special_1101 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, auto, typename, auto...> class Template, auto T0, auto T1, typename T2, auto... Ts>
        struct is_templated_from_special_1101<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
        template <typename, template <auto, auto, auto, typename...> class>
        struct is_templated_from_special_1110 final : meta_type
        {
            static consteval bool value() { return false; }
        };
        template <template <auto, auto, auto, typename...> class Template, auto T0, auto T1, auto T2, typename... Ts>
        struct is_templated_from_special_1110<Template<T0, T1, T2, Ts...>, Template> final : meta_type
        {
            static consteval bool value() { return true; }
        };
    public:
        /// @brief Whether `T` is defined from `Template`.
        template <template <typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from<T, Template>::value();
        }
        template <template <auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_auto<T, Template>::value();
        }
        template <template <typename, typename, typename, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0001<T, Template>::value();
        }
        template <template <typename, typename, auto, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0010<T, Template>::value();
        }
        template <template <typename, typename, auto, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0011<T, Template>::value();
        }
        template <template <typename, auto, typename, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0100<T, Template>::value();
        }
        template <template <typename, auto, typename, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0101<T, Template>::value();
        }
        template <template <typename, auto, auto, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0110<T, Template>::value();
        }
        template <template <typename, auto, auto, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_0111<T, Template>::value();
        }
        template <template <auto, typename, typename, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1000<T, Template>::value();
        }
        template <template <auto, typename, typename, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1001<T, Template>::value();
        }
        template <template <auto, typename, auto, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1010<T, Template>::value();
        }
        template <template <auto, typename, auto, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1011<T, Template>::value();
        }
        template <template <auto, auto, typename, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1100<T, Template>::value();
        }
        template <template <auto, auto, typename, auto...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1101<T, Template>::value();
        }
        template <template <auto, auto, auto, typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from_special_1110<T, Template>::value();
        }
    };

    /// @ingroup sys
    /// @brief Obtain the type `T`, inherited with the `cv` qualifiers of a reference-stripped `With`.
    template <typename T, typename With>
    using replace_cv = std::conditional_t<
        std::is_const_v<std::remove_reference_t<With>>,
        std::add_const_t<std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>,
        std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>;
} // namespace sys::meta
