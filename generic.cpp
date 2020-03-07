#include "includes/generic.h"

ZGenericFuncs::ZGenericFuncs(QObject *parent)
    : QObject(parent)
{

}

ZGenericFuncs::~ZGenericFuncs() = default;

int ZGenericFuncs::numDigits(int n) {
    const int base = 10;
    const int minusBase = ((-1)*base)+1;

    if ((n >= 0) && (n < base))
        return 1;

    if ((n >= minusBase) && (n < 0))
        return 2;

    if (n<0)
        return 2 + numDigits(abs(n) / base);

    return 1 + numDigits(n / base);
}
