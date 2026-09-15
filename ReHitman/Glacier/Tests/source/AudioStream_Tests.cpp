#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/Audio/ZIOStreamer.h>
#include <Glacier/Audio/ZDllSoundWintel.h>
#include <Glacier/Audio/ZSoundGraph.h>
#include <Glacier/Audio/ZSynth.h>
#include <Glacier/Audio/ZSynthDS.h>
#include <Tests/EngineFixture.h>
#include <Glacier/ZSTL/ZBlockAlloc.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace
{
    class TestIOStreamer final : public ZIOStreamer
    {
    public:
        uint32_t GetFileHandle(const char*) override
        {
            return 1;
        }

        bool Busy() override
        {
            return false;
        }

        void IssueRead() override
        {
        }
    };

    class TestSoundBuffer final : public _ZSoundBuffer
    {
    public:
        explicit TestSoundBuffer(ZSynth* _synth)
            : _ZSoundBuffer(_synth)
        {
        }

        void Render() override
        {
            m_bRendered = true;
        }

        void Start() override
        {
            m_bStarted = true;
            m_bPlaying = true;
        }

        bool m_bRendered = false;
        bool m_bStarted = false;
    };

    void PrepareStream(ZIOStream& _stream, ZBlockAlloc& _allocator, int _size)
    {
        auto* memory = static_cast<char*>(_allocator.AllocBlocks((_size + 3) / 4));
        ASSERT_NE(memory, nullptr);

        _stream = {};
        _stream.m_pAlloc = &_allocator;
        _stream.m_lMemHandle = 0;
        _stream.m_lCurrentReadHandle = -1;
        _stream.m_lCurrentBlockSize = 0;
        _stream.m_lHandleLastBlock = -1;
        _stream.m_lBytesReady = _size;
        _stream.m_lBytesLeftInStream = _size;
    }
}

TEST(AudioStream, GetDataReadsAcrossBlockChain)
{
    ZBlockAlloc allocator;
    allocator.Create(3, 4);

    ZIOStream stream;
    PrepareStream(stream, allocator, 10);
    std::memcpy(allocator.m_pBlockMemAligned, "ABCDEFGHIJ", 10);

    char output[10]{};
    int bytesRead = 0;
    const int status = stream.GetData(output, sizeof(output), bytesRead, 0);

    EXPECT_EQ(status, 2);
    EXPECT_EQ(bytesRead, 10);
    EXPECT_EQ(std::memcmp(output, "ABCDEFGHIJ", 10), 0);
    EXPECT_EQ(stream.m_lBytesReady, 0);
    EXPECT_EQ(stream.m_lBytesLeftInStream, 0);
}

TEST(AudioStream, GetDataPadsRequestAtEndOfStream)
{
    ZBlockAlloc allocator;
    allocator.Create(1, 4);

    ZIOStream stream;
    PrepareStream(stream, allocator, 3);
    std::memcpy(allocator.m_pBlockMemAligned, "STR", 3);

    char output[6]{};
    int bytesRead = 0;
    const int status = stream.GetData(output, sizeof(output), bytesRead, 0x7F);

    EXPECT_EQ(status, 2);
    EXPECT_EQ(bytesRead, 3);
    EXPECT_EQ(std::memcmp(output, "STR", 3), 0);
    EXPECT_EQ(static_cast<unsigned char>(output[3]), 0x7F);
    EXPECT_EQ(static_cast<unsigned char>(output[4]), 0x7F);
    EXPECT_EQ(static_cast<unsigned char>(output[5]), 0x7F);
}

TEST(AudioStream, ReadFromStreamReportsNotReadyAndAlreadyAtEnd)
{
    ZIOStream stream{};
    stream.m_lBytesLeftInStream = 8;

    char* data = reinterpret_cast<char*>(1);
    int bytesRead = -1;
    EXPECT_EQ(stream.ReadFromStream(&data, 4, bytesRead), 0);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(bytesRead, 0);

    stream.m_lBytesLeftInStream = 0;
    data = reinterpret_cast<char*>(1);
    bytesRead = -1;
    EXPECT_EQ(stream.ReadFromStream(&data, 4, bytesRead), 3);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(bytesRead, 0);
}

TEST(AudioStreamer, AddAudioStreamInitializesExtractionGeometry)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(8, 8, 1000));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup group{};
    streamer.m_Streams[0].m_pUserData = &group;

    ZIOStream* stream = streamer.AddAudioStream(8, 0x120, 24, false, 0, 4, 16, 2, false);

    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->m_pUserData, &group);
    EXPECT_EQ(stream->m_lFilePointer, 0x120u);
    EXPECT_EQ(stream->m_lFileOffset, 0x120u);
    EXPECT_EQ(stream->m_lBytesLeftInStream, 16);
    EXPECT_EQ(stream->m_lSizeOfStream, 24);
    EXPECT_EQ(stream->m_lNumberOfBlocks, 3);
    EXPECT_EQ(stream->m_lDataChunks, 2);
    EXPECT_EQ(stream->m_lNumMetaChunks, 2);
    EXPECT_EQ(stream->m_lMetaChunkCounter, 2);
    EXPECT_EQ(stream->m_pMetaMem, metadata);
    EXPECT_TRUE(stream->m_bActive);
    EXPECT_FALSE(stream->m_bPreloadReady);
}

TEST(AudioStreamer, AddAudioStreamRejectsFifthConcurrentStream)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(8, 8, 100));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup groups[4]{};
    for (int i = 0; i < 4; ++i)
    {
        streamer.m_Streams[i].m_pUserData = &groups[i];
        ASSERT_NE(streamer.AddAudioStream(1, i * 16, 8, false, 0, 0, 0, 0, false), nullptr);
    }

    EXPECT_EQ(streamer.AddAudioStream(1, 0, 8, false, 0, 0, 0, 0, false), nullptr);
    EXPECT_EQ(streamer.m_lCurrentNumStreams, 4);
}

TEST(AudioStreamer, RemoveStreamReturnsAllocatedBlocks)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(4, 8, 100));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup group{};
    streamer.m_Streams[0].m_pUserData = &group;

    ZIOStream* stream = streamer.AddAudioStream(1, 0, 16, false, 0, 0, 0, 0, false);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(streamer.m_Alloc.m_lNumAllocated, 1);

    streamer.RemoveStream(stream);

    EXPECT_EQ(streamer.m_Alloc.m_lNumAllocated, 0);
    EXPECT_EQ(streamer.m_lCurrentNumStreams, 0);
}

TEST(AudioStreamer, LoopRestartsLoadedPositionAtLoopOffset)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(4, 8, 100));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup group{};
    streamer.m_Streams[0].m_pUserData = &group;

    ZIOStream* stream = streamer.AddAudioStream(1, 100, 16, true, 4, 0, 0, 0, false);
    ASSERT_NE(stream, nullptr);
    stream->m_lBytesLoaded = 12;
    streamer.m_CurrentStream.m_pStream = stream;
    streamer.m_CurrentStream.m_lLoadSize = 4;
    streamer.m_CurrentStream.m_bUpdateStream = true;

    ASSERT_TRUE(streamer.Update());

    EXPECT_EQ(stream->m_lBytesLoaded, 4);
    EXPECT_EQ(stream->m_lFilePointer, 104u);
}

TEST(AudioSynth, RenderBufferStartsReadyVoice)
{
    ZSynthDS synth;
    TestSoundBuffer buffer(&synth);
    SWaveHeader wave{};
    buffer.m_rWave = &wave;
    buffer.m_bInUse = true;
    buffer.m_bReady = true;

    synth.RenderBuffer(&buffer);

    EXPECT_TRUE(buffer.m_bRendered);
    EXPECT_TRUE(buffer.m_bStarted);
    EXPECT_TRUE(buffer.m_bPlaying);
}

TEST(AudioDll, WintelConstructorPreservesDirectSoundDefaults)
{
    ZDllSoundWintel soundDll;

    EXPECT_EQ(soundDll.GetNumSynths(), 0);
    EXPECT_EQ(soundDll.m_pSynth, nullptr);
    EXPECT_EQ(soundDll.m_lSynthCmdBufferSize, 0x10000);
    EXPECT_EQ(soundDll.m_pSynthCmd, soundDll.m_pSynthCmdBuffer);
}

TEST(AudioDll, InstallCommandsUseSynthFrameLayout)
{
    ZDllSoundWintel soundDll;
    soundDll.m_bSoundInitialized = true;
    soundDll.InitFrame();

    ASSERT_TRUE(soundDll.InstallWaves(12, "resident.wav"));
    ASSERT_TRUE(soundDll.InstallWaveHeaders(24, "headers.whd"));
    ASSERT_TRUE(soundDll.InstallStreamWaves(36, "streams.wav"));

    const char* cursor = soundDll.m_pSynthCmdBuffer;
    const auto* waves = reinterpret_cast<const SSynthCmdBase*>(cursor);
    EXPECT_EQ(waves->m_lType, 0x100u);
    EXPECT_EQ(waves->m_lSize, 0x8Cu);
    cursor += waves->m_lSize;

    const auto* headers = reinterpret_cast<const SSynthCmdBase*>(cursor);
    EXPECT_EQ(headers->m_lType, 0x101u);
    EXPECT_EQ(headers->m_lSize, 0x8Cu);
    cursor += headers->m_lSize;

    const auto* streams = reinterpret_cast<const SSynthCmdBase*>(cursor);
    EXPECT_EQ(streams->m_lType, 0x102u);
    EXPECT_EQ(streams->m_lSize, 0x10Cu);
    EXPECT_EQ(soundDll.m_pSynthCmd, cursor + streams->m_lSize);
}

TEST(AudioDll, AppendSourceCommandBuildsDirectSound2DCommand)
{
    ZDllSoundWintel soundDll;
    soundDll.InitFrame();

    ZSoundObject object;
    object.m_eSourceType = ZSoundObject::SOURCE_2D;
    object.m_eState = STATE_STARTPLAY;
    object.m_lSoundFlags = 8;
    object.m_lBufferId = -1;
    object.m_fPrio = 42.0f;
    object.m_fStartOffset = 1.5f;
    object.m_lPan = -250;
    object.m_fVolumeAttenuation[1] = 75.0f;
    SWave wave{};
    wave.m_lHeaderOffset = 0x1235;

    soundDll.AppendSourceCommand(&object, 0x4567, &wave);

    const auto* command = reinterpret_cast<const SStartSound2D*>(soundDll.m_pSynthCmdBuffer);
    EXPECT_EQ(command->m_lType, 1u);
    EXPECT_EQ(command->m_lSize, sizeof(SStartSound2D));
    EXPECT_EQ(command->m_lFlags & 1, 1);
    EXPECT_EQ(command->m_lSndRef, 0x4567u);
    EXPECT_EQ(command->m_lHeaderOffset, 0x1234u);
    EXPECT_EQ(command->m_lBufferId, 127);
    EXPECT_EQ(command->m_lStartOffset, 384u);
    EXPECT_EQ(command->m_lPan, -250);
    EXPECT_EQ(command->m_fVolume, 75.0f);
}

TEST(AudioDll, SynthResultsUpdateSoundObjectState)
{
    ZDllSoundWintel soundDll;
    soundDll.m_ObjectManager.Initialize();
    ZSoundObject* object = soundDll.AllocateObject();
    ASSERT_NE(object, nullptr);
    const ZREF reference = object->m_lKey;

    struct Result
    {
        SSynthCmdBase command;
        uint32_t reference;
        int32_t value;
    } result{{0x1003, sizeof(Result)}, reference, 0};

    object->m_fTimeLeft = 5.0f;
    ASSERT_TRUE(soundDll.ProcessSynthCmd(reinterpret_cast<const char*>(&result)));
    EXPECT_EQ(object->m_fTimeLeft, 7.0f);
    EXPECT_NE(object->m_lSoundFlags & 0x40000u, 0u);

    result.command.m_lType = 0x1004;
    result.value = 23;
    ASSERT_TRUE(soundDll.ProcessSynthCmd(reinterpret_cast<const char*>(&result)));
    EXPECT_EQ(object->GetBufferId(), 23);

    result.command.m_lType = 0x1008;
    result.value = 2;
    ASSERT_TRUE(soundDll.ProcessSynthCmd(reinterpret_cast<const char*>(&result)));
    EXPECT_EQ(object->GetPoseData(), soundDll.m_pMetaMemory + 0x2000);

    object->m_eState = STATE_PLAYING;
    result.command.m_lType = 0x1002;
    ASSERT_TRUE(soundDll.ProcessSynthCmd(reinterpret_cast<const char*>(&result)));
    EXPECT_EQ(object->m_eState, STATE_DEPRICATED);
}

TEST(AudioDll, PackedSoundResolvesItsWaveByByteOffset)
{
    ZDllSoundWintel soundDll;
    char packedData[256]{};
    constexpr int soundOffset = 32;
    constexpr int waveOffset = 128;
    auto* sound = reinterpret_cast<SSound*>(packedData + soundOffset);
    sound->m_Type = ZAudioTypes::Sound;
    sound->m_lWaveOffset = waveOffset;
    auto* wave = reinterpret_cast<SWave*>(packedData + waveOffset);
    wave->m_Type = ZAudioTypes::Wave;

    ASSERT_TRUE(soundDll.InstallSounds(packedData, sizeof(packedData)));
    EXPECT_EQ(soundDll.GetPackedObject(soundOffset), sound);
    EXPECT_EQ(soundDll.GetWave(sound), wave);
    EXPECT_EQ(soundDll.GetPackedObject(sizeof(packedData) - 4), nullptr);
}

TEST(AudioDll, CalcPriorityMatchesDirectSound2DAnd3DFalloff)
{
    ZDllSoundWintel soundDll;
    ZSoundObject object;
    object.m_dwPriority = 2;
    object.m_fVolumeAttenuation[1] = 50.0f;
    object.m_eSourceType = ZSoundObject::SOURCE_2D;

    soundDll.CalcPriority(&object);
    EXPECT_FLOAT_EQ(object.m_fPrio, 2600.0f);

    object.m_eSourceType = static_cast<ZSoundObject::ESourceType>(0);
    object.m_vPosition = {20.0f, 0.0f, 0.0f};
    object.m_fMinDist = 10.0f;
    object.m_fMinDistFactor = 1.0f;
    soundDll.CalcPriority(&object);
    EXPECT_FLOAT_EQ(object.m_fPrio, 2350.0f);
}

TEST(AudioDll, FilterChainCommandsSerializeOcclusionPath)
{
    ZDllSoundWintel soundDll;
    auto* graph = new ZSoundGraph();
    graph->m_pExits = new SExit[1]{};
    graph->m_pExits[0].m_fOpenness = 0.25f;
    graph->m_lNumExits = 1;
    graph->m_lNumPathes = 1;
    graph->m_Pathes[0].m_bNew = true;
    graph->m_Pathes[0].m_lRemapIdx = 7;
    graph->m_Pathes[0].m_lNumConnections = 1;
    graph->m_Pathes[0].m_Filters[0] = {0, 1, 0.0f};
    soundDll.m_pSoundGraph = graph;
    soundDll.InitFrame();

    soundDll.AddFilterChainCommands();

    const char* cursor = soundDll.m_pSynthCmdBuffer;
    EXPECT_EQ(reinterpret_cast<const SSynthCmdBase*>(cursor)->m_lType, 0x30u);
    cursor += 8;
    EXPECT_EQ(reinterpret_cast<const SSynthCmdBase*>(cursor)->m_lType, 0x41u);
    cursor += 16;
    EXPECT_EQ(reinterpret_cast<const SSynthCmdBase*>(cursor)->m_lType, 0x42u);
    EXPECT_EQ(reinterpret_cast<const SSynthFilterBase*>(cursor)->m_lNextFilter, 0);
    EXPECT_FLOAT_EQ(reinterpret_cast<const SCmdOcclusionWintel*>(cursor)->m_fOpenness, 0.25f);
    cursor += sizeof(SCmdOcclusionWintel);
    EXPECT_EQ(reinterpret_cast<const SSynthCmdBase*>(cursor)->m_lType, 0x40u);
    cursor += 8;
    EXPECT_EQ(reinterpret_cast<const SSynthCmdBase*>(cursor)->m_lType, 0x31u);
}

TEST(SoundGraph, InitializeParsesArraysAndTestsConnections)
{
    const uint32_t counts[] = {1, 0, 2, 1, 0};
    const size_t size = sizeof(counts) + sizeof(SExit) + 2 * sizeof(SGRoom) + sizeof(uint32_t);
    char* data = static_cast<char*>(::operator new(size));
    std::memset(data, 0, size);
    char* cursor = data;
    std::memcpy(cursor, counts, sizeof(counts));
    cursor += sizeof(counts);
    auto* exit = reinterpret_cast<SExit*>(cursor);
    exit->m_fOpenness = 1.0f;
    exit->m_lRoomId = 0;
    exit->m_lRoomId2 = 1;
    cursor += sizeof(SExit);
    auto* rooms = reinterpret_cast<SGRoom*>(cursor);
    rooms[0] = {0, 1, 0};
    rooms[1] = {0, 0, 1};
    cursor += 2 * sizeof(SGRoom);
    *reinterpret_cast<uint32_t*>(cursor) = (1u << 16);

    ZSoundGraph graph;
    graph.InstallLoadedGraph(data, static_cast<int>(size));
    graph.Initialize();

    EXPECT_EQ(graph.m_lNumExits, 1u);
    EXPECT_EQ(graph.m_lNumRooms, 2u);
    EXPECT_TRUE(graph.TestRooms(0, 1, false));
    graph.m_pExits[0].m_fOpenness = 0.0f;
    EXPECT_FALSE(graph.TestRooms(0, 1, false));
    EXPECT_TRUE(graph.TestRooms(0, 1, true));
}

TEST(SoundGraph, FindChainComparesConnectionSequence)
{
    ZSoundGraph graph;
    SPath paths[2]{};
    paths[0].m_lNumConnections = 2;
    paths[0].m_tConnections[0] = 3;
    paths[0].m_tConnections[1] = 7;
    paths[1].m_lNumConnections = 1;
    paths[1].m_tConnections[0] = 9;
    SPath wanted{};
    wanted.m_lNumConnections = 2;
    wanted.m_tConnections[0] = 3;
    wanted.m_tConnections[1] = 7;

    EXPECT_EQ(graph.FindChain(paths, 2, &wanted), 0);
    wanted.m_tConnections[1] = 8;
    EXPECT_EQ(graph.FindChain(paths, 2, &wanted), -1);
}

class AudioDirectSoundTest : public Tests::EngineFixture
{
};

TEST_F(AudioDirectSoundTest, CreateAndFreeWhenDeviceIsAvailable)
{
    ZSynthDS synth;
    if (!synth.Create())
        GTEST_SKIP() << "No DirectSound device is available";

    EXPECT_NE(synth.GetDS(), nullptr);
    synth.Free();
}
