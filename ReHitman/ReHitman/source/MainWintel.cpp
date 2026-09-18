#include <Windows.h>
#include <Glacier/Startup/GlacierStartup.h>
#include <BuildInfo.h>
#include <cstdio>


namespace
{
    void PrintBuilInfo()
    {
        printf("ReHitman build %s (Git commit %s / branch %s) compiler %s\n",
            BuildInfo::g_psBuildTag,
            BuildInfo::g_psGitCommit,
            BuildInfo::g_psGitBranch,
            BuildInfo::g_psCompiler
        );
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    PrintBuilInfo(); // Not hbm code
    using namespace Glacier;

    if (!IsDebuggerPresent())
    {
        return Glacier_RunNormal(reinterpret_cast<intptr_t>(hInstance), nShowCmd, lpCmdLine);
    }

    g_sProjectID = "hitman3";
    return Glacier_Main(reinterpret_cast<intptr_t>(hInstance), nShowCmd, lpCmdLine);
}
