#include "streams.h"

CDataStream::CDataStream(int nTypeIn, int nVersionIn)
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const_iterator pbegin, const_iterator pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const char *pbegin, const char *pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const vector_type &vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const std::vector<char> &vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
{
    Init(nTypeIn, nVersionIn);
}

CDataStream::CDataStream(const std::vector<unsigned char> &vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
{
    Init(nTypeIn, nVersionIn);
}

void CDataStream::Init(int nTypeIn, int nVersionIn)
{
    nReadPos = 0;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = std::ios::badbit | std::ios::failbit;
}

CDataStream &CDataStream::operator+=(const CDataStream &b)
{
    vch.insert(vch.end(), b.begin(), b.end());
    return *this;
}

std::string CDataStream::str() const
{
    return std::string(begin(), end());
}

bool CDataStream::Rewind(size_type n)
{
    // Rewind by n characters if the buffer hasn't been compacted yet
    if (n > nReadPos)
        return false;
    nReadPos -= (unsigned int)n;
    return true;
}

void CDataStream::setstate(short bits, const char *psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

CDataStream &CDataStream::read(char *pch, int nSize)
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

CDataStream &CDataStream::ignore(int nSize)
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

CDataStream &CDataStream::write(const char *pch, int nSize)
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

CAutoFile::CAutoFile(FILE *filenew, int nTypeIn, int nVersionIn)
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

void CAutoFile::setstate(short bits, const char *psz)
{
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

CAutoFile &CAutoFile::read(char *pch, size_t nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::read : file handle is NULL");
    if (fread(pch, 1, nSize, file) != nSize)
        setstate(std::ios::failbit, feof(file) ? "CAutoFile::read : end of file" : "CAutoFile::read : fread failed");
    return (*this);
}

CAutoFile &CAutoFile::write(const char *pch, size_t nSize)
{
    if (!file)
        throw std::ios_base::failure("CAutoFile::write : file handle is NULL");
    if (fwrite(pch, 1, nSize, file) != nSize)
        setstate(std::ios::failbit, "CAutoFile::write : write failed");
    return (*this);
}

void CBufferedFile::setstate(short bits, const char *psz) {
    state |= bits;
    if (state & exceptmask)
        throw std::ios_base::failure(psz);
}

bool CBufferedFile::Fill() {
    unsigned int pos = (unsigned int)(nSrcPos % vchBuf.size());
    unsigned int readNow = (unsigned int)(vchBuf.size() - pos);
    unsigned int nAvail = (unsigned int)(vchBuf.size() - (nSrcPos - nReadPos) - nRewind);
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

CBufferedFile &CBufferedFile::read(char *pch, size_t nSize) {
    if (nSize + nReadPos > nReadLimit)
        throw std::ios_base::failure("Read attempted past buffer limit");
    if (nSize + nRewind > vchBuf.size())
        throw std::ios_base::failure("Read larger than buffer size");
    while (nSize > 0) {
        if (nReadPos == nSrcPos)
            Fill();
        unsigned int pos = (unsigned int)(nReadPos % vchBuf.size());
        size_t nNow = nSize;
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

bool CBufferedFile::SetPos(uint64_t nPos) {
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

bool CBufferedFile::Seek(uint64_t nPos) {
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

bool CBufferedFile::SetLimit(uint64_t nPos) {
    if (nPos < nReadPos)
        return false;
    nReadLimit = nPos;
    return true;
}

void CBufferedFile::FindByte(char ch) {
    for ( ; ; ) {
        if (nReadPos == nSrcPos)
            Fill();
        if (vchBuf[nReadPos % vchBuf.size()] == ch)
            break;
        nReadPos++;
    }
}
