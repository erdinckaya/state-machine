#pragma once
#include <variant>

namespace sm {
    //region traits
    template<bool VAL>
    struct set_bool {
        static constexpr bool value = VAL;
    };

    struct true_type : set_bool<true> {};
    struct false_type : set_bool<false> {};


    template<typename T, typename V = void>
    struct is_state : false_type {};

    template<typename T>
    struct is_state<T, std::void_t<
                               decltype(std::declval<T>().enter()),
                               decltype(std::declval<T>().exit())>> : set_bool<std::is_same_v<decltype(std::declval<T>().enter()), void> &&
                                                                      std::is_same_v<decltype(std::declval<T>().exit()), void>> {};

    template<typename T>
    constexpr bool is_state_v = is_state<T>::value;

    template<typename T, typename V = void>
    struct has_internal_table : false_type {};

    template<typename T>
    struct has_internal_table<T, std::void_t<typename T::InternalTable>> : true_type {};

    template<typename T>
    constexpr bool has_internal_table_v =  has_internal_table<T>::value;


    template<typename... States>
    using StatePack = std::variant<States...>;

    template <typename ...States>
    struct check_states : false_type {};

    template<typename ...States>
    struct check_states<StatePack<States...>> : set_bool<std::conjunction_v<is_state<States>...>> {};

    template <typename ...States>
    constexpr bool check_states_v = check_states<States...>::value;

    //endregion

    //region StateMachine

    template<typename... Types>
    using TypePack = std::variant<Types...>;

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    struct AnyEvent {};
    struct DoNothing {};
    struct AnyCondition {};

    template<typename EventT, typename CurrStateT, typename NextStateT, typename ActionT, typename ConditionT>
    struct TableRow {
        using Event = EventT;
        using CurrState = CurrStateT;
        using NextState = NextStateT;
        using Action = ActionT;
        using Condition = ConditionT;
    };

    template<typename... StateMachineTableRow>
    using StateMachineTable = std::variant<StateMachineTableRow...>;


    template<typename EventT, typename ActionT, typename ConditionT>
    struct InternalTableRow {
        using Event = EventT;
        using Action = ActionT;
        using Condition = ConditionT;
    };

    template<typename... Rows>
    using InternalStateMachineTable = std::variant<Rows...>;


    template<typename Head>
    struct InternalStateMachineEngine {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            return false;
        }
    };

    template<typename Head>
    struct InternalStateMachineEngine<InternalStateMachineTable<Head>> {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            using RowEvent = typename Head::Event;

            if constexpr (!std::is_same_v<RowEvent, Event> && !std::is_same_v<AnyEvent, Event>) {
                return false;
            }

            using Condition = typename Head::Condition;
            if (!std::is_same_v<Condition, AnyCondition> && !Condition::check(stateMachine, event)) {
                return false;
            }

            using Action = typename Head::Action;
            if constexpr (!std::is_same_v<DoNothing, Action>) {
                Action::execute(stateMachine, event);
            }

            return true;
        }
    };

    template<typename Head, typename... Tail>
    struct InternalStateMachineEngine<InternalStateMachineTable<Head, Tail...>> {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            if (InternalStateMachineEngine<InternalStateMachineTable<Head>>::onEvent(stateMachine, event)) {
                return true;
            }
            return InternalStateMachineEngine<InternalStateMachineTable<Tail...>>::onEvent(stateMachine, event);
        }
    };

    template<typename StatePack>
    struct StateMachine {
        static_assert(check_states_v<StatePack>);

        template<typename StateT>
        void updateState(const StateT &state) {

            std::visit(overloaded{
                               [](auto s) {
                                   s.exit();
                               },
                       },
                       mState);

            mState = state;
            std::visit(overloaded{
                               [](auto s) {
                                   s.enter();
                               },
                       },
                       mState);
        }

        StatePack mState;
    };


    template<typename Head>
    struct ANDImpl {
    };

    template<typename First>
    struct ANDImpl<TypePack<First>> {
        template<typename StateMachine, typename Event>
        static bool check(const StateMachine &stateMachine, const Event &event) {
            return First::check(stateMachine, event);
        }
    };

    template<typename First, typename... Tail>
    struct ANDImpl<TypePack<First, Tail...>> {
        template<typename StateMachine, typename Event>
        static bool check(const StateMachine &stateMachine, const Event &event) {
            return ANDImpl<TypePack<First>>::check(stateMachine, event) && ANDImpl<TypePack<Tail...>>::check(stateMachine, event);
        }
    };

    template<typename... Tail>
    struct AND : ANDImpl<TypePack<Tail...>> {};

    template<typename Head>
    struct ORImpl {
    };

    template<typename First>
    struct ORImpl<TypePack<First>> {
        template<typename StateMachine, typename Event>
        static bool check(const StateMachine &stateMachine, const Event &event) {
            return First::check(stateMachine, event);
        }
    };

    template<typename First, typename... Tail>
    struct ORImpl<TypePack<First, Tail...>> {
        template<typename StateMachine, typename Event>
        static bool check(const StateMachine &stateMachine, const Event &event) {
            return ORImpl<TypePack<First>>::check(stateMachine, event) || ORImpl<TypePack<Tail...>>::check(stateMachine, event);
        }
    };

    template<typename... Tail>
    struct OR : ORImpl<TypePack<Tail...>> {};

    template<typename Condition>
    struct NOT {
        template<typename StateMachine, typename Event>
        static bool check(const StateMachine &stateMachine, const Event &event) {
            return !Condition::check(stateMachine, event);
        }
    };


    template<typename Head>
    struct StateMachineEngine {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            return false;
        }
    };

    template<typename Head>
    struct StateMachineEngine<StateMachineTable<Head>> {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            using RowCurrState = typename Head::CurrState;
            using RowEvent = typename Head::Event;
            if (!std::holds_alternative<RowCurrState>(stateMachine.mState)) {
                return false;
            }

            if constexpr (has_internal_table_v<RowCurrState>) {
                using InternalTable = typename RowCurrState::InternalTable;
                if (InternalStateMachineEngine<InternalTable>::onEvent(stateMachine, event)) {
                    return true;
                }
            }

            if constexpr (!std::is_same_v<RowEvent, Event> && !std::is_same_v<AnyEvent, Event>) {
                return false;
            }

            using Condition = typename Head::Condition;
            if (!std::is_same_v<Condition, AnyCondition> && !Condition::check(stateMachine, event)) {
                return false;
            }

            using RowNextState = typename Head::NextState;
            using Action = typename Head::Action;
            if constexpr (!std::is_same_v<DoNothing, Action>) {
                Action::execute(stateMachine, event);
            }
            stateMachine.updateState(RowNextState{});

            return true;
        }
    };

    template<typename Head, typename... Tail>
    struct StateMachineEngine<StateMachineTable<Head, Tail...>> {
        template<typename StateMachine, typename Event>
        static bool onEvent(StateMachine &stateMachine, const Event &event) {
            if (StateMachineEngine<StateMachineTable<Head>>::onEvent(stateMachine, event)) {
                return true;
            }
            return StateMachineEngine<StateMachineTable<Tail...>>::onEvent(stateMachine, event);
        }
    };
    //endregion
}// namespace sm
