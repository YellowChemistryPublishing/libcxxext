#pragma once

/// @file

#include <LanguageSupport.h>
#include <meta/Container.h>
#include <meta/Type.h>
#include <traits/Empty.h>

namespace sys
{
    template <typename Iter>
    requires (!meta::type<Iter>::is_ref() && ICopyConstructible<Iter>)
    // clang-format off
    struct subrange final : meta::type_switch
    <
        meta::type_case
        <
            std::derived_from<Iter, traits::contiguous_iterator<Iter, std::remove_reference_t<decltype(*std::declval<Iter>())>>>,
            traits::contiguous_range<subrange<Iter>>
        >,
        meta::type_case
        <
            std::derived_from<Iter, traits::random_access_iterator<Iter, std::remove_reference_t<decltype(*std::declval<Iter>())>>>,
            traits::random_access_range<subrange<Iter>>
        >,
        meta::type_case
        <
            std::derived_from<Iter, traits::bidirectional_iterator<Iter, std::remove_reference_t<decltype(*std::declval<Iter>())>>>,
            traits::bidirectional_range<subrange<Iter>>
        >,
        meta::type_case
        <
            std::derived_from<Iter, traits::forward_iterator<Iter, std::remove_reference_t<decltype(*std::declval<Iter>())>>>,
            traits::forward_range<subrange<Iter>>
        >,
        meta::type_case<true, traits::empty>
    >
    // clang-format on
    {
    private:
        Iter _beg, _end;
    public:
        subrange(Iter beg, Iter end) : _beg(beg), _end(end) { }

        constexpr Iter begin() const noexcept { return this->_beg; }
        constexpr Iter end() const noexcept { return this->_end; }
    };
} // namespace sys
