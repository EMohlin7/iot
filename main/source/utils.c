#include "utils.h"
#include "string.h"

int intToAscii(int num, char* a, int len){
    char tmp[len];
    int i = len;
    while(num > 0 && i >= 0){
        --i;
        tmp[i] = (num % 10) + 48;
        num /= 10;
    }
    if(i == len)
        return 0;
    memcpy(a, tmp+i, len-i);
    a[len-i] = 0;
    return len-i;
}