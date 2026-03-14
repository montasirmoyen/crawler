#include <iostream>
#include <unordered_set>
#include <thread>
#include <vector>
#include <atomic>
#include "core/url_queue.h"
#include "network/fetcher.h"
#include "parser/html_parser.h"
#include "core/visited_set.h"

void worker(SafeQueue &queue, VisitedSet &visited, Fetcher &fetcher, Parser &parser,
            std::atomic<int> &count, std::atomic<int> &activeWorkers, int limit)
{
    std::string url;

    while (queue.pop(url))
    {
        activeWorkers.fetch_add(1);

        if (count >= limit)
        {
            activeWorkers.fetch_sub(1);
            queue.requestShutdown();
            break;
        }

        if (visited.testAndInsert(url))
        {
            // check again to avoid "TOCTOU" race condition
            if (count >= limit)
            {
                activeWorkers.fetch_sub(1);
                queue.requestShutdown();
                break;
            }

            std::cout << "Thread " << std::this_thread::get_id() << " crawling: " << url << std::endl;

            std::string html = fetcher.download(url);
            auto links = parser.extractLinks(html);

            for (const auto &link : links)
            {
                queue.push(link);
            }

            count++;

            // check again, maybe this was the last page needed
            if (count >= limit)
            {
                activeWorkers.fetch_sub(1);
                queue.requestShutdown();
                break;
            }
        }

        int remainingWorkers = activeWorkers.fetch_sub(1) - 1;
        if (remainingWorkers == 0 && queue.empty())
        {
            queue.requestShutdown();
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: ./crawler <seed_url> <limit> <num_threads>" << std::endl;
        return 1;
    }

    std::string seed = argv[1];
    int limit = std::stoi(argv[2]);
    int numThreads = std::stoi(argv[3]);

    SafeQueue urlQueue;
    Fetcher fetcher;
    Parser parser;
    VisitedSet visited;
    std::atomic<int> crawledCount(0);
    std::atomic<int> activeWorkers(0);

    urlQueue.push(seed);

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(worker, std::ref(urlQueue), std::ref(visited),
                             std::ref(fetcher), std::ref(parser),
                             std::ref(crawledCount), std::ref(activeWorkers), limit);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "\nCrawling completed, total pages: " << visited.size() << std::endl;
    return 0;
}