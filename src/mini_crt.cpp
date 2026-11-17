// -----------------------------------------------------------------------------
//  Minimal CRT for the size-optimized MSVC build.
//
//  Enabled only when PIKA_MINI_CRT is defined (build_msvc.bat does this).
//  Other toolchains (MinGW, CMake) use their normal C/C++ runtime and skip
//  this translation unit entirely.
//
//  The game uses only malloc/calloc/free, memcpy/memset, strlen/strstr and
//  C++ new/delete.  Linking the full MSVC CRT adds ~90 KB to the executable;
//  this file provides just those symbols and a tiny WinMain entry point.
// -----------------------------------------------------------------------------
#if defined(PIKA_MINI_CRT)
#include <windows.h>
#include <cstddef>

extern "C" int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

extern "C" void WinMainCRTStartup(void)
{
    HINSTANCE hInstance = GetModuleHandleA(nullptr);
    int result = WinMain(hInstance, nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess((UINT)result);
}

// ---------------------------------------------------------------------------
//  Heap
// ---------------------------------------------------------------------------
extern "C" void* __cdecl malloc(size_t size)
{
    if (size == 0) size = 1;
    return HeapAlloc(GetProcessHeap(), 0, size);
}

extern "C" void __cdecl free(void* ptr)
{
    if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
}

extern "C" void* __cdecl calloc(size_t count, size_t size)
{
    size_t total = count * size;
    void* p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

extern "C" void* __cdecl realloc(void* ptr, size_t size)
{
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return nullptr; }
    return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

// ---------------------------------------------------------------------------
//  C++ allocation
// ---------------------------------------------------------------------------
void* operator new(size_t n) { return malloc(n); }
void* operator new[](size_t n) { return malloc(n); }
void operator delete(void* p) noexcept { free(p); }
void operator delete[](void* p) noexcept { free(p); }
void operator delete(void* p, size_t) noexcept { free(p); }
void operator delete[](void* p, size_t) noexcept { free(p); }

// ---------------------------------------------------------------------------
//  String / memory routines used by the game
// ---------------------------------------------------------------------------
extern "C" void* __cdecl memcpy(void* dst, const void* src, size_t n)
{
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
    return dst;
}

extern "C" void* __cdecl memmove(void* dst, const void* src, size_t n)
{
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

extern "C" void* __cdecl memset(void* dst, int value, size_t n)
{
    unsigned char* d = (unsigned char*)dst;
    while (n--) *d++ = (unsigned char)value;
    return dst;
}

extern "C" int __cdecl memcmp(const void* a, const void* b, size_t n)
{
    const unsigned char* x = (const unsigned char*)a;
    const unsigned char* y = (const unsigned char*)b;
    while (n--) {
        if (*x != *y) return (int)*x - (int)*y;
        x++; y++;
    }
    return 0;
}

extern "C" char* __cdecl getenv(const char* name)
{
    static char value[256];
    DWORD n = GetEnvironmentVariableA(name, value, sizeof(value));
    return n ? value : nullptr;
}

extern "C" size_t __cdecl strlen(const char* s)
{
    const char* p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

#endif // PIKA_MINI_CRT


