#include "serialize.h"
#include "hash.h"

template<typename Stream>
void WriteCompactSize(Stream& os, uint64_t nSize)
{
    if (nSize < 0xfd)
    {
        auto chSize = (uint8_t)nSize;
        WRITEDATA(os, chSize);
    }
    else if (nSize <= std::numeric_limits<uint16_t>::max())
    {
        uint8_t chSize = 0xfd;
        auto xSize = (uint16_t)nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    else if (nSize <= std::numeric_limits<uint32_t>::max())
    {
        uint8_t chSize = 0xfe;
        auto xSize = (uint32_t)nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    else
    {
        uint8_t chSize = 0xff;
        auto xSize = nSize;
        WRITEDATA(os, chSize);
        WRITEDATA(os, xSize);
    }
    return;
}


template<typename Stream>
uint64_t ReadCompactSize(Stream& is)
{
    uint8_t chSize;
    READDATA(is, chSize);
    uint64_t nSizeRet = 0;
    if (chSize < 0xfd)
    {
        nSizeRet = chSize;
    }
    else if (chSize == 0xfd)
    {
        uint16_t xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    else if (chSize == 0xfe)
    {
        uint32_t xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    else
    {
        uint64_t xSize;
        READDATA(is, xSize);
        nSizeRet = xSize;
    }
    if (nSizeRet > (uint64_t)MAX_SIZE)
        throw std::ios_base::failure("ReadCompactSize() : size too large");
    return nSizeRet;
}



// Template instantiation

template void WriteCompactSize<CAutoFile>(CAutoFile&, uint64_t);
template void WriteCompactSize<CDataStream>(CDataStream&, uint64_t);
template void WriteCompactSize<CHashWriter>(CHashWriter&, uint64_t);

template uint64_t ReadCompactSize<CAutoFile>(CAutoFile&);
template uint64_t ReadCompactSize<CDataStream>(CDataStream&);

//
//CDataStream
//


bool CDataStream::Rewind(size_type n)
{
    // Rewind by n characters if the buffer hasn't been compacted yet
    if (n > nReadPos)
        return false;
    nReadPos -= (unsigned int)n;
    return true;
}

void CDataStream::setstate(short bits, const char* psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

CDataStream& CDataStream::read(char* pch, int nSize)
{
    // Read from the beginning of the buffer
    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
        if (nReadPosNext > vch.size())
        {
            setstate(std::ios::failbit, "CDataStream::read() : end of data");
            memset(pch, 0, nSize);
            nSize = (int)(vch.size() - nReadPos);
        }
        memcpy(pch, &vch[nReadPos], nSize);
        nReadPos = 0;
        vch.clear();
        return (*this);
    }
    memcpy(pch, &vch[nReadPos], nSize);
    nReadPos = nReadPosNext;
    return (*this);
}

CDataStream& CDataStream::ignore(int nSize)
{
    // Ignore from the beginning of the buffer
    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
        if (nReadPosNext > vch.size())
            setstate(std::ios::failbit, "CDataStream::ignore() : end of data");
        nReadPos = 0;
        vch.clear();
        return (*this);
    }
    nReadPos = nReadPosNext;
    return (*this);
}

CDataStream& CDataStream::write(const char* pch, int nSize)
{
    // Write to the end of the buffer
    assert(nSize >= 0);
    vch.insert(vch.end(), pch, pch + nSize);
    return (*this);
}

void CDataStream::GetAndClear(CSerializeData &data) {
    vch.swap(data);
    CSerializeData().swap(vch);
}

//
//CAutoFile
//

CAutoFile::CAutoFile(FILE* filenew, int nTypeIn, int nVersionIn)
{
    file = filenew;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = std::ios::badbit | std::ios::failbit;
}

CAutoFile::~CAutoFile()
{
    fclose();
}

void CAutoFile::fclose()
{
    if (file != NULL && file != stdin && file != stdout && file != stderr)
        ::fclose(file);
    file = NULL;
}

void CAutoFile::setstate(short bits, const char* psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

CAutoFile& CAutoFile::read(char* pch, size_t nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::read : file handle is NULL");
    if (fread(pch, 1, nSize, file) != nSize)
        setstate(std::ios::failbit, feof(file) ? "CAutoFile::read : end of file" : "CAutoFile::read : fread failed");
    return (*this);
}

CAutoFile& CAutoFile::write(const char* pch, size_t nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::write : file handle is NULL");
    if (fwrite(pch, 1, nSize, file) != nSize)
        setstate(std::ios::failbit, "CAutoFile::write : write failed");
    return (*this);
}

//
//CBufferedFile
//

void CBufferedFile::setstate(short bits, const char *psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

bool CBufferedFile::Fill()
{
    auto pos = (uint32_t)(nSrcPos % vchBuf.size());
    auto readNow = (uint32_t)(vchBuf.size() - pos);
    auto nAvail = (uint32_t)(vchBuf.size() - (nSrcPos - nReadPos) - nRewind);
    if (nAvail < readNow)
        readNow = nAvail;
    if (readNow == 0)
        return false;
    size_t read = fread((void*)&vchBuf[pos], 1, readNow, src);
    if (read == 0) {
        setstate(std::ios_base::failbit, feof(src) ? "CBufferedFile::Fill : end of file" : "CBufferedFile::Fill : fread failed");
        return false;
    } else {
        nSrcPos += read;
        return true;
    }
}

CBufferedFile::CBufferedFile(FILE *fileIn, uint64_t nBufSize, uint64_t nRewindIn, int nTypeIn, int nVersionIn) :
    src(fileIn), nSrcPos(0), nReadPos(0), nReadLimit(std::numeric_limits<uint64_t>::max()), nRewind(nRewindIn), vchBuf(nBufSize, 0),
    state(0), exceptmask(std::ios_base::badbit | std::ios_base::failbit), nType(nTypeIn), nVersion(nVersionIn) { }

bool CBufferedFile::good() const
{
    return state == 0;
}

bool CBufferedFile::eof() const
{
    return nReadPos == nSrcPos && feof(src);
}

CBufferedFile& CBufferedFile::read(char *pch, size_t nSize)
{
    if (nSize + nReadPos > nReadLimit)
        throw std::ios_base::failure("Read attempted past buffer limit");
    if (nSize + nRewind > vchBuf.size())
        throw std::ios_base::failure("Read larger than buffer size");
    while (nSize > 0) {
        if (nReadPos == nSrcPos)
            Fill();
        auto pos = (uint32_t)(nReadPos % vchBuf.size());
        auto nNow = nSize;
        if (nNow + pos > vchBuf.size())
            nNow = vchBuf.size() - pos;
        if (nNow + nReadPos > nSrcPos)
            nNow = (size_t)(nSrcPos - nReadPos);
        memcpy(pch, &vchBuf[pos], nNow);
        nReadPos += nNow;
        pch += nNow;
        nSize -= nNow;
    }
    return (*this);
}

uint64_t CBufferedFile::GetPos()
{
    return nReadPos;
}

bool CBufferedFile::SetPos(uint64_t nPos)
{
    nReadPos = nPos;
    if (nReadPos + nRewind < nSrcPos) {
        nReadPos = nSrcPos - nRewind;
        return false;
    } else if (nReadPos > nSrcPos) {
        nReadPos = nSrcPos;
        return false;
    } else {
        return true;
    }
}

bool CBufferedFile::Seek(uint64_t nPos)
{
    long nLongPos = (long)nPos;
    if (nPos != (uint64_t)nLongPos)
        return false;
    if (fseek(src, nLongPos, SEEK_SET))
        return false;
    nLongPos = ftell(src);
    nSrcPos = nLongPos;
    nReadPos = nLongPos;
    state = 0;
    return true;
}

bool CBufferedFile::SetLimit(uint64_t nPos)
{
    if (nPos < nReadPos)
        return false;
    nReadLimit = nPos;
    return true;
}

void CBufferedFile::FindByte(char ch)
{
    for ( ; ; ) {
        if (nReadPos == nSrcPos)
            Fill();
        if (vchBuf[nReadPos % vchBuf.size()] == ch)
            break;
        nReadPos++;
    }
}
