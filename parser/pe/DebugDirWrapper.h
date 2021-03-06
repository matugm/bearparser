#pragma once

#include "../ExeNodeWrapper.h"
#include "pe_formats.h"

class DebugDirWrapper : public ExeElementWrapper
{
public:

    enum DebugDirFID {
        NONE = FIELD_NONE,
        CHARACTERISTIC,
        TIMESTAMP,
        MAJOR_VER,
        MINOR_VER,
        TYPE,
        DATA_SIZE,
        RAW_DATA_ADDR,
        RAW_DATA_PTR,
        FIELD_COUNTER
    };

    DebugDirWrapper(Executable *pe);
    ~DebugDirWrapper() { clear(); }

    bool wrap();

    virtual void* getPtr();
    virtual bufsize_t getSize();
    virtual QString getName();
    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void* getFieldPtr(size_t fieldId, size_t subField);
    virtual QString getFieldName(size_t fieldId);
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE);

    QString translateType(int type);

private:
    pe::IMAGE_DEBUG_DIRECTORY* debugDir();

    void clear() {}
};

