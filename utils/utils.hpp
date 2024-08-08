long long getRandNumber()
{
    long long result;
    __asm__ volatile(
        "rdrand %0\n"
        : "=r"(result)
        :
        : "cc");
    return result;
}
