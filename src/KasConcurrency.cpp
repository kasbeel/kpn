#include <KasConcurrency.hpp>

namespace KasConcurrency {
    Semaphore::Semaphore(int n) : count(n) {}

    void Semaphore::wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
    }

    void Semaphore::notify() {
        std::lock_guard<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
} // namespace KasConcurrency