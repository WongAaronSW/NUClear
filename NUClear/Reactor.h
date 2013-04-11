#ifndef NUCLEAR_REACTOR_H
#define NUCLEAR_REACTOR_H
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <typeindex>
#include <chrono>
#include <atomic>
#include "Internal/Reaction.h"
#include "Internal/Every.h"

namespace NUClear {
    class ReactorController;

    /**
     * @brief Base class for any system that wants to react to events/data from the rest of the system.
     * @details 
     *  Provides functionality for binding callbacks to incoming data events. Callbacks are executed
     *  in a transparent, multithreaded manner.
     * @author Jake Woods
     * @author Trent Houliston
     * @version 1.1
     * @date 2-Apr-2013
     */
    class Reactor {
        public:
            friend class ReactorController;

            Reactor(ReactorController& reactorController);
            ~Reactor();

            /**
             * @brief Notifies this reactor that an event has occured
             * @tparam TTrigger the type of the event that occured.
             */
            template <typename TTrigger>
            void notify();   
        protected:
            /**
             * @brief Empty wrapper class that represents a collection of triggering elements
             * @tparam TTriggers The list of triggers.
             */
            template <typename... TTriggers>
            class Trigger { Trigger() = delete; ~Trigger() = delete; };

            /**
             * @brief Empty wrapper class that represents a collection of required but non-triggering data/events
             * @tparam TWith The list of required data/events.
             */
            template <typename... TWith>
            class With { With() = delete; ~With() = delete; };

            template <typename... TOption>
            class Options { Options() = delete; ~Options() = delete; };

            // Provide access to NUClear::Every directly.
            template <int ticks, class period = std::chrono::milliseconds>
            using Every = NUClear::Internal::Every<ticks, period>;

            template <typename TTrigger, typename TFunc>
            void on(TFunc callback); 

            template <typename TTrigger, typename TWithOrOption, typename TFunc>
            void on(TFunc callback); 

            template <typename TTrigger, typename TWith, typename TOption, typename TFunc>
            void on(TFunc callback);
        private:
            ReactorController& reactorController;
            std::map<std::type_index, std::vector<Internal::Reaction>> m_callbacks;
            
            /**
             * @brief Base template instantitation that gets specialized. 
             * @details 
             *  This should never be instantiated and will throw a giant compile error if it somehow is. 
             *  The template parameters are left unnamed to reflect the fact that they are simply placeholders.
             */
            template <typename, typename, typename, typename>
            struct OnImpl {};

            /**
             * @brief Standard Trigger<...>, With<...> specialization of OnImpl.
             * @details 
             *  This class essentially acts as a polymorphic lambda function in that it allows us
             *  to select what specialization of OnImpl we want based on the compile-time template arguments.
             *  In this case this specialization matches OnImpl<Trigger<...>, With<...>>(callback) such that
             *  we can get the individual "trigger" and "with" lists. Essentially, think of this as a function 
             *  rather then a class as that's what it would be if C++ allowed partial template specialization of
             *  functions.
             * @tparam TTriggers the list of events/data that trigger this reaction
             * @tparam TWiths the list of events/data that is required for this reaction but does not trigger the reaction.
             * @tparam TFunc the callback type, should be automatically deduced
             */
            template <typename... TTriggers, typename... TWiths, typename... TOptions, typename TFunc>
            struct OnImpl<Trigger<TTriggers...>, With<TWiths...>, Options<TOptions...>, TFunc> {
                Reactor* context;
                OnImpl(Reactor* context); 
                void operator()(TFunc callback);            
            };
            
            /**
             * @brief Builds a callback wrapper function for a given callback. 
             * @details 
             *  All callbacks are wrapped in a void() lambda that knows how to get the correct arguments
             *  when called. This is done so all callbacks can be stored and called in a uniform way.
             * @tparam TFunc the callback type
             * @tparam TTriggersAndWiths the list of all triggers and required data for this reaction
             * @param callback the callback function
             * @returns The wrapped callback
             */
            template <typename TFunc, typename... TTriggersAndWiths>
            Internal::Reaction buildReaction(TFunc callback);

            
            /**
             * @brief Adds a single data -> callback mapping for a single type.
             * @tparam TTrigger the event/data type to add the callback for
             * @param callback the callback to add
             */
            template <typename TTrigger>
            void bindTriggers(Internal::Reaction callback);

            /**
             * @brief Recursively calls the single-param bindTriggers method for every trigger in the list.
             * @tparam TTriggerFirst the next trigger to call bindTriggers on
             * @tparam TTriggerSecond the following trigger to call bindTriggers on
             * @tparam TTriggers the remaining triggers to evaluate
             * @param callback the callback to bind to all of these triggers.
             */
            template <typename TTriggerFirst, typename TTriggerSecond, typename... TTriggers>
            void bindTriggers(Internal::Reaction callback);
            /**
             * @brief The implementation method for bindTriggers, provides partial template specialization for specific-trigger type logic.
             * @tparam TTrigger the trigger to bind to
             * @param callback the callback to bind
             * @param placeholder used for partial template specialization
             */
            template <typename TTrigger>
            void bindTriggersImpl(Internal::Reaction callback, TTrigger* /*placeholder*/);
        
            /**
             * @brief The implementation method for bindTriggers, provides partial template specialization for specific-trigger type logic.
             * @tparam TTrigger the trigger to bind to
             * @param callback the callback to bind
             * @param placeholder used for partial template specialization
             */
            template <int ticks, class period = std::chrono::milliseconds>
            void bindTriggersImpl(Internal::Reaction callback, Every<ticks, period>* /*placeholder*/);
        
            /**
             * @brief Gets the callback list for a given type
             * @tparam TTrigger the type to get the callback list for
             * @returns The callback list
             */
            template <typename TTrigger>
            std::vector<Internal::Reaction>& getCallbackList();     
    };
}

#include "ReactorController.h"
#include "Reactor.ipp"
#endif

