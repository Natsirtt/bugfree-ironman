#include "file.hpp"

#include <cmath>

File::File() : mName(""), mSize(0), mPartitionsNb(0), mPartitionSize(0) {
}

File::File(std::string name, long long size, int partitionSize) : mName(name), mSize(size), mPartitionSize(partitionSize) {
    mPartitionsNb = std::max(size / partitionSize, 1ll);
    if (size % partitionSize != 0) {
        mPartitionsNb++;
    }
}

std::string File::getName() {
    return mName;
}

long long File::getSize() {
    return mSize;
}

int File::getPartitionSize() {
    return mPartitionSize;
}

int File::getLastPartitionSize() {
    int size = mSize % mPartitionSize;
    if (size == 0) {
        return mPartitionSize;
    }
    return size;
}

int File::getPartitionsNb() {
    return mPartitionsNb;
}

std::string File::getKey() {
    return mName;
}
