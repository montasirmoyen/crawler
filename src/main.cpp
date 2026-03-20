#include <iostream>
#include <unordered_set>
#include <thread>
#include <vector>
#include <atomic>
#include <fstream>
#include <algorithm>
#include "core/url_queue.h"
#include "network/fetcher.h"
#include "parser/html_parser.h"
#include "core/visited_set.h"

std::string xmlEscape(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (char c : value)
    {
        switch (c)
        {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '\"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&apos;";
            break;
        default:
            escaped += c;
            break;
        }
    }

    return escaped;
}

std::string extractHost(const std::string &url)
{
    size_t schemePos = url.find("://");
    size_t hostStart = (schemePos == std::string::npos) ? 0 : schemePos + 3;
    size_t hostEnd = url.find('/', hostStart);
    if (hostEnd == std::string::npos)
    {
        return url.substr(hostStart);
    }
    return url.substr(hostStart, hostEnd - hostStart);
}

void writeSitemap(const std::vector<std::string> &allUrls, const std::string &seed)
{
    std::string seedHost = extractHost(seed);
    std::vector<std::string> sitemapUrls;
    sitemapUrls.reserve(allUrls.size());

    for (const auto &url : allUrls)
    {
        if (extractHost(url) == seedHost)
        {
            sitemapUrls.push_back(url);
        }
    }

    if (std::find(sitemapUrls.begin(), sitemapUrls.end(), seed) == sitemapUrls.end())
    {
        sitemapUrls.push_back(seed);
    }

    std::sort(sitemapUrls.begin(), sitemapUrls.end());
    sitemapUrls.erase(std::unique(sitemapUrls.begin(), sitemapUrls.end()), sitemapUrls.end());

    std::ofstream out("sitemap.xml");
    if (!out.is_open())
    {
        std::cerr << "Failed to write sitemap.xml" << std::endl;
        return;
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";

    for (const auto &url : sitemapUrls)
    {
        out << "  <url>\n";
        out << "    <loc>" << xmlEscape(url) << "</loc>\n";
        out << "    <priority>" << (url == seed ? "1.0" : "0.8") << "</priority>\n";
        out << "  </url>\n";
    }

    out << "</urlset>\n";
    std::cout << "Generated sitemap.xml with " << sitemapUrls.size() << " URLs" << std::endl;
}

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
    writeSitemap(visited.snapshot(), seed);
    return 0;
}