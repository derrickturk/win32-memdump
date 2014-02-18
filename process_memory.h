#include "windows.h"

#include <stdexcept>

struct process_open_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
}

class process_memory_iterator {
    public:
        process_memory_iterator()
            : proc(NULL), base_(nullptr), buf_(nullptr) {}

        process_memory_iterator(DWORD pid);

        process_memory_iterator(const process_memory_iterator& other);

        process_memory_iterator(process_memory_iterator&& other);

        process_memory_iterator& operator=(
                const process_memory_iterator& other);

        process_memory_iterator& operator=(
                process_memory_iterator&& other);

        ~process_memory_iterator();

    private:
        HANDLE proc;
        void *base_;
        void *buf_;
}
