#pragma once

#include "win_hdrs/win_types.h"
#include "CustomException.h"

#include <QtCore>

#include <iostream>
#include <stdlib.h>

#define DBG_LVL 0
#define TRACE() if (DBG_LVL) printf("%s: line: %d\n", __FUNCTION__,__LINE__);
#define LOG(msg) if (DBG_LVL) printf("%s: %s\n", __FUNCTION__,msg);
//------------------------------------------------

typedef uint32_t bufsize_t;
const bufsize_t BUFSIZE_MAX = bufsize_t(-1);

typedef uint64_t offset_t;
const offset_t INVALID_ADDR = offset_t(-1);
const offset_t  OFFSET_MAX = INVALID_ADDR - 1;

class BufferException : public CustomException
{
public:
    BufferException(const QString info) : CustomException(info) {}
};

class AbstractByteBuffer
{
public:
    AbstractByteBuffer() { }
    virtual ~AbstractByteBuffer() { }
//-----
    virtual bufsize_t getContentSize() = 0;
    virtual BYTE* getContent() = 0;

    const BYTE operator[](size_t idx);

    virtual offset_t getOffset(BYTE *ptr, bool allowExceptions = false); // validates
    virtual BYTE* getContentAt(offset_t offset, bufsize_t size, bool allowExceptions = false);
    virtual BYTE* getContentAtPtr(BYTE *ptr, bufsize_t size, bool allowExceptions = false);

    virtual bool setBufferedValue(BYTE *dstPtr, BYTE *srcPtr, bufsize_t srcSize, bufsize_t paddingSize, bool allowExceptions = false);

    bool fillContent(BYTE filling);

    bool containsBlock(offset_t rawOffset, bufsize_t size);
    bool intersectsBlock(offset_t rawOffset, bufsize_t size);

    uint64_t getNumValue(offset_t offset, bufsize_t size, bool* isOk);
    bool setNumValue(offset_t offset, bufsize_t size, uint64_t newVal);
};

