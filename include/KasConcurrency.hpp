#ifndef KASCONCURRENCY_HPP
#define KASCONCURRENCY_HPP
#include <condition_variable>
#include <mutex>

namespace KasConcurrency {
    class Semaphore {
      private:
        std::mutex mtx;
        std::condition_variable cv;
        int count;

      public:
        explicit Semaphore(int n);
        void wait();
        void notify();
    };
} // namespace KasConcurrency
#endif // KASCONCURRENCY_HPP