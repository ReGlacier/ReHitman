#include <map>
#include <vector>

#include <Glacier/ZSTL/STLport.h>

int main()
{
    std::map<int, int> nativeMap;
    std::vector<int> nativeVector;
    stlp::map<int, int> glacierMap;
    stlp::vector<int> glacierVector;

    nativeMap[1] = 2;
    nativeVector.push_back(3);
    glacierMap[4] = 5;
    glacierVector.push_back(6);

    return nativeMap[1] + nativeVector[0] + glacierMap[4] + glacierVector[0] == 16 ? 0 : 1;
}
