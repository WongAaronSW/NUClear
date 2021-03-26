/*
 * Copyright (C) 2013      Trent Houliston <trent@houliston.me>, Jake Woods <jake.f.woods@gmail.com>
 *               2014-2017 Trent Houliston <trent@houliston.me>
                 2021      Aaron Wong <aaron.wong@live.com.au>
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

#include <catch.hpp>
#include <nuclear>

namespace {

int i               = 0;
int j               = 0;
int number_of_emits = 5;

class TestReactor : public NUClear::Reactor {
public:
    TestReactor(std::unique_ptr<NUClear::Environment> environment) : Reactor(std::move(environment)) {

        on<Once<Always>>().then("", [this] {
            // Increment the counter,
            ++i;

            // Function finished, then should unbind.
        });
        on<Always>().then([this] {
            // Increment j, to ensure that system prcesses other messages,
            ++j;
            // Once j reaches the proposed amount, we shutdown.
            if (j == number_of_emits) { powerplant.shutdown(); }
        });
    }
};
}  // namespace

TEST_CASE("Testing on<Once> functionality", "[api][once]") {

    NUClear::PowerPlant::Configuration config;
    config.thread_count = 1;
    NUClear::PowerPlant plant(config);

    // We are installing with an initial log level of debug
    plant.install<TestReactor, NUClear::DEBUG>();

    // We'll make send 5 emits, but only one should run.
    plant.emit(std::make_unique<int>(number_of_emits));

    plant.start();

    REQUIRE(i == 1);
}
