#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

class SafeQueue
{
private:
    std::queue<std::string> q;
    std::mutex m;               // using this to protect the queue
    std::condition_variable cv; // to manage thread sleeping/waking
    bool shutdown = false;      // the "switch"
public:
    void push(std::string url)
    {
        std::lock_guard<std::mutex> lock(m);
        if (shutdown)
            return;
        q.push(url);
        cv.notify_one();
    }

    void requestShutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m);
            shutdown = true;
        }
        cv.notify_all();
    }

    bool pop(std::string &val)
    {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]
                { return !q.empty() || shutdown; });

        if (shutdown && q.empty()) return false;

        val = q.front();
        q.pop();
        return true;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }
};