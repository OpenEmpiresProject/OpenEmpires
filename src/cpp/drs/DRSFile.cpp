#include "DRSFile.h"

#include <iostream>
#include <cassert>
#include <algorithm>

using namespace drs;
using namespace std;

#pragma pack(push, 1)
struct drs_header
{
    char copyright[40];
    char version[4];
    char ftype[12];
    int32_t table_count;
    int32_t file_offset;
};

struct drs_table_info {
	char file_extension[4];
	int32_t file_info_offset; // Absolute position from DRS file start
	int32_t num_files;
};

struct drs_file_info
{
    int32_t file_id;
    int32_t file_data_offset; // Absolute position from DRS file start
    int32_t file_size;
};
#pragma pack(pop)


bool DRSFile::load(const std::string& filename) {
    file.open(filename, std::ios::binary);
    if (!file) return false;

    drs_header header;

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!file)
    {
        throw std::runtime_error("Failed to read DRS header.");
    }

    for (size_t i = 0; i < header.table_count; i++)
    {
        drs_table_info tableInfo;
        file.read(reinterpret_cast<char*>(&tableInfo), sizeof(tableInfo));
        if (!file)
        {
            throw std::runtime_error("Failed to read table info.");
        }

        for (size_t i = 0; i < tableInfo.num_files; i++)
        {
            drs_file_info fileInfo;
            file.read(reinterpret_cast<char*>(&fileInfo), sizeof(fileInfo));
            if (!file)
            {
                throw std::runtime_error("Failed to file info.");
            }
            // cout << "File ID: " << fileInfo.id << ", Offset: " << fileInfo.offset
            //      << ", Size: " << fileInfo.size << endl;

            DRSResourceEntry entry;
            entry.id = fileInfo.file_id;
            entry.offset = fileInfo.file_data_offset;
            entry.size = fileInfo.file_size;
            m_resources[fileInfo.file_id] = entry;
        }
    }
    return true;
}

DRSResourceData DRSFile::getResource(int resourceId)
{
    auto it = m_resources.find(resourceId);
    if (it == m_resources.end()) throw std::runtime_error("Resource not found");

    const DRSResourceEntry& entry = it->second;

    assert(entry.size > 0 && "Resource size is zero");

    DRSResourceData resourceData;
    resourceData.entry = entry;
    resourceData.data = new uint8_t[entry.size];

    file.seekg(entry.offset);
    file.read(reinterpret_cast<char*>(resourceData.data), entry.size);

    return resourceData;
}

SLPFile drs::DRSFile::getSLPFile(int resourceId)
{
    return SLPFile(getResource(resourceId));
}

std::vector<int> DRSFile::listResources() const
{
    std::vector<int> ids;
    for (const auto& kv : m_resources) ids.push_back(kv.first);

    std::sort(ids.begin(), ids.end());
    return ids;
}