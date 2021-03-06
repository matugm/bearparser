#include "SectHdrsWrapper.h"
#include "PEFile.h"

const size_t SECNAME_LEN = 8;

bool SectionHdrWrapper::wrap()
{
    this->header = NULL;
    getPtr();
    reloadName();
    return true;
}

void* SectionHdrWrapper::getPtr()
{
    if (header != NULL) {
        return (void*) this->header;
    }
    PEFile *pe = dynamic_cast<PEFile*>(this->m_Exe);
    if (pe == NULL) return NULL;
    if (this->sectNum >= pe->hdrSectionsNum()) return NULL;

    uint32_t hdrOffset = pe->secHdrsOffset();
    uint64_t secOffset = hdrOffset + (this->sectNum * sizeof(pe::IMAGE_SECTION_HEADER));
    if ( (secOffset + sizeof(IMAGE_SECTION_HEADER)) > m_Exe->getRawSize()) return NULL;
    BYTE* content = m_Exe->getContent();
    if (!content) return NULL;

    this->header = (pe::IMAGE_SECTION_HEADER*) ((uint64_t)content + secOffset);
    return (void*) this->header;
}

bool SectionHdrWrapper::reloadName()
{
    pe::IMAGE_SECTION_HEADER* header = (pe::IMAGE_SECTION_HEADER*) getPtr();
    if (!header) return false;

    if (this->name) {
        if (memcmp(this->name, header->Name, SECNAME_LEN) == 0) {
            return true; //no need to reload
        }
    }
    const size_t BUF_LEN = SECNAME_LEN + 2;
    char *buf = new char[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    snprintf(buf, BUF_LEN, "%.8s", (char*) header->Name);

    delete []this->name;
    this->name = buf;
    return true;
}

bufsize_t SectionHdrWrapper::getSize()
{
    PEFile *pe = dynamic_cast<PEFile*>(this->m_Exe);
    if (pe == NULL) return 0;
    return sizeof(pe::IMAGE_SECTION_HEADER);
}

QString SectionHdrWrapper::getName()
{
    //reloadName();
    if (!this->name) return ""; //cannot load
    return this->name;
}

void* SectionHdrWrapper::getFieldPtr(size_t fieldId, size_t subField)
{
    pe::IMAGE_SECTION_HEADER* sec = (pe::IMAGE_SECTION_HEADER*) getPtr();
    if (!sec) return NULL;
    if (!this->name) return NULL;
    switch (fieldId)
    {
        case NAME: return (void*) &sec->Name;
        case VSIZE: return (void*) &sec->Misc.VirtualSize;
        case VPTR: return (void*) &sec->VirtualAddress;
        case RSIZE: return (void*) &sec->SizeOfRawData;
        case RPTR: return(void*) &sec->PointerToRawData;

        case RELOC_PTR: return (void*) &sec->PointerToRelocations;
        case RELOC_NUM: return (void*) &sec->NumberOfRelocations;
        case LINENUM_PTR: return (void*) &sec->PointerToLinenumbers;
        case LINENUM_NUM: return (void*) &sec->NumberOfLinenumbers;

        case CHARACT: return (void*) &sec->Characteristics;
    }
    return this->getPtr();
}

QString SectionHdrWrapper::getFieldName(size_t fieldId)
{
    switch (fieldId)
    {
        case NAME: return "Name";
        case VSIZE: return "Virtual Size";
        case VPTR: return "Virtual Addr.";
        case RSIZE: return "Raw size";
        case RPTR: return "Raw Addr.";
        case CHARACT: return "Characteristics";
        case RELOC_PTR: return "Ptr to Reloc.";
        case RELOC_NUM: return "Num. of Reloc.";
        case LINENUM_PTR: return "Ptr to Linenum.";
        case LINENUM_NUM: return "Num. of Linenum.";
    }
    return "";
}

Executable::addr_type SectionHdrWrapper::containsAddrType(size_t fieldId, size_t subField)
{
    switch (fieldId)
    {
        case VPTR: return Executable::RVA;
        case RPTR: return Executable::RAW;
        //case RELOC_PTR: return Executable::RAW;
    }
    return Executable::NOT_ADDR;
}

WrappedValue::data_type SectionHdrWrapper::containsDataType(size_t fieldId, size_t subField) 
{
    if (fieldId == NAME) {
        return WrappedValue::STRING;
    }
    return WrappedValue::INT;
}

offset_t SectionHdrWrapper::getContentOffset(Executable::addr_type aType)
{
    if (this->header == NULL) return INVALID_ADDR;

    //bool isOk = false;
    offset_t offset = INVALID_ADDR;

    if (aType == Executable::RAW) {
        offset = static_cast<offset_t>(this->header->PointerToRawData);//(this->getNumValue(RPTR, &isOk));
    } else if (aType == Executable::VA || aType == Executable::RVA) {
        offset = static_cast<offset_t>(this->header->VirtualAddress);//this->getNumValue(VPTR, &isOk));
    }
    //if (!isOk) return INVALID_ADDR;
    return offset;
}
//TODO: move to util...
offset_t roundupToUnit(offset_t size, offset_t unit)
{
    size_t unitsNum = size / unit;
    offset_t roundDown = unitsNum * unit;
    if (roundDown < size) unitsNum ++;
    return unitsNum * unit;
}

offset_t SectionHdrWrapper::getContentEndOffset(Executable::addr_type addrType, bool roundup)
{
    offset_t startOffset = getContentOffset(addrType);
    if (startOffset == INVALID_ADDR) return INVALID_ADDR;

    offset_t endOffset = static_cast<offset_t>(getContentSize(addrType, roundup)) + startOffset;
    return endOffset;
}

bufsize_t SectionHdrWrapper::getContentSize(Executable::addr_type aType, bool roundup)
{
    if (this->header == NULL) return 0;
    //bool isOk = false;
    bufsize_t size = 0;

    if (aType == Executable::RAW) {
        size = static_cast<bufsize_t>(this->header->SizeOfRawData);//this->getNumValue(RSIZE, &isOk));
    } else if (aType == Executable::VA || aType == Executable::RVA) {
        size = static_cast<bufsize_t>(this->header->Misc.VirtualSize);//this->getNumValue(VSIZE, &isOk));
    }
    //if (!isOk) return 0;

    if (roundup) {
        PEFile *pe = dynamic_cast<PEFile*>(m_Exe);
        if (pe) {
            offset_t unit = pe->getAlignment(aType);
            offset_t startOffset = getContentOffset(aType);
            //TODO: check it!
            if (aType == Executable::RAW) {
                bufsize_t maxSize = pe->getRawSize() - startOffset; //round down only for Raw
                size = roundupToUnit(size, unit);
                if (size > maxSize) size = maxSize;
            }
        }
    }
    return size;
}

//-----------------------------------------------------------------------------------

bool SectHdrsWrapper::wrap()
{
    PEFile *pe = dynamic_cast<PEFile*>(this->m_Exe);
    if (pe == NULL) return false;

    size_t count = pe->hdrSectionsNum();

    for (int i = 0; i < count; i++) {
        SectionHdrWrapper *sec = new SectionHdrWrapper(this->m_Exe, i);
        if (sec == NULL) break;
        if (sec->getPtr() == NULL) {
            printf("deleting invalid section..\n");
            delete sec;
            sec = NULL;
            break;
        }
        this->entries.push_back(sec);

        bool roundup = true;
        if (sec->getContentSize(Executable::RAW, true) == 0) {
            //printf("skipping empty section..\n");
            continue;
        }

        offset_t RVA =sec->getContentOffset(Executable::RVA);
        offset_t raw =sec->getContentOffset(Executable::RAW);

        offset_t endRVA = sec->getContentEndOffset(Executable::RVA, roundup);
        offset_t endRaw = sec->getContentEndOffset(Executable::RAW, roundup);
        vSec[endRVA] = sec;

        if (rSec[endRaw] != NULL) {
            if (rSec[endRaw]->getContentOffset(Executable::RAW) < sec->getContentOffset(Executable::RAW)) {
                //printf("endRaw = %llX - SKIP\n", endRaw);
                continue; //skip
            }
        }
        rSec[endRaw] = sec;
    }
    return true;
}

size_t SectHdrsWrapper::getFieldsCount()
{
    return this->entries.size();
}

void* SectHdrsWrapper::getPtr()
{
    if (entries.size() == 0) return NULL;
    return entries[0]->getPtr();
}

bufsize_t SectHdrsWrapper::getSize()
{
    PEFile *pe = dynamic_cast<PEFile*>(this->m_Exe);
    if (pe == NULL) return 0;

    size_t secCount = getFieldsCount();

    uint64_t hdrOffset = pe->secHdrsOffset();
    uint64_t fileSize = m_Exe->getRawSize();
    uint64_t endOffset = hdrOffset + (secCount * sizeof(pe::IMAGE_SECTION_HEADER));

    if (endOffset > fileSize) {
        return bufsize_t (fileSize - hdrOffset);
    }
    return bufsize_t (endOffset - hdrOffset);
}

void* SectHdrsWrapper::getFieldPtr(size_t fieldId, size_t subField)
{
    if (fieldId >= entries.size()) return NULL;
    return entries[fieldId]->getFieldPtr(subField);
}

QString SectHdrsWrapper::getFieldName(size_t fieldId)
{
    if (fieldId >= entries.size()) return NULL;
    return entries[fieldId]->getName();
}

SectionHdrWrapper* SectHdrsWrapper::getSecHdrAtOffset(offset_t offset, Executable::addr_type addrType, bool roundup, bool verbose)
{
    size_t size = this->entries.size();
    std::map<offset_t, SectionHdrWrapper*> *secMap = NULL;

    if (addrType == Executable::RAW) {
        secMap = &this->rSec;
    } else if (addrType == Executable::RVA || addrType == Executable::VA) {
        secMap = &this->vSec;
    }
    if (secMap == NULL) return NULL;

    std::map<offset_t, SectionHdrWrapper*>::iterator found = secMap->lower_bound(offset);
    std::map<offset_t, SectionHdrWrapper*>::iterator itr;
    for (itr = found; itr != secMap->end(); itr++) {
        SectionHdrWrapper* sec = itr->second;
        if (sec == NULL) continue; //TODO: check it
        if (verbose) printf("found [%llX] key: %llX sec: %llX %llX\n", offset, itr->first, sec->getContentOffset(addrType), sec->getContentEndOffset(addrType, false));

        offset_t startOffset = sec->getContentOffset(addrType);
        if (startOffset == INVALID_ADDR) continue;

        offset_t endOffset = sec->getContentEndOffset(addrType, roundup);

        if (offset >= startOffset && offset < endOffset) {
            return sec;
        }
        if (offset < startOffset) break;
    }
    return NULL;
}

void SectHdrsWrapper::printSectionsMapping(Executable::addr_type aType)
{
    std::map<offset_t, SectionHdrWrapper*> *secMap = NULL;

    if (aType == Executable::RAW) {
        secMap = &this->rSec;
    } else if (aType == Executable::RVA || aType == Executable::VA) {
        secMap = &this->vSec;
    }
    if (secMap == NULL) return;

    std::map<offset_t, SectionHdrWrapper*>::iterator itr;
    for (itr = secMap->begin(); itr != secMap->end(); itr++) {
        SectionHdrWrapper* sec = itr->second;
        offset_t secEnd = itr->first;

        printf("[%llX] %s %llX %llX\n", secEnd, sec->getName().toStdString().c_str(), sec->getContentOffset(aType), sec->getContentEndOffset(aType, true));
    }
    printf("---\n\n");
}

