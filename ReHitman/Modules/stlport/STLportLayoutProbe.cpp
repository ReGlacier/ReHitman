// Verifies that the vendored STLport 4.6.1 configuration still matches the original
// Glacier binary layouts for the STL containers observed in PC/PS2 decompilation.
// This is intentionally kept as a tiny C++98-compatible executable so CI can catch
// accidental namespace, compiler, or include-path changes that would break ABI parity.

#include <map>
#include <set>
#include <string>
#include <vector>

extern "C" int printf(const char*, ...);

struct zstring
{
    char* m_pData;
    unsigned int m_iLength;
    unsigned int m_iCapacity;
};

int main()
{
    typedef stlp::map<void*, unsigned int> Map;
    typedef stlp::string String;
    typedef stlp::vector<void*> Vector;
    typedef stlp::multimap<void*, unsigned int> Multimap;
    typedef stlp::set<zstring> Set;

    ::printf("sizeof(stlp::map<void*, unsigned int>) = 0x%X (%u)\n", unsigned(sizeof(Map)), unsigned(sizeof(Map)));
    ::printf("sizeof(stlp::string) = 0x%X (%u)\n", unsigned(sizeof(String)), unsigned(sizeof(String)));
    ::printf("sizeof(stlp::vector<void*>) = 0x%X (%u)\n", unsigned(sizeof(Vector)), unsigned(sizeof(Vector)));
    ::printf("sizeof(stlp::multimap<void*, unsigned int>) = 0x%X (%u)\n", unsigned(sizeof(Multimap)), unsigned(sizeof(Multimap)));
    ::printf("sizeof(stlp::set<zstring>) = 0x%X (%u)\n", unsigned(sizeof(Set)), unsigned(sizeof(Set)));
    ::printf("sizeof(zstring) = 0x%X (%u)\n", unsigned(sizeof(zstring)), unsigned(sizeof(zstring)));

    int result = 0;
    if (sizeof(Map) != 0xC)
        result = 1;
    if (sizeof(String) != sizeof(zstring))
        result = 1;
    if (sizeof(Vector) != 0xC)
        result = 1;
    if (sizeof(Multimap) != 0xC)
        result = 1;
    if (sizeof(Set) != 0xC)
        result = 1;

    return result;
}
