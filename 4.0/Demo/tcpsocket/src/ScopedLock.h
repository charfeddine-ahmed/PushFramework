#pragma once

class ScopedLock
{
public:

    ScopedLock(CRITICAL_SECTION& _csLock)
        :csLock(_csLock)
    {
        ::EnterCriticalSection(&csLock);
    }

    ~ScopedLock(void)
    {
        ::LeaveCriticalSection(&csLock);
    }
private:
    CRITICAL_SECTION& csLock;
};
