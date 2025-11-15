#include <iostream>
#include <coroutine>
#include <queue>
#include <random>

struct NumberGenerator {
    struct promise_type {
        int current_value = 0;

        std::suspend_always yield_value(int value) noexcept {
            current_value = value;
            return {};
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        NumberGenerator get_return_object() {
            return NumberGenerator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    NumberGenerator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~NumberGenerator() {
        if (handle) handle.destroy();
    }

    bool next() {
        if (!handle.done()) {
            handle.resume();
            return !handle.done();
        }
        return false;
    }

    int value() const {
        return handle.promise().current_value;
    }
};

NumberGenerator generateRandomNumbers() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 256);

    while (true) {
        int x = dist(gen);
        co_yield x;

        if (x % 2 == 1) {
            co_return;
        }
    }
}

int main() {
    std::queue<int> q;
    NumberGenerator generator = generateRandomNumbers();

    std::cout << "Demo: Random number generator [1..256]\n";
    std::cout << "The coroutine stops when an odd number is generated.\n\n";

    while (generator.next()) {
        int value = generator.value();
        q.push(value);

        std::cout << "Generated: " << value << "\n";

        if (value % 2 == 1) {
            std::cout << "\nOdd number detected → Coroutine stopped by controller.\n";
            break;
        }
    }

    std::cout << "\nQueue contents:\n";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }

    std::cout << "\n";
    return 0;
}