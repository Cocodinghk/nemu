#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "sete-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "sete-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "sete-template.h"
#undef DATA_BYTE
