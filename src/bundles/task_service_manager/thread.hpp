#pragma once
#include <future>
#include <thread>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <pthread.h>
#    include <signal.h>
#endif

class Thread
{
  public:
    virtual ~Thread() { stopThread(); }

  public:
    bool
    startThread()
    {
        if (isRunning())
        {
            return true;
        }

        stopThread();

        std::promise<bool> status;
        auto ret = status.get_future();

        runStatus_ = true;
        td_ = std::thread(std::bind(&Thread::threadRun, this, std::ref(status)));
        return ret.get();
    }

    bool
    stopThread(uint32_t millisecond = uint32_t(-1))
    {
        if (!isRunning())
        {
            if (td_.joinable())
            {
                td_.join();
            }
            return true;
        }

        runStatus_ = false;

        std::unique_lock<std::timed_mutex> lck(threadMutex_, std::chrono::milliseconds(millisecond));
        if (!lck.owns_lock())
        {
            runStatus_ = true;
            return false;
        }

        if (td_.joinable())
        {
            td_.join();
        }

        return true;
    }

    void
    Wait()
    {
        if (isRunning())
        {
            if (td_.joinable())
            {
                td_.join();
            }
            return;
        }
    }

  public:
    bool
    isRunning() const
    {
#ifdef _WIN32
        if (WaitForSingleObject(td_.native_handle(), 0) != WAIT_TIMEOUT)
        {
            return false;
        }
#else
        auto handle = td_.native_handle();
        if (handle == 0)
        {
            return false;
        }
        if (pthread_kill(handle, 0) == ESRCH)
        {
            return false;
        }
#endif
        std::unique_lock<std::timed_mutex> lck(threadMutex_, std::try_to_lock);
        return !lck.owns_lock();
    }

  private:
    void
    threadRun(std::promise<bool>& status)
    {
        std::unique_lock<std::timed_mutex> lck(threadMutex_);

        try
        {
            auto ret = this->onThreadStart();
            status.set_value(ret);
            if (!ret)
            {
                return;
            }
        }
        catch (...)
        {
            status.set_exception(std::current_exception());
            return;
        }

        while (runStatus_)
        {
            if (!this->onThreadRun())
            {
                break;
            }
            std::this_thread::yield();
        }

        try
        {
            this->onThreadEnd();
        }
        catch (...)
        {
        }
    }

  protected:
    virtual bool onThreadRun() = 0;
    virtual bool onThreadStart() = 0;
    virtual void onThreadEnd() = 0;

  private:
    mutable std::thread td_;
    bool volatile runStatus_ = false;
    mutable std::timed_mutex threadMutex_;
};
