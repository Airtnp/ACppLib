//
// Created by xiaol on 12/1/2019.
//

#ifndef ACPPLIB_GENERATOR_HPP
#define ACPPLIB_GENERATOR_HPP

#include "sn_CommonHeader.h"

#ifdef SN_ENABLE_CPP_20_EXPERIMENTAL
#include <experimental/coroutine>

namespace sn_Async {
    /// MSVC STL generator implementation
    /// @ref: https://github.com/microsoft/STL/blob/master/stl/inc/experimental/generator
    using namespace std::experimental;

    template <typename T, typename Alloc = std::allocator<char>>
    struct generator {
        struct promise_type {
            const T* currentValue;
            std::exception_ptr eptr;
            auto get_return_object() {
                return generator{*this};
            }

            suspend_always initial_suspend() noexcept {
                return {};
            }

            suspend_always final_suspend() noexcept {
                return {};
            }

            void unhandled_exception() noexcept {
                eptr = std::current_exception();
            }

            void rethrow_if_exception() {
                if (eptr) {
                    std::rethrow_exception(eptr);
                }
            }

            auto yield_value(const T& value) {
                currentValue = std::addressof(value);
                return suspend_always{};
            }

            void return_void() {}

            template <typename U>
            U&& await_transform(U&& v) {
                return std::forward<U>(v);
            }

            static void* operator new(size_t size) {
                Alloc al;
                return std::allocator_traits<Alloc>::allocate(al, size);
            }

            static void* operator delete(size_t size) {
                Alloc al;
                return std::allocator_traits<Alloc>::deallocate(al, size);
            }
        };

        struct iterator {
            using iterator_category = input_iterator_tag;
            using difference_type   = ptrdiff_t;
            using value_type        = T;
            using reference         = T const&;
            using pointer           = T const*;

            coroutine_handle<promise_type> coro = nullptr;

            iterator() = default;
            iterator(nullptr_t) : coro(nullptr) {}

            iterator(coroutine_handle<promise_type> coroArg) : coro(coroArg) {}

            iterator& operator++() {
                coro.resume();
                if (coro.done()) {
                    std::exchange(coro, {}).promise().rethrow_if_exception();
                }
                return *this;
            }

            void operator++(int) {
                ++*this;
            }

            [[nodiscard]] bool operator==(iterator const& rhs) const {
                return coro == rhs.coro;
            }

            [[nodiscard]] bool operator!=(iterator const& rhs) const {
                return !(*this == rhs);
            }

            [[nodiscard]] reference operator*() const {
                return *coro.promise().currentValue;
            }

            [[nodiscard]] pointer operator->() const {
                return coro.promise().currentValue;
            }
        };

        [[nodiscard]] iterator begin() {
            if (coro) {
                coro.resume();
                if (coro.done()) {
                    coro.promise().rethrow_if_exception();
                    return {nullptr};
                }
            }
            return {coro};
        }

        [[nodiscard]] iterator end() {
            return {nullptr};
        }

        explicit generator(promise_type& prom) : coro{coroutine_handle<promise_type>::from_promise(prom)} {}
        generator() = default;
        generator(generator const&) = delete;
        generator& operator=(generator const&) = delete;

        generator(generator&& rhs) : coro{rhs.coro} {
            rhs.coro = nullptr;
        }

        generator operator==(generator&& rhs) {
            if (this != std::addressof(rhs)) {
                coro = rhs.coro;
                rhs.coro = nullptr;
            }
            return *this;
        }

        ~generator() {
            if (coro) {
                coro.destroy();
            }
        }
    private:
        coroutine_handle<promise_type> coro = nullptr;
    };

    struct none_generator {
        struct promise_type {
            auto initial_suspend() { return suspend_never{}; }
            auto final_suspend() { return suspend_never{}; }
            auto get_return_object() { return none_generator{}; }
            void unhandled_exception() noexcept {}
            void return_void() {}
        };
    };

    template <class T>
    struct range_generator {
        struct await_callback {
            bool await_ready() { return false; }
            coroutine_handle<> await_suspend(coroutine_handle<>) {
                std::cout << "Suspend inner\n";
                return handle;
            }
            void await_resume() {}
            coroutine_handle<>& handle;
        };
        struct promise_type {
            T current_value;
            coroutine_handle<> handle;
            bool has_finished = false;
            auto yield_value(T v) { current_value = v; return await_callback{handle}; }
            auto initial_suspend() { return suspend_always{}; }
            auto final_suspend() { has_finished = true; return suspend_always{}; }
            auto get_return_object() { return range_generator{*this}; }
            void unhandled_exception() noexcept {}
            void return_void() {}
        };
        using handle_type = coroutine_handle<promise_type>;
        struct normal_iterator;
        // co_await begin()
        struct await_iterator {
            handle_type& handle;
            bool await_ready() {
                return false;
            }
            coroutine_handle<promise_type> await_suspend(coroutine_handle<> h) {
                std::cout << "Suspend outer\n";
                handle.promise().handle = h;
                return handle;
            }
            normal_iterator await_resume() {
                return normal_iterator{handle, handle.promise().has_finished};
            }
        };
        // normal end()
        struct normal_iterator {
            handle_type& handle;
            normal_iterator(handle_type& p, bool is_end): handle{p}, is_end{is_end} {}
            await_iterator operator++() {
                return await_iterator{handle};
            }
            T operator*() {
                return handle.promise().current_value;
            }
            bool is_end;
            bool operator!=(const normal_iterator& rhs) {
                return handle != rhs.handle || is_end != rhs.is_end;
            }
        };
        await_iterator begin() { return await_iterator{handle}; }
        normal_iterator end() { return normal_iterator{handle, true}; }
        range_generator(promise_type& promise): handle{handle_type::from_promise(promise)} {}
        handle_type handle;
    };
}
#endif

#endif //ACPPLIB_GENERATOR_HPP
