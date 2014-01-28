/**
 * Copyright (C) 2013 Jake Woods <jake.f.woods@gmail.com>, Trent Houliston <trent@houliston.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NUCLEAR_REACTOR_H
#define NUCLEAR_REACTOR_H

#include <string>
#include <sstream>
#include <functional>
#include <vector>
#include <typeindex>
#include <chrono>
#include <atomic>
#include "nuclear_bits/threading/Reaction.h"
#include "nuclear_bits/threading/ReactionHandle.h"
#include "nuclear_bits/dsl/dsl.h"
#include "nuclear_bits/metaprogramming/unpack.h"
#include "nuclear_bits/metaprogramming/apply.h"
#include "nuclear_bits/metaprogramming/TypeMap.h"
#include "nuclear_bits/metaprogramming/MetaProgramming.h"
#include "nuclear_bits/metaprogramming/buildVector.h"
#include "nuclear_bits/ForwardDeclarations.h"

namespace NUClear {
    
    // Import our meta programming utility
    using namespace metaprogramming;

    /**
     * @brief Base class for any system that wants to react to events/data from the rest of the system.
     *
     * @details 
     *  Provides functionality for binding callbacks to incoming data events. Callbacks are executed
     *  in a transparent, multithreaded manner. TODO needs to be expanded and updated
     *
     * @author Jake Woods
     * @author Trent Houliston
     * @version 1.1
     * @date 2-Apr-2013
     */
    class Reactor {
        public:
            friend class PowerPlant;

            Reactor(std::unique_ptr<Environment> environment);
            ~Reactor();
 
        protected:
        
        /***************************************************************************************************************
         * The types here are imported from other contexts so that when extending from the Reactor type in normal      *
         * usage there does not need to be any namespace declarations on the used types. This affords a simpler API    *
         * for the user.                                                                                               *
         **************************************************************************************************************/
            std::unique_ptr<Environment> environment;

            /// @brief TODO
            PowerPlant* powerPlant;
        
            /// @brief TODO inherit from commandtype
            using time_t = clock::time_point;
        
            /// @brief TODO inherit from commandtype
            template <typename... TTriggers>
            using Trigger = dsl::Trigger<TTriggers...>;
        
            /// @brief TODO inherit from commandtype
            template <typename... TWiths>
            using With = dsl::With<TWiths...>;
        
            /// @brief TODO inherit from commandtype
            using Scope = dsl::Scope;
        
            /// @brief TODO inherit from commandtype
            template <typename... TOptions>
            using Options = dsl::Options<TOptions...>;
        
            /// @brief TODO inherit from commandtype
            using Initialize = dsl::Initialize;
        
            /// @brief TODO inherit from commandtype
            using Shutdown = dsl::Shutdown;
        
            /// @brief TODO inherit from commandtype
            template <int ticks, class period = std::chrono::milliseconds>
            using Every = dsl::Every<ticks, period>;
        
            /// @brief TODO inherit from commandtype
            template <class period>
            using Per = dsl::Per<period>;
        
            /// @brief TODO inherit from commandtype
            template <typename TData>
            using Raw = dsl::Raw<TData>;
        
            /// @brief TODO inherit from commandtype
            template <int num, class TData>
            using Last = dsl::Last<num, TData>;

            /// @brief The type of data that is returned by Last<num, TData>
            template <class TData>
            using LastList = std::vector<std::shared_ptr<const TData>>;

            /// @breif TODO inherit docs from commandtype
            using CommandLineArguments = dsl::CommandLineArguments;
        
            /// @brief TODO inherit from commandtype
            template <enum EPriority P>
            using Priority = dsl::Priority<P>;
        
            /// @brief TODO inherit from commandtype
            template <typename TData>
            using Network = dsl::Network<TData>;
        
            /// @brief TODO inherit from commandtype
            template <typename TSync>
            using Sync = dsl::Sync<TSync>;
        
            /// @brief TODO inherit from commandtype
            using Single = dsl::Single;
        
            /// @brief This provides functions to modify how an on statement runs after it has been created
            using ReactionHandle = threading::ReactionHandle;

        // FUNCTIONS
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             *
             * @tparam TParams  TODO
             * @tparam TFunc    TODO
             *
             * @param callback TODO
             */
            template <typename... TParams, typename TFunc>
            Reactor::ReactionHandle on(TFunc callback);
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             *
             * @tparam THandlers
             * @tparam TData
             *
             * @param data
             */
            template <typename... THandlers, typename TData>
            void emit(std::unique_ptr<TData>&& data);

        private:
            /// @brief TODO
            template <typename TKey>
            using CallbackCache = metaprogramming::TypeList<Reactor, TKey, std::unique_ptr<threading::Reaction>>;
        
            /**
             * @brief Base template instantitation that gets specialized. 
             *
             * @details 
             *  This should never be instantiated and will throw a giant compile error if it somehow is. 
             *  The template parameters are left unnamed to reflect the fact that they are simply placeholders.
             */
            template <typename, typename...>
            struct On;
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             */
            template <typename, typename...>
            struct OnBuilder;
        
            template <typename TFunc, typename TTuple, int Stage = 0>
            struct CheckFunctionSignature;
        
            /**
             * @brief Standard Trigger<...>, With<...> specialization of On.
             *
             * @details 
             *  This class essentially acts as a polymorphic lambda function in that it allows us
             *  to select what specialization of On we want based on the compile-time template arguments.
             *  In this case this specialization matches OnImpl<Trigger<...>, With<...>>(callback) such that
             *  we can get the individual "trigger" and "with" lists. Essentially, think of this as a function 
             *  rather then a class as that's what it would be if C++ allowed partial template specialization of
             *  functions.
             *
             * @tparam TTriggers    the list of events/data that trigger this reaction
             * @tparam TWiths       the list of events/data that is required for this reaction but does not trigger the
             *                      reaction.
             * @tparam TOptions     the options that the callback is executed with
             * @tparam TFunc        the callback type, should be automatically deduced
             */
            template <typename TFunc, typename... TTriggers, typename... TWiths, typename... TOptions, typename... TFuncArgs>
            struct On<TFunc, Trigger<TTriggers...>, With<TWiths...>, Options<TOptions...>, std::tuple<TFuncArgs...>> {
                static ReactionHandle on(Reactor* context, TFunc callback);
            };
        
            /**
             * @brief This function populates the passed ReactionOptions object will all of the data held in the options.
             *
             * @tparam the options which to apply to this options object
             *
             * @param the options to populate
             */
            template <typename... TOption>
            void buildOptions(threading::ReactionOptions& options);
        
            /**
             * @brief This case of build options is used when the Single option is specified.
             *
             * @param options the options object we are building
             * @param placeholder which is used to specialize this method
             */
            void buildOptionsImpl(threading::ReactionOptions& options, Single* /*placeholder*/);
        
            /**
             * @brief This case of build options is used to add the Sync option.
             *
             * @tparam TSync the sync type we are synchronizing on
             *
             * @param placeholder which is used to specialize this method
             */
            template <typename TSync>
            void buildOptionsImpl(threading::ReactionOptions& options, Sync<TSync>* /*placeholder*/);
        
            /**
             * @brief This case of build options is used to add the Priority option.
             *
             * @tparam TSync the sync type we are synchronizing on
             *
             * @param placeholder which is used to specialize this method
             */
            template <enum EPriority P>
            void buildOptionsImpl(threading::ReactionOptions& options, Priority<P>* /*placeholder*/);
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             *
             * @tparam TData
             */
            template <typename TData>
            struct Exists;
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             */
            struct BasicCallback;
            
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             */
            struct FillCallback;
        
            /**
             * @brief Builds a callback wrapper function for a given callback. TODO this needs updating
             *
             * @details 
             *  All callbacks are wrapped in a void() lambda that knows how to get the correct arguments
             *  when called. This is done so all callbacks can be stored and called in a uniform way.
             *
             * @tparam TFunc the callback type
             * @tparam TTriggersAndWiths the list of all triggers and required data for this reaction
             *
             * @param callback the callback function
             *
             * @returns The wrapped callback
             */
            template <typename TFunc, typename... TTriggersAndWiths>
            std::unique_ptr<threading::Reaction> buildReaction(TFunc callback, threading::ReactionOptions& options);
        
            /**
             * @brief Adds a single data -> callback mapping for a single type.
             *
             * @tparam TTrigger the event/data type to add the callback for
             *
             * @param callback the callback to add
             *
             * @return A handler which allows us to modify our Reaction at runtime
             */
            template <typename TTrigger, typename... TTriggers>
            ReactionHandle bindTriggers(std::unique_ptr<threading::Reaction>&& callback);
        
            /**
             * @brief TODO
             *
             * @details
             *  TODO
             *
             * @tparam TData TODO
             */
            template <typename TData>
            struct TriggerType;
    };
}

// We need to really make sure that PowerPlant is included as we use it in our ipp file
#include "nuclear_bits/Environment.h"
#include "nuclear_bits/PowerPlant.h"
#include "nuclear_bits/Reactor.ipp"
#endif

