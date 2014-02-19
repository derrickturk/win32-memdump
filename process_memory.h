#include "windows.h"

#include <cstddef>
#include <stdexcept>
#include <memory>
#include <iterator>

struct process_open_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct invalid_iterator : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct memory_query_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct memory_read_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class process_memory_iterator {
    public:
        process_memory_iterator() noexcept
            : proc_(nullptr),
              base_(nullptr),
              buf_(nullptr),
              buf_sz_(0),
              buf_off_(0) {}

        explicit process_memory_iterator(DWORD pid);

        process_memory_iterator(const process_memory_iterator&) noexcept
            = default;

        process_memory_iterator(process_memory_iterator&&) noexcept
            = default;

        process_memory_iterator& operator=(const process_memory_iterator&)
            = default;

        process_memory_iterator& operator=(process_memory_iterator&&)
            = default;

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
                && buf_ == other.buf_
                && buf_sz_ == other.buf_sz_
                && base_ == other.base_
                && buf_off_ == other.buf_off_;
        }

        bool
        operator!=(const process_memory_iterator& other) const noexcept
        {
            return !(*this == other);
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(proc_);
        }


        process_memory_iterator begin() const { return *this; }

        process_memory_iterator end() const
        { return process_memory_iterator {}; }
        

        using difference_type = void;
        using value_type = unsigned char;
        using pointer = unsigned char*;
        using reference = unsigned char&;
        using iterator_category = std::forward_iterator_tag;

    private:
        std::shared_ptr<HANDLE> proc_;
        mutable void *base_;
        mutable std::shared_ptr<unsigned char> buf_;
        mutable std::size_t buf_sz_;
        mutable std::size_t buf_off_;

        void update_pagerange();
        void read_memory(void *new_base, std::size_t new_size);
};
