#include <unordered_set>
#include <mutex>
#include <string>

class VisitedSet {
private:
    std::unordered_set<std::string> set;
    std::mutex mtx;
public:
    bool testAndInsert(const std::string& url) {
        std::lock_guard<std::mutex> lock(mtx);
        if (set.find(url) != set.end()) {
            return false; // already visited
        }
        set.insert(url);
        return true;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return set.size();
    }
};