#include "DOSExe.h"


bool DOSExeBuilder::signatureMatches(AbstractByteBuffer *buf)
{
    WORD *magic = (WORD*) buf->getContentAt(0, sizeof(WORD));
    if (magic == NULL) return false;

    if ((*magic) == S_DOS) {
        return true;
    }
    return false;
}

Executable* DOSExeBuilder::build(AbstractByteBuffer *buf)
{
    Executable *exe = NULL;
    if (signatureMatches(buf) == false) return NULL;

    try {
        exe = new DOSExe(buf);
    } catch (ExeException &e) {
        //
    }
    return exe;
}

//-------------------------------------------------------------

DOSExe::DOSExe(AbstractByteBuffer *v_buf)
    : MappedExe(v_buf, Executable::BITS_16), dosHdrWrapper(NULL)
{
    wrap(buf);
}

void DOSExe::wrap(AbstractByteBuffer *v_buf)
{
    this->dosHdrWrapper = new DosHdrWrapper(this);

     WORD* magic = (WORD*) this->dosHdrWrapper->getFieldPtr(DosHdrWrapper::MAGIC);

    if (this->dosHdrWrapper->getPtr() == NULL || magic == NULL) {
        throw ExeException("Could not Wrap!");
    }

    if ((*magic) != S_DOS) {
        printf("It is not a DOS file!\n");
        throw ExeException("It is not a DOS file!");
    }
    this->wrappers[WR_DOS_HDR] = this->dosHdrWrapper;
}

offset_t DOSExe::peSignatureOffset()
{
    LONG* lfnew = (LONG*) this->dosHdrWrapper->getFieldPtr(DosHdrWrapper::LFNEW);
    if (lfnew == NULL) return 0;

    return static_cast<offset_t>(*lfnew);
}

