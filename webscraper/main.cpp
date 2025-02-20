#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <curl/curl.h>
#include <zlib.h>

struct FetchedData {
    std::string url;
    std::string rawData;
    std::string compressedData;
};

class ThreadSafeQueue {
private:
    std::queue<std::shared_ptr<FetchedData>> queue;
    std::mutex mtx;
    std::condition_variable cv;
public:
    void push(std::shared_ptr<FetchedData> data) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(data);
        cv.notify_one();
    }
    std::shared_ptr<FetchedData> pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        auto data = queue.front();
        queue.pop();
        return data;
    }
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string fetchURL(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

std::string compressData(const std::string& input) {
    z_stream zs{};
    deflateInit(&zs, Z_BEST_COMPRESSION);
    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();
    char outbuffer[32768];
    zs.next_out = (Bytef*)outbuffer;
    zs.avail_out = sizeof(outbuffer);
    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);
    return std::string(outbuffer, zs.total_out);
}

void producer(ThreadSafeQueue& queue, const std::vector<std::string>& urls) {
    for (const auto& url : urls) {
        auto data = std::make_shared<FetchedData>();
        data->url = url;
        data->rawData = fetchURL(url);
        data->compressedData = compressData(data->rawData);
        queue.push(data);
    }
}

void consumer(ThreadSafeQueue& queue) {
    while (true) {
        auto data = queue.pop();
        if (!data) break;

        std::ofstream rawFile("./data/raw_" + data->url.substr(data->url.find("//") + 2) + ".txt");
        rawFile << data->rawData;
        rawFile.close();

        std::ofstream compFile("./data/compressed_" + data->url.substr(data->url.find("//") + 2) + ".bin", std::ios::binary);
        compFile.write(data->compressedData.c_str(), data->compressedData.size());
        compFile.close();

        std::cout << "Saved data for: " << data->url << "\n";
    }
}

int main() {
    ThreadSafeQueue queue;
    std::vector<std::string> urls = {"https://example.com", "https://www.google.com"};
    
    std::thread prodThread(producer, std::ref(queue), urls);
    std::thread consThread(consumer, std::ref(queue));
    
    prodThread.join();
    queue.push(nullptr);
    consThread.join();
    return 0;
}

