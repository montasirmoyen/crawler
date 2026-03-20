# crawler
A high-performance application designed to recursively traverse the web by leveraging a multi-threaded architecture and robust synchronization primitives for concurrency and resource management.

Initially created to explore multi-threading in a more applied way, beyond the Operating Systems class I'm taking this semester.

This project was part of a challenge: https://www.montasirmoyen.com/blog/crawler

<details>
<summary>Latest Additions</summary>
<ul>
<li>3/20/2026: sitemap.xml generation after crawling</li>
<li>3/13/2026: initial release</li>
</ul>
</details>


## Usage
```
git clone https://github.com/montasirmoyen/crawler
cd crawler
cmake -S . -B build
cmake --build build
./build/crawler <url> <limit> <num_threads>
```
**Example Usage**
```
./build/crawler https://www.montasirmoyen.com/ 3 1  
```

![Usage Example](public/usage.png)