#include <Glacier/Geom/ZParticleTemplate.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <algorithm>
#include <cstring>


namespace Glacier
{
    ZParticleTemplate::ZParticleTemplate(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSTDOBJ(psName, pBaseGeom)
        , m_pszFileName()
        , m_szNextTemplateName()
        , m_pColorTable(nullptr)
        , m_lControllerIndex(0)
    {
    }

    ZParticleTemplate::~ZParticleTemplate()
    {
        ZUniMemory::Free(m_pColorTable);
        m_pColorTable = nullptr;
    }

    const RTP::ZPropertyInfo& ZParticleTemplate::GetProperties() const { return Info; }
    uint32_t ZParticleTemplate::GetObjectId() const { return m_Id; }
    void ZParticleTemplate::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const { id = m_Id; mask = m_Mask; }
    ZGEOMCLASSINFO* ZParticleTemplate::GetOldClassInfo() const { return m_OldClassInfo; }

    ZGEOM* ZParticleTemplate::DuplicateInit(ZGROUP*, const ZMat3x3*, const ZVector3*, const char*, bool)
    {
        return nullptr;
    }

    void ZParticleTemplate::CopyData(const ZGEOM* pSource)
    {
        ZGEOM::CopyData(pSource);
        const auto* source = geom_cast<ZParticleTemplate>(pSource);
        if (!source)
            return;

        m_vGravity = source->m_vGravity;
        m_fFriction = source->m_fFriction;
        m_fMaxAge = source->m_fMaxAge;
        m_fScale = source->m_fScale;
        m_fScaleVariation = source->m_fScaleVariation;
        m_fScaleVel = source->m_fScaleVel;
        m_fScaleAcc = source->m_fScaleAcc;
        m_fAngleSpeed = source->m_fAngleSpeed;
        m_fAngleSpeedVel = source->m_fAngleSpeedVel;
        m_fAngleSpeedAcc = source->m_fAngleSpeedAcc;
        m_bRandomStartAngle = source->m_bRandomStartAngle;
        m_bAlignWithEmitterDir = source->m_bAlignWithEmitterDir;
        m_bAlignWithParticleDir = source->m_bAlignWithParticleDir;
        m_fAngleStart = source->m_fAngleStart;
        m_fMotionStretch = source->m_fMotionStretch;
        m_vWindSpeed = source->m_vWindSpeed;
        m_fWindForce = source->m_fWindForce;
        m_fCollisionRate = source->m_fCollisionRate;
        m_fRestitutionFactor = source->m_fRestitutionFactor;
        m_lColorRepeat = source->m_lColorRepeat;
        m_bFake3dRotation = source->m_bFake3dRotation;
        m_bRandomTrajectoryEnvelope = source->m_bRandomTrajectoryEnvelope;
        m_bRandomRotationDir = source->m_bRandomRotationDir;
        m_rNextTemplate = source->m_rNextTemplate;
        m_lPrimsTable = source->m_lPrimsTable;
        m_pszFileName = source->m_pszFileName;
        m_szNextTemplateName = source->m_szNextTemplateName;
        m_fChanceTransfer = source->m_fChanceTransfer;
        std::memcpy(m_vColorKeys, source->m_vColorKeys, sizeof(m_vColorKeys));
    }

    void ZParticleTemplate::ClassInit()
    {
        ZSTDOBJ::ClassInit();

        constexpr float DegreesToRadians = 0.017453292f;
        m_fScaleAcc *= DegreesToRadians;
        m_fAngleSpeed *= DegreesToRadians;
        m_fAngleSpeedVel *= DegreesToRadians;
        m_fAngleSpeedAcc *= DegreesToRadians;
        m_fCollisionRate = std::clamp(m_fCollisionRate, 0.0f, 1.0f);
        m_fRestitutionFactor = std::clamp(m_fRestitutionFactor, 0.0f, 1.0f);

        CreateColorTable();
        if (m_fFriction <= 0.01f)
            m_fFriction = 0.01f;
        m_vScaledGravity = m_vGravity * (1.0f / m_fFriction);

        if (!m_rNextTemplate && m_szNextTemplateName.c_str() && *m_szNextTemplateName.c_str())
        {
            // TODO: Finish this place after ZParticleTemplateList will be reversed
        }
    }

    void ZParticleTemplate::ClassInit2()
    {
        ZGEOM::ClassInit2();
        ZREF controller = 0;
        GetController(&controller, &m_lControllerIndex);
    }

    void ZParticleTemplate::GetController(ZREF* pController, uint32_t* pIndex)
    {
        *pController = g_pSysInterface->m_pEngineData->m_rParticleControllerGeom;
        if (!m_lControllerIndex)
        {
            ZGEOM* controller = ZGEOM::RefToPtr(*pController);
            if (controller)
            {
                using RegisterParticleTemplate = uint32_t(__thiscall*)(ZGEOM*, ZREF);
                auto function = reinterpret_cast<RegisterParticleTemplate>((*reinterpret_cast<void***>(controller))[118]);
                m_lControllerIndex = function(controller, GetRef());
            }
        }
        *pIndex = m_lControllerIndex;
    }

    ZREF ZParticleTemplate::FindTemplate(const char*)
    {
        // TODO: Finish this place after ZParticleTemplateList will be reversed
        return 0;
    }

    void ZParticleTemplate::CreateColorTable()
    {
        // TODO: Finish this place after ZSysFile will be reversed
        m_pColorTable = static_cast<uint32_t*>(ZUniMemory::Allocate(128 * sizeof(uint32_t)));
        for (uint32_t i = 0; i < 128; ++i)
        {
            const float key = static_cast<float>(i) * (7.0f / 127.0f);
            const uint32_t first = std::min(static_cast<uint32_t>(key), 7u);
            const uint32_t second = std::min(first + 1, 7u);
            const float blend = key - static_cast<float>(first);
            float color[4];
            for (uint32_t component = 0; component < 4; ++component)
            {
                color[component] = (m_vColorKeys[first][component]
                    + (m_vColorKeys[second][component] - m_vColorKeys[first][component]) * blend) / 255.0f;
            }
            ZPrimControlBase::Instance()->ColorFVTOIU(&m_pColorTable[i], color);
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
#       define PARTICLE_PROPERTY(Name, Next, Type, Table) \
            static RTP::ZDataProperty<Type> Name { \
                .m_Node = { .m_Next = Next, .m_Name = #Name, .m_Filter = 1 }, \
                .m_VirtualTable = Table, .m_Offset = CLASS_PROPERTY(ZParticleTemplate, Name) }

        PARTICLE_PROPERTY(m_vColorKeys, nullptr, float[8][4], &RTP::VirtualTables::Data_float_8_4);
        PARTICLE_PROPERTY(m_fChanceTransfer, m_vColorKeys, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_szNextTemplateName, m_fChanceTransfer, ZRTString, &RTP::VirtualTables::Data_ZRTString);
        PARTICLE_PROPERTY(m_pszFileName, m_szNextTemplateName, ZFILENAME, &RTP::VirtualTables::Data_ZFILENAME);
        PARTICLE_PROPERTY(m_lPrimsTable, m_pszFileName, uint, &RTP::VirtualTables::Data_uint);
        static RTP::ZDataProperty<ZGEOMREF> m_rNextTemplate {
            .m_Node = { .m_Next = m_lPrimsTable, .m_Name = "m_rNextTemplate", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZParticleTemplate, m_rNextTemplate)) };
        PARTICLE_PROPERTY(m_bRandomRotationDir, m_rNextTemplate, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_bRandomTrajectoryEnvelope, m_bRandomRotationDir, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_bFake3dRotation, m_bRandomTrajectoryEnvelope, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_lColorRepeat, m_bFake3dRotation, int, &RTP::VirtualTables::Data_int);
        PARTICLE_PROPERTY(m_fRestitutionFactor, m_lColorRepeat, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fCollisionRate, m_fRestitutionFactor, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fWindForce, m_fCollisionRate, float, &RTP::VirtualTables::Data_float);
        static RTP::ZDataProperty<float[3]> m_vWindSpeed {
            .m_Node = { .m_Next = m_fWindForce, .m_Name = "m_vWindSpeed", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(CLASS_PROPERTY(ZParticleTemplate, m_vWindSpeed)) };
        PARTICLE_PROPERTY(m_fMotionStretch, m_vWindSpeed, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fAngleStart, m_fMotionStretch, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_bAlignWithParticleDir, m_fAngleStart, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_bAlignWithEmitterDir, m_bAlignWithParticleDir, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_bRandomStartAngle, m_bAlignWithEmitterDir, bool, &RTP::VirtualTables::Data_bool);
        PARTICLE_PROPERTY(m_fAngleSpeedAcc, m_bRandomStartAngle, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fAngleSpeedVel, m_fAngleSpeedAcc, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fAngleSpeed, m_fAngleSpeedVel, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fScaleAcc, m_fAngleSpeed, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fScaleVel, m_fScaleAcc, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fScaleVariation, m_fScaleVel, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fScale, m_fScaleVariation, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fAgeVariation, m_fScale, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fMaxAge, m_fAgeVariation, float, &RTP::VirtualTables::Data_float);
        PARTICLE_PROPERTY(m_fFriction, m_fMaxAge, float, &RTP::VirtualTables::Data_float);
        static RTP::ZDataProperty<float[3]> m_vGravity {
            .m_Node = { .m_Next = m_fFriction, .m_Name = "m_vGravity", .m_Filter = 1 },
            .m_VirtualTable = &RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(CLASS_PROPERTY(ZParticleTemplate, m_vGravity)) };
#       undef PARTICLE_PROPERTY
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZParticleTemplate,
        ZSTDOBJ,
        0x00972F78,
        "ZParticleTemplate",
        0x0076C4F0,
        cProperties::m_vGravity,
        0x00808068,
        0x00972F18,
        0x00972F1C
    );
#   pragma endregion
}
