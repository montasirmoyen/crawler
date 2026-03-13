#include <iostream>
#include "core/url_queue.h"
#include "network/fetcher.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./crawler <seed_url> <limits>" << std::endl;
        return 1;
    }

    std::string seed = argv[1];
    int limits = std::stoi(argv[2]);

    SafeQueue urlQueue;
    Fetcher fetcher;
    
    urlQueue.push(seed);
    int crawledCount = 0;

    while (crawledCount < limits) {
        std::string currentUrl = urlQueue.pop();
        std::cout << "Crawling: " << currentUrl << std::endl;

        std::string html = fetcher.download(currentUrl);

        // i could parse the html here
        crawledCount++; // but for now, ill keep it simple
    }

    std::cout << "\nCrawling completed, total pages: " << crawledCount << std::endl;
    return 0;
}