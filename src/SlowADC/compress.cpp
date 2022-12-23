#include "SlowADC/compress.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

#include <iostream>

CompressedRLE* compressRLE(const CombiscopeHistogram* uncompressed, const int size){
    auto* data = (unsigned char*) uncompressed;

    unsigned char counter;
    int i = 0;
    int NBytes = 0;
    while (i < size){
        if(i != size - 1 && data[i] == data[i+1]){
            counter = 2;
            i += 2;
            while((i < size) &&
                    (data[i] == data[i - 1]) &&
                    (counter < 127)){
                i++;
                counter++;
            }
            NBytes += 2;
        }else{
            counter=1;
            while (true){
                i++;
                if (i >= size)
                    break;
                if (i < size - 1){
                    if (data[i] == data[i + 1])
                        break;
                }
                if (counter == 127)
                    break;
                counter++;
            }
            NBytes += counter + 1;
        }
    }

    auto* compressed = new CompressedRLE(NBytes);

    i = 0;
    NBytes = 0;
    while (i < size){
        if(i != size - 1 && data[i] == data[i + 1]){
            counter = 2;
            compressed->data[NBytes + 1] = data[i];
            i += 2;
            while((i < size) &&
                    (data[i] == data[i - 1]) &&
                    (counter < 127)){
                i++;
                counter++;
            }
            compressed->data[NBytes] = counter;
            NBytes += 2;
        }else{
            counter = 1;
            compressed->data[NBytes + counter] = data[i];
            while(true){
                i++;
                if(i >= size)
                    break;
                if (i < size - 1){
                    if(data[i] == data[i + 1])
                        break;
                }
                if(counter == 127)
                    break;
                counter++;
                compressed->data[NBytes + counter] = data[i];
            }
            compressed->data[NBytes] = counter | 128;
            NBytes += counter + 1;
        }
    }

    return compressed;
}

void CreateCode(const unsigned char *table, Code *code){
    bool bits[256];
    unsigned short mask[256];
    for(unsigned short & entry : mask){
        entry = 512;
    }

    unsigned char vertex;
    unsigned short prevVertex;
    unsigned char bit, byteMask;
    unsigned int byte;

    for(int i = 0; i < 256; i++){
        code[i].NBits = 0;
        if (table[i] != 255){
            prevVertex = i;
            vertex=table[i];
            while (vertex != 255){
                if (mask[vertex] == 512){
                    mask[vertex] = prevVertex;
                }
                bits[code[i].NBits] = mask[vertex] != prevVertex;
                code[i].NBits++;
                prevVertex = vertex + 256;
                vertex = table[prevVertex];
            }

            for (int j = 0; j < code[i].NBits; j++){
                byte = (code[i].NBits - 1 - j) / 8;
                bit = (code[i].NBits - 1 - j) % 8;
                byteMask =~ (1 << bit);
                code[i].Bits[byte] = code[i].Bits[byte] & byteMask;
                code[i].Bits[byte] = code[i].Bits[byte] | (bits[j] << bit);
            }
        }
    }
}

int CompressedSize(const CompressedRLE* uncompressed, const Code *Code){
    int size = 0;
    for(int i = 0; i < uncompressed->size; i++){
        size += Code[uncompressed->data[i]].NBits;
    }
    return (size + 8) / 8;
}

void CompressHoffman(const CompressedRLE* uncompressed, const Code *code, unsigned char *outBuff){
    int index = 0;
    unsigned char bit, mask;
    unsigned int byte;
    for (int i = 0; i < uncompressed->size; i++){
        for (int j = 0; j < code[uncompressed->data[i]].NBits; j++){
            byte = j / 8;
            bit = j % 8;
            bool b = (code[uncompressed->data[i]].Bits[byte] & (1 << bit)) != 0;

            byte = index / 8;
            bit = index % 8;
            mask =~ (1 << bit);
            outBuff[byte] = outBuff[byte] & mask;
            outBuff[byte] = outBuff[byte] | (b << bit);

            index++;
        }
    }
}

void Sort(Knot **pKnot, const int left, const int right){
    int i, j;
    Knot *comp;
    Knot *value;
    i = left;
    j = right;
    comp = pKnot[(left + right) / 2];
    while (i <= j){
        while ((pKnot[i]->weight > comp->weight) && (i < right))
            i++;
        while ((pKnot[j]->weight) < comp->weight && (j > left))
            j--;
        if (i <= j){
            value = pKnot[i];
            pKnot[i] = pKnot[j];
            pKnot[j] = value;
            i++;
            j--;
        }
    }
    if(left < j)
        Sort(pKnot, left, j);
    if(i < right)
        Sort(pKnot, i, right);
}

void CreateTable(const CompressedRLE* uncompressed,	unsigned char *Table){
    int i;
    Knot knots[511], *pKnot[256];

    for(i = 0; i < uncompressed->size; i++){
        knots[uncompressed->data[i]].weight++;
    }

    int NKnots = 0;
    int NSymbols = 0;

    for(i = 0; i < 256; i++){
        if (knots[i].weight > 0){
            pKnot[NKnots] = &knots[i];
            NKnots++;
            NSymbols++;
        }
    }

    for(i = 0; i < NSymbols - 1; i++){
        Sort(pKnot,0,NKnots-1);
        knots[256 + i].weight = pKnot[NKnots - 2]->weight + pKnot[NKnots - 1]->weight;
        pKnot[NKnots - 2]->parent = i;
        pKnot[NKnots - 1]->parent = i;
        pKnot[NKnots - 2] = &knots[256 + i];
        NKnots--;
    }

    for(i = 0; i < 511; i++){
        Table[i] = (unsigned char)(knots[i].parent);
    }
}

CompressedHoff compressHoffman(const CompressedRLE* uncompressed){
    unsigned char table[511];

    Code code[256];
    CreateTable(uncompressed, table);
    CreateCode(table, code);
    int cSize = CompressedSize(uncompressed, code);

    auto *outBuff = new unsigned char [cSize + 511 + sizeof(int)];
    std::memcpy(outBuff, table, 511);
    std::memcpy(outBuff + 511, &uncompressed->size, sizeof(int));

    CompressHoffman(uncompressed, code, outBuff + 511 + sizeof(int));

    return CompressedHoff{
            (char*) outBuff,
            (cSize + 511 + (int)sizeof(int))
    };
}

Out packADC(std::filesystem::path path, Json& config){
    size_t boardCount = config["slow"].size();
    const int signalCount = CH_COUNT * boardCount; // 16 ch 4 boards
    size_t globalSignalIndex = 0;
    auto outQueue = new Out[signalCount];
    size_t totalSize = sizeof(V2) + sizeof(int);

    innerFreeOut();

    for(size_t boardInd = 0; boardInd < boardCount; boardInd++) {
        std::filesystem::path currentFile = path;

        currentFile += '/' + (std::string)config["slow"][boardInd]["ip"] + ".slow";

        char *inBuffer;
        size_t length = 0;
        std::ifstream inFile;
        inFile.open(currentFile, std::ios::in | std::ios::binary | std::ios::ate);
        if (inFile.is_open()) {
            length = inFile.tellg();
            inBuffer = new char[length];
            inFile.seekg(0, std::ios::beg);
            inFile.read(inBuffer, length);
            inFile.close();
        }else{
            return out;
        }

        unsigned long pointCount = length / (sizeof(short) * CH_COUNT);
        //std::cout << length << ' ' << length * 2e-6 << std::endl;
        header.nPoints = pointCount;
        header.tMax = (double) pointCount * 2e-6 * timeCorrectionMult;

        std::filesystem::file_time_type time = std::filesystem::last_write_time(currentFile);
        time_t t_time_t = to_time_t(time);
        auto t_struct = localtime(&t_time_t);
        header.time = {
                (unsigned short)(t_struct->tm_year + 1900),
                (unsigned short)(t_struct->tm_mon + 1),
                (unsigned short)(t_struct->tm_mday),
                (unsigned short)(t_struct->tm_wday + 1),
                (unsigned short)(t_struct->tm_hour),
                (unsigned short)(t_struct->tm_min),
                (unsigned short)(t_struct->tm_sec),
                0
        };

        size_t signalSize = sizeof(CombiscopeHistogram) - 8 + pointCount * sizeof(double);
        for (int signalIndex = 0; signalIndex < CH_COUNT; signalIndex++) {
            std::string name = "";
            if(config["slow"][boardInd]["channels"][signalIndex].contains("sync")){
                name = "Synchronisation";
            }else if(config["slow"][boardInd]["channels"][signalIndex].contains("nc")){
                continue;
            }else{
                name = "R=" + std::to_string((int)config["fibers"][config["poly"][(int)config["slow"][boardInd]["channels"][signalIndex]["poly"]]["fiber"]]["R"] / 10) +
                       ", Poly " + to_string(config["slow"][boardInd]["channels"][signalIndex]["poly"]) +
                       ", ch " + to_string(config["slow"][boardInd]["channels"][signalIndex]["ch"]) +
                       ", G=" + to_string(config["poly"][(int)config["slow"][boardInd]["channels"][signalIndex]["poly"]]["channels"][(int)config["slow"][boardInd]["channels"][signalIndex]["ch"]]["slow_gain"]);

            }

            auto *buffer = new unsigned char[signalSize];
            std::memcpy(buffer, &type, sizeof(int));
            std::memcpy(buffer + sizeof(int) + 128 * 3, (char *)&header + sizeof(int) + 128 * 3, sizeof(Time) + sizeof(int) + 4 * sizeof(double));

            std::memcpy(buffer + sizeof(int), name.c_str(), name.length());

            std::string comment = "ADC ";
            comment.append(std::to_string(boardInd));
            comment.append(", Ch ");
            comment.append(std::to_string(signalIndex));
            std::memcpy(buffer + sizeof(int) + 128, comment.c_str(), comment.length());
            std::memcpy(buffer + sizeof(int) + 256, units, 128);
            auto *buffPosition = buffer + sizeof(CombiscopeHistogram) - 8;

            for (size_t i = 0; i < pointCount; i++) {
                flip.asLong = *(short *) &inBuffer[CH_COUNT * i * sizeof(short) + chMap[signalIndex] * sizeof(short)];
                std::memcpy(buffPosition + i, &flip.asChar[0], sizeof(char));
                std::memcpy(buffPosition + i + pointCount, &flip.asChar[1], sizeof(char));
                std::memcpy(buffPosition + i + pointCount * 2, &flip.asChar[2], sizeof(char));
                std::memcpy(buffPosition + i + pointCount * 3, &flip.asChar[3], sizeof(char));
            }

            CompressedRLE *rle = compressRLE((CombiscopeHistogram *) buffer, signalSize);

            CompressedHoff packed = compressHoffman(rle);
            delete rle;

            totalSize += packed.size + sizeof(int);
            outQueue[globalSignalIndex] = Out{
                    packed.size,
                    const_cast<char *>(packed.data)
            };
            delete[] buffer;
            globalSignalIndex++;
        }
        delete[] inBuffer;
    }
    out.point = new char[totalSize];
    std::memcpy(out.point, V2, sizeof(V2));
    out.size += sizeof(V2);
    std::memcpy(out.point + out.size, &globalSignalIndex, sizeof(int));
    out.size += sizeof(int);
    for (size_t signalIndex = 0; signalIndex < globalSignalIndex; signalIndex++) {
        std::memcpy(out.point + out.size, &outQueue[signalIndex].size, sizeof(int));
        out.size += sizeof(int);
        std::memcpy(out.point + out.size, outQueue[signalIndex].point, outQueue[signalIndex].size);
        out.size += outQueue[signalIndex].size;

        delete[] outQueue[signalIndex].point;
    }
    delete[] outQueue;
    return out;
}

void innerFreeOut(){
    delete[] out.point;
    out.size = 0;
    currentOutPos = nullptr;
    reqCount = 0;
    req = nullptr;
}