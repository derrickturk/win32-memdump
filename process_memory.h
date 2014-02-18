#include "windows.h"

#include <cstddef>
#include <stdexcept>
#include <memory>

struct process_open_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
}

struct invalid_iterator : public std::runtime_error {
    using std::runtime_error::runtime_error;
}

class process_memory_iterator {
    public:
        process_memory_iterator() noexcept
            : proc(NULL),
              base_(nullptr),
              buf_(nullptr),
              buf_sz_(0),
              buf_off_(0) {}

        explicit process_memory_iterator(DWORD pid);

        process_memory_iterator(const process_memory_iterator& other);

        process_memory_iterator(process_memory_iterator&& other)
            noexcept;

        process_memory_iterator& operator=(
                const process_memory_iterator& other) noexcept;

        process_memory_iterator& operator=(
                process_memory_iterator&& other) noexcept;

        ~process_memory_iterator() = default;


        unsigned char operator*() const;

        unsigned char operator->() const
        {
            return *(*this);
        }

        process_memory_iterator& operator++();

        process_memory_iterator operator++(int)
        {
            process_memory_iterator i(*this);
            ++(*this);
            return i;
        }


        bool
        operator==(const process_memory_iterator& other) const noexcept
        {
            return proc_ == other.proc_
                && base == other.base_
                && buf_off_ == other.buf_off_;
        }

        bool
        operator!=(const process_memory_iterator& other) const noexcept
        {
            return !(*this == other)
        }

        explicit operator bool() const noexcept
        {
            process_memory_iterator i;
            return !(*this == i);
        }

    private:
        HANDLE proc_;
        mutable void *base_;
        mutable unique_ptr<unsigned char[]> *buf_;
        mutable std::size_t buf_sz_;
        mutable std::size_t buf_off_;
};
