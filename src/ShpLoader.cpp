/*
 * ShpLoader.cpp
 *
 *  Created on: 02.08.2018
 *      Author: marek
 */

#include <iostream>
#include <fstream>
#include <cstring>

#include "ShpLoader.h"

ShpLoader::ShpLoader(const char *path)
{
    file.open(path, ios::binary);
    if(!file.is_open())
    {
        cerr << "Can't open file: " << endl;
        return;
    }

    file.read((char *) &imgCount, sizeof(imgCount));
    file.seekg(4, std::ios::cur);
    file.read((char *) &width, sizeof(width));
    file.read((char *) &height, sizeof(height));

    if(width == 0 || height == 0)
    {
        return;
    }
    file.seekg(4, std::ios::cur);

    for(int i = 0; i < imgCount; i++)
    {
        images.push_back(ShpImage(file));
    }
    file.seekg(16, std::ios::cur);

    for(int i = 0; i < images.size(); i++)
    {
        ShpImage &img = images[i];
        if(img.format == Format::XORPrev)
        {
            img.refImage = &images[i - 1];
        }
        else if(img.format == Format::XORLCW)
        {
            for(auto &im : images)
            {
                if(img.refOffset== im.offset)
                {
                    img.refImage = &im;
                    break;
                }
            }
        }
        decompress(img);
    }

    uint8_t dd[24*24];
    memcpy(dd, images[0].data.get(), sizeof(dd));
    file.close();
}

ShpLoader::~ShpLoader()
{
}

ShpLoader::ShpImage::ShpImage(ifstream &file)
{
    file.read((char *) &offset, sizeof(offset));
    format = (Format) (offset >> 24);
    offset &= 0xFFFFFF;

    file.read((char *) &refOffset, sizeof(refOffset));
    file.read((char *) &refFormat, 2);
}

ShpLoader::ShpImage::ShpImage(ShpImage &&obj)
{
    format = obj.format;
    offset = obj.offset;
    refFormat = obj.refFormat;
    refOffset = obj.refOffset;
    refImage = obj.refImage;
    data = move(obj.data);
}

ShpLoader::ShpImage::~ShpImage()
{
}

void ShpLoader::decompress(ShpImage &img)
{
    switch(img.format)
    {
        case Format::XORPrev:
        case Format::XORLCW:
            if(!img.refImage->data)
            {
                ++recurseDepth;
                decompress(*img.refImage);
                --recurseDepth;
            }
            img.data = make_unique<char[]>(width * height);
            memcpy(img.data.get(), img.refImage->data.get(), width * height);
            xorDeltaDecode(img);
            break;
        case Format::LCW:
            img.data = make_unique<char[]>(width * height);
            lcwDecode(img);
            break;
        default:
            break;
    }
}

int ShpLoader::lcwDecode(ShpImage &img)
{
    uint8_t i;
    int destIndex = 0;
    uint16_t count;
    uint8_t t;
    uint16_t srcIndex;
    char *data = img.data.get();
    while(true)
    {
        file.read((char *) &i, 1);
        if((i & 0x80) == 0)
        {
            // case 2
            uint8_t secondByte;
            file.read((char *) &secondByte, 1);
            count = ((i & 0x70) >> 4) + 3;
            uint16_t rpos = ((i & 0xf) << 8) + secondByte;

            if(destIndex + count > width * height)
            {
                return destIndex;
            }

            int srcIndex = destIndex - rpos;
            if(destIndex - srcIndex == 1)
            {
                for(int i = 0; i < count; i++)
                {
                    data[destIndex + i] = data[destIndex - 1];
                }
            }
            else
            {
                for(int i = 0; i < count; i++)
                {
                    data[destIndex + i] = data[srcIndex + i];
                }
            }

            destIndex += count;
        }
        else if((i & 0x40) == 0)
        {
            // case 1
            count = i & 0x3F;
            if(count == 0)
            {
                return destIndex;
            }

            file.read(data + destIndex, count);
            destIndex += count;
        }
        else
        {
            uint8_t count3 = i & 0x3F;
            if(count3 == 0x3E)
            {
                // case 4
                uint8_t color;
                file.read((char *) &t, 1);
                file.read((char *) &count, 1);
                count = t | (count << 8);

                file.read((char *) &color, 1);

                for(uint16_t end = destIndex + count; destIndex < end; destIndex++)
                {
                    data[destIndex] = color;
                }
            }
            else if(count3 == 0x3F)
            {
                // case 5
                file.read((char *) &t, 1);
                file.read((char *) &count, 1);
                count = t | (count << 8);
                file.read((char *) &t, 1);
                file.read((char *) &srcIndex, 1);
                srcIndex = t | (srcIndex << 8);

                for(uint16_t end = destIndex + count; destIndex < end; destIndex++)
                {
                    data[destIndex] = data[srcIndex++];
                }
            }
            else
            {
                // case 3
                count = count3 + 3;
                file.read((char *) &t, 1);
                file.read((char *) &srcIndex, 1);
                srcIndex = t | (srcIndex << 8);

                for(uint16_t end = destIndex + count; destIndex < end; destIndex++)
                {
                    data[destIndex] = data[srcIndex++];
                }
            }
        }
    }
}


int ShpLoader::xorDeltaDecode(ShpImage &img)
{
    int destIndex = 0;
    uint8_t i, count, value, t;
    char *data = img.data.get();
    while(true)
    {
        file.read((char *) &i, 1);
        if((i & 0x80) == 0)
        {
            count = i & 0x7F;
            if(count == 0)
            {
                // case 6
                file.read((char *) &count, 1);
                file.read((char *) &value, 1);
                for(uint16_t end = destIndex + count; destIndex < end; destIndex++)
                {
                    data[destIndex] ^= value;
                }
            }
            else
            {
                // case 5
                for(uint16_t end = destIndex + count; destIndex < end; destIndex++)
                {
                    file.read((char *) &value, 1);
                    data[destIndex] ^= value;
                }
            }
        }
        else
        {
            count = i & 0x7F;
            if(count == 0)
            {
                file.read((char *) &t, 1);
                file.read((char *) &count, 1);
                if(count == 0)
                {
                    return destIndex;
                }

                if((count & 0x8000) == 0)
                {
                    // case 2
                    destIndex += count & 0x7FFF;
                }
                else if((count & 0x4000) == 0)
                {
                    // case 3
                    for(uint16_t end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
                    {
                        file.read((char *) &value, 1);
                        data[destIndex] ^= value;
                    }
                }
                else
                {
                    // case 4
                    file.read((char *) &value, 1);
                    for(uint16_t end = destIndex + (count & 0x3FFF); destIndex < end; destIndex++)
                    {
                        data[destIndex] ^= value;
                    }
                }
            }
            else
            {
                // case 1
                destIndex += count;
            }
        }
    }
}
