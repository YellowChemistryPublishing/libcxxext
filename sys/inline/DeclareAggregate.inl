#include <cstddef>
#include <utility>

#include <Integer.h>

#if !defined(_libcxxext_internal_aggregate_mark_primary_template) || !_libcxxext_internal_aggregate_mark_primary_template
#error "This file should not be included directly. `#include <module/sys>`, or `#include <Aggregate.h>` instead."
#endif

namespace sys
{
#if !defined(_libcxxext_internal_aggregate_mark_primary_template) || !_libcxxext_internal_aggregate_mark_primary_template
    // For your editing convenience, real primary template is in `<Aggregate.h>`.
    template <typename... Ts>
    struct aggregate final
    { };
#endif

    /// @overload
    template <
#ifdef _libcxxext_internal_tparam_count
#if _libcxxext_internal_tparam_count == 1
        typename T0
#elif _libcxxext_internal_tparam_count == 2
        typename T0, typename T1
#elif _libcxxext_internal_tparam_count == 3
        typename T0, typename T1, typename T2
#elif _libcxxext_internal_tparam_count == 4
        typename T0, typename T1, typename T2, typename T3
#elif _libcxxext_internal_tparam_count == 5
        typename T0, typename T1, typename T2, typename T3, typename T4
#elif _libcxxext_internal_tparam_count == 6
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5
#elif _libcxxext_internal_tparam_count == 7
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
#elif _libcxxext_internal_tparam_count == 8
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
#elif _libcxxext_internal_tparam_count == 9
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8
#elif _libcxxext_internal_tparam_count == 10
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9
#elif _libcxxext_internal_tparam_count == 11
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10
#elif _libcxxext_internal_tparam_count == 12
        typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11
#endif
#endif
        >
    struct aggregate<
#ifdef _libcxxext_internal_tparam_count
#if _libcxxext_internal_tparam_count == 1
        T0
#elif _libcxxext_internal_tparam_count == 2
        T0, T1
#elif _libcxxext_internal_tparam_count == 3
        T0, T1, T2
#elif _libcxxext_internal_tparam_count == 4
        T0, T1, T2, T3
#elif _libcxxext_internal_tparam_count == 5
        T0, T1, T2, T3, T4
#elif _libcxxext_internal_tparam_count == 6
        T0, T1, T2, T3, T4, T5
#elif _libcxxext_internal_tparam_count == 7
        T0, T1, T2, T3, T4, T5, T6
#elif _libcxxext_internal_tparam_count == 8
        T0, T1, T2, T3, T4, T5, T6, T7
#elif _libcxxext_internal_tparam_count == 9
        T0, T1, T2, T3, T4, T5, T6, T7, T8
#elif _libcxxext_internal_tparam_count == 10
        T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
#elif _libcxxext_internal_tparam_count == 11
        T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10
#elif _libcxxext_internal_tparam_count == 12
        T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11
#endif
#endif
        > final
    {
    public:
        template <size_t Idx>
        using type_at = meta::parameter_pack<
#ifdef _libcxxext_internal_tparam_count
#if _libcxxext_internal_tparam_count == 1
            T0
#elif _libcxxext_internal_tparam_count == 2
            T0, T1
#elif _libcxxext_internal_tparam_count == 3
            T0, T1, T2
#elif _libcxxext_internal_tparam_count == 4
            T0, T1, T2, T3
#elif _libcxxext_internal_tparam_count == 5
            T0, T1, T2, T3, T4
#elif _libcxxext_internal_tparam_count == 6
            T0, T1, T2, T3, T4, T5
#elif _libcxxext_internal_tparam_count == 7
            T0, T1, T2, T3, T4, T5, T6
#elif _libcxxext_internal_tparam_count == 8
            T0, T1, T2, T3, T4, T5, T6, T7
#elif _libcxxext_internal_tparam_count == 9
            T0, T1, T2, T3, T4, T5, T6, T7, T8
#elif _libcxxext_internal_tparam_count == 10
            T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
#elif _libcxxext_internal_tparam_count == 11
            T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10
#elif _libcxxext_internal_tparam_count == 12
            T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11
#endif
#endif
            >::template at<Idx>;

        // clang-format off
#ifdef _libcxxext_internal_tparam_count
#if _libcxxext_internal_tparam_count == 1
            T0 item1;
#elif _libcxxext_internal_tparam_count == 2
            T0 item1; T1 item2;
#elif _libcxxext_internal_tparam_count == 3
            T0 item1; T1 item2; T2 item3;
#elif _libcxxext_internal_tparam_count == 4
            T0 item1; T1 item2; T2 item3; T3 item4;
#elif _libcxxext_internal_tparam_count == 5
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5;
#elif _libcxxext_internal_tparam_count == 6
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6;
#elif _libcxxext_internal_tparam_count == 7
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7;
#elif _libcxxext_internal_tparam_count == 8
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7; T7 item8;
#elif _libcxxext_internal_tparam_count == 9
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7; T7 item8; T8 item9;
#elif _libcxxext_internal_tparam_count == 10
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7; T7 item8; T8 item9; T9 item10;
#elif _libcxxext_internal_tparam_count == 11
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7; T7 item8; T8 item9; T9 item10; T10 item11;
#elif _libcxxext_internal_tparam_count == 12
            T0 item1; T1 item2; T2 item3; T3 item4; T4 item5; T5 item6; T6 item7; T7 item8; T8 item9; T9 item10; T10 item11; T11 item12;
#endif
#endif
        // clang-format on

        friend constexpr bool operator==(const aggregate&, const aggregate&) = default;
        friend constexpr auto operator<=>(const aggregate&, const aggregate&) = default;

        static consteval sz size() noexcept { return sz(size_t(_libcxxext_internal_tparam_count)); }

        template <size_t Idx>
        consteval decltype(auto) operator()(this auto&& _this)
        {
            if constexpr (Idx >= size_t(_libcxxext_internal_tparam_count))
                throw std::domain_error("Index out of bounds.");
#ifdef _libcxxext_internal_tparam_count
#if _libcxxext_internal_tparam_count >= 12
            else if constexpr (Idx == 11uz)
                return std::forward_like<decltype(_this)>(_this.item12);
#endif
#if _libcxxext_internal_tparam_count >= 11
            else if constexpr (Idx == 10uz)
                return std::forward_like<decltype(_this)>(_this.item11);
#endif
#if _libcxxext_internal_tparam_count >= 10
            else if constexpr (Idx == 9uz)
                return std::forward_like<decltype(_this)>(_this.item10);
#endif
#if _libcxxext_internal_tparam_count >= 9
            else if constexpr (Idx == 8uz)
                return std::forward_like<decltype(_this)>(_this.item9);
#endif
#if _libcxxext_internal_tparam_count >= 8
            else if constexpr (Idx == 7uz)
                return std::forward_like<decltype(_this)>(_this.item8);
#endif
#if _libcxxext_internal_tparam_count >= 7
            else if constexpr (Idx == 6uz)
                return std::forward_like<decltype(_this)>(_this.item7);
#endif
#if _libcxxext_internal_tparam_count >= 6
            else if constexpr (Idx == 5uz)
                return std::forward_like<decltype(_this)>(_this.item6);
#endif
#if _libcxxext_internal_tparam_count >= 5
            else if constexpr (Idx == 4uz)
                return std::forward_like<decltype(_this)>(_this.item5);
#endif
#if _libcxxext_internal_tparam_count >= 4
            else if constexpr (Idx == 3uz)
                return std::forward_like<decltype(_this)>(_this.item4);
#endif
#if _libcxxext_internal_tparam_count >= 3
            else if constexpr (Idx == 2uz)
                return std::forward_like<decltype(_this)>(_this.item3);
#endif
#if _libcxxext_internal_tparam_count >= 2
            else if constexpr (Idx == 1uz)
                return std::forward_like<decltype(_this)>(_this.item2);
#endif
#if _libcxxext_internal_tparam_count >= 1
            else if constexpr (Idx == 0uz)
                return std::forward_like<decltype(_this)>(_this.item1);
#endif
#endif
        }

#if defined(_libcxxext_internal_tparam_count) && _libcxxext_internal_tparam_count > 0
        consteval decltype(auto) left(this auto&& _this) { return std::forward_like<decltype(_this)>(_this.item1); }
        consteval decltype(auto) right(this auto&& _this) { return std::forward_like<decltype(_this)>(_this._ppcat(item, _libcxxext_internal_tparam_sub_one)); }
#endif
    };
} // namespace sys
