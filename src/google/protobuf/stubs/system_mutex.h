
#pragma once

#if HAS_OS_ZEPHYR
  #include <kernel.h>
#else
  #include <mutex>
#endif

#ifdef HAS_OS_ZEPHYR
/**
 * @brief optima mutex adapter for zephyr
 */
class SystemMutex {
public:
    struct k_mutex _mutex;

    /**
     * @brief optima mutex constructor
     */
    constexpr SystemMutex() noexcept {
        k_mutex_init(&_mutex);
    }

    /**
     * @brief optima mutex destructor
     */
    ~SystemMutex() noexcept {
        unlock();
    }

    /**
     * @brief optima mutex lock
     */
    inline void lock() noexcept {
        k_mutex_lock(&_mutex, K_FOREVER);
    }

    /**
     * @brief optima mutex lock attempt
     * @return bool true if lock successfull
     * @return bool false if lock unsuccessfull
     */
    [[nodiscard]] inline bool try_lock() noexcept {
        return k_mutex_lock(&_mutex, K_NO_WAIT) == 0;
    }

    /**
     * @brief optima mutex unlock
     */
    void inline unlock() noexcept {
        k_mutex_unlock(&_mutex);
    }
};

#else

using SystemMutex = std::mutex;

#endif

/// Assume the calling thread has already obtained mutex ownership
  /// and manage it.
  struct adopt_lock_t { explicit adopt_lock_t() = default; };

/// Tag used to make a scoped lock take ownership of a locked mutex.
  _GLIBCXX17_INLINE constexpr adopt_lock_t	adopt_lock { };

/** @brief A simple scoped lock type. (isolated from std::mutex)
 *
 * A lock_guard controls mutex ownership within a scope, releasing
 * ownership in the destructor.
 */
template<typename _Mutex>
class simply_lock_guard
{
public:
    typedef _Mutex mutex_type;

    explicit simply_lock_guard(mutex_type& __m) : _M_device(__m)
    { _M_device.lock(); }

    simply_lock_guard(mutex_type& __m, adopt_lock_t) noexcept : _M_device(__m)
    { } // calling thread owns mutex

    ~simply_lock_guard()
    { _M_device.unlock(); }

    simply_lock_guard(const simply_lock_guard&) = delete;
    simply_lock_guard& operator=(const simply_lock_guard&) = delete;

private:
    mutex_type&  _M_device;
};
