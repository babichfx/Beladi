#include <iostream>
#include <vector>
#include <deque>
#include <random>
#include <chrono>
#include <time.h>

#define DOLOG           false
#define CACHESIZE       10
#define TESTSIZE        1000000
#define DATAPIECES      50
#define LOADPROCESSDUR  10

template <typename T>
class CacheBLD{
    //typename used to clarify that is type and not any other thing
    using vecIt = typename std::vector<T>::iterator;
    using deqIt = typename std::deque<T>::iterator;

    std::deque<T>& inputData;
    size_t cacheSize;
    size_t hits, works; //stat
    std::vector<T> cacheArray;
    timespec timeSl, tmTmp;

    vecIt lookingForDataInCache(const T& newData){
        if(DOLOG)
            std::cout << "Looking for: " << newData << "... ";
        return find(cacheArray.begin(), cacheArray.end(), newData);
    }
    void addNewDataToCache(const T& newData) {
        cacheArray.push_back(newData);
        if(DOLOG)
            std::cout << "Added data: " << *(--(cacheArray.end())) << std::endl;
        works++;
        nanosleep(&timeSl, &tmTmp); //simulate long process
    }
    void  replaceDataInCache(const vecIt& vIt, const T& newData) {
        if(DOLOG)
            std::cout << *vIt << " replaced with: " << newData << std::endl;
        *vIt = newData;
        works++;
        nanosleep(&timeSl, &tmTmp); //simulate long process
        if(DOLOG)
            printCache();
    }
public:
    CacheBLD(size_t newSize, std::deque<T>& in) : cacheSize(newSize), inputData(in) {
        for(uint i=0; i<cacheSize; ++i)
            cacheArray.push_back(-1);
        cacheArray.shrink_to_fit();
        cacheArray.clear();
        hits = works = 0;
        timeSl.tv_nsec = LOADPROCESSDUR;
    }
    void  loadNewElement(){
        for(auto inputIt = inputData.begin(); inputIt != inputData.end(); ++inputIt){
            vecIt dataCacheIt = lookingForDataInCache(*inputIt);
            if(dataCacheIt == cacheArray.end()){    //is data in cache?
                if(cacheArray.size() >= cacheSize){ //cahce is full?
                    //std::cout << "check\n";
                    deqIt tmpInputIt = inputIt;     //to compare iterators
                    vecIt replaceInCacheIt = cacheArray.begin(); //whrere to replace
                    for(vecIt cLookIt = cacheArray.begin(); cLookIt != cacheArray.end(); ++cLookIt){
                        deqIt n = std::find(inputIt, inputData.end(), *cLookIt);
                        if(n == inputData.end()){   //if data never repeat
                            replaceInCacheIt = cLookIt;
                            break;
                        }
                        if(n > tmpInputIt){     //farthest data
                            tmpInputIt = n;
                            replaceInCacheIt = cLookIt;
                        }
                    }
                    replaceDataInCache(replaceInCacheIt, *inputIt);
                } else { //cache not full
                    addNewDataToCache(*inputIt);
                }//cache is fill if
            } else { //if data was found in cache
                hits++;
                if(DOLOG)
                    std::cout << "Hit with: " << *inputIt << std::endl;
            }//data in cache if
        }
    }
    void  printCache(){
        std::cout << "Cache capacity: " << cacheArray.capacity()
                  << ". Cache size: " << cacheArray.size() << " --> ";
        for(auto& el : cacheArray){
            std::cout << el << ", ";
        }
        std::cout << std::endl;
    }
    void  printStat(){
        std::cout << "Accesses to memory: " << works << ", ";
        std::cout << "hits: " << hits << ".\n";
        std::cout << "---------------------------------------------\n";
    }
};

int main()
{
    using std::cout;
    using std::endl;

    std::deque<int> inData1;
    std::srand(15);
    for(int i=0; i<TESTSIZE; i++)
        inData1.push_back(rand() % DATAPIECES);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    CacheBLD<int> cache1(CACHESIZE, inData1);
    cache1.loadNewElement();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    cache1.printCache();
    cache1.printStat();

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                 (end - begin).count()/1000000.0 << "[sec]" << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>
                 (end - begin).count() << "[µs]" << std::endl;

    return 0;
}
