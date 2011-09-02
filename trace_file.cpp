/**************************************************************************
 *
 * Copyright 2011 Zack Rusin
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include "trace_file.hpp"

#include "trace_snappyfile.hpp"

#include <assert.h>
#include <string.h>

#include <zlib.h>

#include "os.hpp"

#include <iostream>

using namespace Trace;


File::File(const std::string &filename,
           File::Mode mode)
    : m_filename(filename),
      m_mode(mode),
      m_isOpened(false)
{
    if (!m_filename.empty()) {
        open(m_filename, m_mode);
    }
}

File::~File()
{
    close();
}


File::Offset File::currentOffset()
{
    return File::Offset();
}

void File::setCurrentOffset(const File::Offset &offset)
{
}

bool File::isZLibCompressed(const std::string &filename)
{
    std::fstream stream(filename.c_str(),
                        std::fstream::binary | std::fstream::in);
    if (!stream.is_open())
        return false;

    unsigned char byte1, byte2;
    stream >> byte1;
    stream >> byte2;
    stream.close();

    return (byte1 == 0x1f && byte2 == 0x8b);
}


bool File::isSnappyCompressed(const std::string &filename)
{
    std::fstream stream(filename.c_str(),
                        std::fstream::binary | std::fstream::in);
    if (!stream.is_open())
        return false;

    unsigned char byte1, byte2;
    stream >> byte1;
    stream >> byte2;
    stream.close();

    return (byte1 == SNAPPY_BYTE1 && byte2 == SNAPPY_BYTE2);
}


ZLibFile::ZLibFile(const std::string &filename,
                   File::Mode mode)
    : File(filename, mode),
      m_gzFile(NULL)
{
}

ZLibFile::~ZLibFile()
{
}

bool ZLibFile::rawOpen(const std::string &filename, File::Mode mode)
{
    m_gzFile = gzopen(filename.c_str(),
                      (mode == File::Write) ? "wb" : "rb");
    return m_gzFile != NULL;
}

bool ZLibFile::rawWrite(const void *buffer, size_t length)
{
    return gzwrite(m_gzFile, buffer, length) != -1;
}

bool ZLibFile::rawRead(void *buffer, size_t length)
{
    return gzread(m_gzFile, buffer, length) != -1;
}

int ZLibFile::rawGetc()
{
    return gzgetc(m_gzFile);
}

void ZLibFile::rawClose()
{
    if (m_gzFile) {
        gzclose(m_gzFile);
        m_gzFile = NULL;
    }
}

void ZLibFile::rawFlush()
{
    gzflush(m_gzFile, Z_SYNC_FLUSH);
}


bool ZLibFile::supportsOffsets() const
{
    return false;
}

bool ZLibFile::rawSkip(unsigned)
{
    return false;
}
