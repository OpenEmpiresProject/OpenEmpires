#include "DRSFile.h"

#include "internal/DRSResourceEntry.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>

using namespace drs;
using namespace std;

#pragma pack(push, 1)
struct DRSHeader
{
    char copyright[40];
    char version[4];
    char ftype[12];
    int32_t tableCount;
    int32_t fileOffset;
};

struct DRSTableInfo
{
    char fileExtension[4];
    int32_t fileInfoOffset; // Absolute position from DRS m_file start
    int32_t numOfFiles;
};

struct DRSFileInfo
{
    int32_t fileId;
    int32_t fileDataOffset; // Absolute position from DRS m_file start
    int32_t fileSize;
};
#pragma pack(pop)

bool DRSFile::load(const std::string& filename)
{
    m_file.open(filename, std::ios::binary);
    if (!m_file)
        return false;

    DRSHeader header;

    m_file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!m_file)
    {
        throw std::runtime_error("Failed to read DRS header.");
    }

    for (size_t i = 0; i < header.tableCount; i++)
    {
        DRSTableInfo tableInfo;
        m_file.read(reinterpret_cast<char*>(&tableInfo), sizeof(tableInfo));
        if (!m_file)
        {
            throw std::runtime_error("Failed to read table info.");
        }

        for (size_t i = 0; i < tableInfo.numOfFiles; i++)
        {
            DRSFileInfo fileInfo;
            m_file.read(reinterpret_cast<char*>(&fileInfo), sizeof(fileInfo));
            if (!m_file)
            {
                throw std::runtime_error("Failed to m_file info.");
            }
            auto resourceData = make_shared<DRSResourceData>();

            resourceData->entry.id = fileInfo.fileId;
            resourceData->entry.offset = fileInfo.fileDataOffset;
            resourceData->entry.size = fileInfo.fileSize;
            m_resources[fileInfo.fileId] = std::move(resourceData);
        }
    }
    return true;
}

std::shared_ptr<DRSResourceData> DRSFile::getResource(uint32_t resourceId)
{
    auto it = m_resources.find(resourceId);
    if (it == m_resources.end())
        throw std::runtime_error("Resource not found");

    auto& resourceData = it->second;

    assert(resourceData->entry.size > 0 && "Resource size is zero");

    if (resourceData->data != nullptr)
    {
        return resourceData;
    }
    resourceData->data = new uint8_t[resourceData->entry.size];

    m_file.seekg(resourceData->entry.offset);
    m_file.read(reinterpret_cast<char*>(resourceData->data), resourceData->entry.size);

    return resourceData;
}

SLPFile drs::DRSFile::getSLPFile(uint32_t resourceId)
{
    auto resourceData = getResource(resourceId);

    return SLPFile(resourceId,
                   std::span<const uint8_t>(resourceData->data, resourceData->entry.size));
}

std::vector<uint32_t> DRSFile::listResources() const
{
    std::vector<uint32_t> ids;
    for (const auto& kv : m_resources)
        ids.push_back(kv.first);

    std::sort(ids.begin(), ids.end());
    return ids;
}