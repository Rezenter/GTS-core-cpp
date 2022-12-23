#ifndef SHTRIPPER_COMPRESS_H
#define SHTRIPPER_COMPRESS_H

#include <thread>
#include <mutex>
#include <vector>
#include <filesystem>
#include "json.hpp"

using Json = nlohmann::json;

static const char V2[] = "ANALIZER1.2";
#define CH_COUNT 16


typedef struct {
    unsigned short year;
    unsigned short month;
    unsigned short dayOfWeek;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milliseconds;
} Time;
typedef struct {
    int type;
    char name[128];
    char comment[128];
    char unit[128];
    Time time;
    unsigned int nPoints;
    double tMin, tMax, yMin, delta;
    unsigned char data[1];
}CombiscopeHistogram;


typedef struct {
    unsigned char Bits[32];
    unsigned char NBits;
}Code;
class Knot{
public:
    int weight;
    int parent;

    Knot() : weight(0), parent(255){};
};

typedef struct {
    const char* data;
    const int size;
} CompressedHoff;
typedef struct {
    int size;
    char* point;
} Out;
class CompressedRLE{
public:
    unsigned char* data;
    const int size;

    explicit CompressedRLE(const int size) : size(size){
        data = new unsigned char [size];
    };
    ~CompressedRLE(){
        delete[] data;
    }
};

union LongFlip{
    long asLong;
    unsigned char asChar[4];
};



static unsigned int reqCount;
static char* req;
static Out out = {
        0,
        nullptr
};
static char* currentOutPos = nullptr;

Out packADC(std::filesystem::path path, Json& config);
CompressedRLE* compressRLE(const CombiscopeHistogram* uncompressed, const int size);
CompressedHoff compressHoffman(const CompressedRLE* uncompressed);

//const static size_t chMap[16] = {0, 8, 1, 9, 2, 10, 3, 11, 5, 13, 4, 12, 7, 15, 6, 14};
const static size_t chMap[16] = {0, 2, 4, 6, 10, 8, 14, 12, 1, 3, 5, 7, 11, 9, 15, 13};

const char units[128] = "V";
static LongFlip flip;
const static int type = 0;
void innerFreeOut();
const static double timeCorrectionMult = 1.0125;

static CombiscopeHistogram header = {
        0,
        "Signal name",
        "comment",
        "Units",
        Time{
                2022,
                12,
                16,
                3,
                15,
                16,
                12,
                33
        },
        0,//pointCount,//nPoints
        0,//t_min
        1.0,//tMax
        0,//baseline shift
        10.0 / 65536.0//delta DAC to VOlt
};

template <typename TP> std::time_t to_time_t(TP tp){
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                                                        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

#endif //SHTRIPPER_COMPRESS_H
