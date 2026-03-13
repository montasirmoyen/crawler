#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

class SafeQueue {
private:
    std::queue<std::string> q;
    std::mutex m; // using this to protect the queue
    std::condition_variable cv; // to manage thread sleeping/waking
public:
    void push(std::string url) {
        std::lock_guard<std::mutex> lock(m);
        q.push(url);
        cv.notify_one(); 
    }

    std::string pop() {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]{ return !q.empty(); });
        std::string val = q.front();
        q.pop();
        return val;
    }
};