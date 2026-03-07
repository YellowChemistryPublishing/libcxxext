#pragma once

/// @file

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>

#include <meta/Properties.h>

namespace sys::meta
{
    /// @brief Metadata for parameter pack `Pack...`.
    /// @note Static class.
    template <typename... Pack>
    struct parameter_pack final : meta_type
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

    /// @brief Metadata for a template type.
    /// @note Static class.
    template <typename T>
    struct template_type final : meta_type
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
    public:
        /// @brief Whether `T` is defined from `Template`.
        template <template <typename...> class Template>
        static consteval bool is_from()
        {
            return is_templated_from<T, Template>::value();
        }
    };

    /// @brief Obtain the type `T`, inherited with the `cv` qualifiers of a reference-stripped `With`.
    template <typename T, typename With>
    using replace_cv = std::conditional_t<
        std::is_const_v<std::remove_reference_t<With>>,
        std::add_const_t<std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>,
        std::conditional_t<std::is_volatile_v<std::remove_reference_t<With>>, std::add_volatile_t<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>>;
} // namespace sys::meta
