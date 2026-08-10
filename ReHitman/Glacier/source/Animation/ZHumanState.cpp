#include <Glacier/Animation/ZHumanState.h>

#include <cmath>
#include <cstring>

namespace Glacier
{
    const char* ZHumanState::m_StateName[9];
    const char* ZHumanState::m_Names[74];
    int ZHumanState::m_FloatCount[9];
    int ZHumanState::m_FloatStart[9];
    int ZHumanState::m_QuatCount[9];
    int ZHumanState::m_QuatStart[9];
    DataInfo ZHumanState::m_DataInfo[74];
    int16_t ZHumanState::m_StateId2Pos[74];
    int ZHumanState::firstRun = 1;

    ZHumanState::ZHumanState()
    {
        if (firstRun)
        {
            InitStaticData();
            firstRun = 0;
        }

        Reset();
    }

    ZHumanState::~ZHumanState() = default;

    void ZHumanState::Reset()
    {
        std::memset(m_Floats, 0, sizeof(m_Floats));

        for (int i = 0; i < 10; ++i)
        {
            m_Quats[i].i = 0.0f;
            m_Quats[i].j = 0.0f;
            m_Quats[i].k = 0.0f;
            m_Quats[i].w = 1.0f;
        }

        m_Floats[1] = 1.0f;
        m_Floats[18] = 0.3f;
        m_Floats[16] = -0.2f;
        m_Floats[22] = -0.3f;
        m_Floats[20] = -0.2f;
        m_Floats[3] = 30.0f;
        m_Floats[4] = 0.0f;
        m_Floats[5] = 0.0f;

        const float invSqrt2 = 1.0f / std::sqrt(2.0f);

        m_Quats[0].i = invSqrt2;
        m_Quats[0].j = invSqrt2;
        m_Quats[0].k = 0.0f;
        m_Quats[0].w = 0.0f;

        m_Quats[6].i = 0.0f;
        m_Quats[6].j = 1.0f;
        m_Quats[6].k = 0.0f;
        m_Quats[6].w = 0.0f;

        m_Quats[8].i = 0.0f;
        m_Quats[8].j = 1.0f;
        m_Quats[8].k = 0.0f;
        m_Quats[8].w = 0.0f;

        m_Quats[4].i = invSqrt2;
        m_Quats[4].j = -invSqrt2;
        m_Quats[4].k = 0.0f;
        m_Quats[4].w = 0.0f;

        m_Quats[5].i = invSqrt2;
        m_Quats[5].j = -invSqrt2;
        m_Quats[5].k = 0.0f;
        m_Quats[5].w = 0.0f;
    }

    int ZHumanState::Mirror(int mask)
    {
        auto swapValues = [](float& lhs, float& rhs)
        {
            const float temp = lhs;
            lhs = rhs;
            rhs = temp;
        };

        auto swapNegatedValues = [](float& lhs, float& rhs)
        {
            const float temp = lhs;
            lhs = -rhs;
            rhs = -temp;
        };

        if (mask & kMirrorTorso)
        {
            m_Floats[0] = -m_Floats[0];
            m_Quats[0].i = -m_Quats[0].i;
            m_Quats[0].w = -m_Quats[0].w;

            const float axis[3] = { 0.0f, 0.0f, 1.0f };
            ZQuat rotation;
            ZQuat current = m_Quats[0];
            qrotaxis(rotation, axis, 3.1415927f);
            qmul(m_Quats[0], current, rotation);
        }

        if (mask & kMirrorSpine)
        {
            m_Floats[4] = -m_Floats[4];
            m_Quats[1].i = -m_Quats[1].i;
            m_Quats[1].k = -m_Quats[1].k;
        }

        if (mask & kMirrorHead)
        {
            m_Quats[2].i = -m_Quats[2].i;
            m_Quats[2].k = -m_Quats[2].k;
            m_Quats[3].i = -m_Quats[3].i;
            m_Quats[3].k = -m_Quats[3].k;
        }

        if (mask & kMirrorLegs)
        {
            swapNegatedValues(m_Floats[6], m_Floats[11]);
            swapValues(m_Floats[7], m_Floats[12]);
            swapValues(m_Floats[8], m_Floats[13]);
            swapNegatedValues(m_Floats[9], m_Floats[14]);
            swapValues(m_Floats[10], m_Floats[15]);

            swapValues(m_Quats[4].i, m_Quats[5].i);
            swapValues(m_Quats[4].j, m_Quats[5].j);
            swapValues(m_Quats[4].k, m_Quats[5].k);
            swapValues(m_Quats[4].w, m_Quats[5].w);

            m_Quats[4].j = -m_Quats[4].j;
            m_Quats[4].w = -m_Quats[4].w;
            m_Quats[5].j = -m_Quats[5].j;
            m_Quats[5].w = -m_Quats[5].w;
        }

        if (mask & kMirrorArms)
        {
            swapValues(m_Floats[16], m_Floats[20]);
            swapValues(m_Floats[17], m_Floats[21]);
            swapNegatedValues(m_Floats[18], m_Floats[22]);
            swapValues(m_Floats[19], m_Floats[23]);

            swapValues(m_Quats[7].i, m_Quats[9].i);
            swapValues(m_Quats[7].j, m_Quats[9].j);
            swapValues(m_Quats[7].k, m_Quats[9].k);
            swapValues(m_Quats[7].w, m_Quats[9].w);
            m_Quats[7].j = -m_Quats[7].j;
            m_Quats[7].w = -m_Quats[7].w;
            m_Quats[9].j = -m_Quats[9].j;
            m_Quats[9].w = -m_Quats[9].w;

            swapValues(m_Quats[6].i, m_Quats[8].i);
            swapValues(m_Quats[6].j, m_Quats[8].j);
            swapValues(m_Quats[6].k, m_Quats[8].k);
            swapValues(m_Quats[6].w, m_Quats[8].w);
            m_Quats[6].j = -m_Quats[6].j;
            m_Quats[6].w = -m_Quats[6].w;
            m_Quats[8].j = -m_Quats[8].j;
            m_Quats[8].w = -m_Quats[8].w;
        }

        if (mask & kMirrorHands)
        {
            for (int i = 0; i < 20; ++i)
            {
                if (i & 1)
                {
                    swapValues(m_Floats[24 + i], m_Floats[44 + i]);
                }
                else
                {
                    swapNegatedValues(m_Floats[24 + i], m_Floats[44 + i]);
                }
            }
        }

        int result = mask & 0xFFFFFE07;
        if (mask & kMirrorLeftLeg)
            result |= kMirrorRightLeg;
        if (mask & kMirrorRightLeg)
            result |= kMirrorLeftLeg;
        if (mask & kMirrorLeftArm)
            result |= kMirrorRightArm;
        if (mask & kMirrorRightArm)
            result |= kMirrorLeftArm;
        if (mask & kMirrorLeftHand)
            result |= kMirrorRightHand;
        if (mask & kMirrorRightHand)
            result |= kMirrorLeftHand;

        return result;
    }

    int ZHumanState::BoneIds(int state, uint16_t* ids)
    {
        switch (state)
        {
        case 0:
            ids[0] = 0;
            return 1;
        case 1:
            ids[0] = 1;
            ids[1] = 2;
            ids[2] = 3;
            return 3;
        case 2:
            ids[0] = 5;
            ids[1] = 4;
            return 2;
        case 3:
            ids[0] = 6;
            ids[1] = 7;
            ids[2] = 8;
            ids[3] = 9;
            return 4;
        case 4:
            ids[0] = 10;
            ids[1] = 11;
            ids[2] = 12;
            ids[3] = 13;
            return 4;
        case 5:
            ids[0] = 14;
            ids[1] = 15;
            ids[2] = 16;
            ids[3] = 17;
            return 4;
        case 6:
            ids[0] = 20;
            ids[1] = 21;
            ids[2] = 22;
            ids[3] = 23;
            return 4;
        case 7:
            for (uint16_t id = 26; id < 41; ++id)
            {
                *ids++ = id;
            }
            return 15;
        case 8:
            for (uint16_t id = 41; id < 56; ++id)
            {
                *ids++ = id;
            }
            return 15;
        default:
            return 0;
        }
    }

    void ZHumanState::InitStaticData()
    {
        m_StateName[0] = "Torso";
        m_StateName[1] = "Spine";
        m_StateName[2] = "Head";
        m_StateName[3] = "Left Leg";
        m_StateName[4] = "Right Leg";
        m_StateName[5] = "Left Arm";
        m_StateName[6] = "Right Arm";
        m_StateName[7] = "Left Hand";
        m_StateName[8] = "Right Hand";

        for (int i = 0; i < 64; ++i)
        {
            m_Names[i] = "";
        }

        m_Names[0] = "Pelvis x";
        m_Names[1] = "Pelvis y";
        m_Names[2] = "Pelvis z";
        m_Names[6] = "Left Foot x";
        m_Names[7] = "Left Foot y";
        m_Names[8] = "Left Foot z";
        m_Names[9] = "Left Leg Turn";
        m_Names[10] = "Left Toe";
        m_Names[16] = "Left Hand x";
        m_Names[17] = "Left Hand y";
        m_Names[18] = "Left Hand z";
        m_Names[19] = "Left Arm Turn";
        m_Names[11] = "Right Foot x";
        m_Names[12] = "Right Foot y";
        m_Names[13] = "Right Foot z";
        m_Names[14] = "Right Leg Turn";
        m_Names[15] = "Right Toe";
        m_Names[20] = "Right Hand x";
        m_Names[21] = "Right Hand y";
        m_Names[22] = "Right Hand z";
        m_Names[23] = "Right Arm Turn";
        m_Names[24] = "Left F00";
        m_Names[25] = "Left F01";
        m_Names[26] = "Left F02";
        m_Names[27] = "Left F0C";
        m_Names[28] = "Left F10";
        m_Names[29] = "Left F11";
        m_Names[30] = "Left F12";
        m_Names[31] = "Left F1C";
        m_Names[32] = "Left F20";
        m_Names[33] = "Left F21";
        m_Names[34] = "Left F22";
        m_Names[35] = "Left F2C";
        m_Names[36] = "Left F30";
        m_Names[37] = "Left F31";
        m_Names[38] = "Left F32";
        m_Names[39] = "Left F3C";
        m_Names[40] = "Left F40";
        m_Names[41] = "Left F41";
        m_Names[42] = "Left F42";
        m_Names[43] = "Left F4C";
        m_Names[44] = "Right F00";
        m_Names[45] = "Right F01";
        m_Names[46] = "Right F02";
        m_Names[47] = "Right F0C";
        m_Names[48] = "Right F10";
        m_Names[49] = "Right F11";
        m_Names[50] = "Right F12";
        m_Names[51] = "Right F1C";
        m_Names[52] = "Right F20";
        m_Names[53] = "Right F21";
        m_Names[54] = "Right F22";
        m_Names[55] = "Right F2C";
        m_Names[56] = "Right F30";
        m_Names[57] = "Right F31";
        m_Names[58] = "Right F32";
        m_Names[59] = "Right F3C";
        m_Names[60] = "Right F40";
        m_Names[61] = "Right F41";
        m_Names[62] = "Right F42";
        m_Names[63] = "Right F4C";

        m_FloatCount[0] = 3;
        m_FloatCount[1] = 3;
        m_FloatCount[2] = 0;
        m_FloatCount[3] = 5;
        m_FloatCount[4] = 5;
        m_FloatCount[5] = 4;
        m_FloatCount[6] = 4;
        m_FloatCount[7] = 20;
        m_FloatCount[8] = 20;

        for (int i = 0; i < 9; ++i)
        {
            m_QuatCount[i] = 0;
        }
        m_QuatCount[0] = 1;
        m_QuatCount[1] = 1;
        m_QuatCount[2] = 2;
        m_QuatCount[3] = 1;
        m_QuatCount[4] = 1;
        m_QuatCount[5] = 2;
        m_QuatCount[6] = 2;

        m_FloatStart[0] = 0;
        m_QuatStart[0] = 0;

        for (int j = 1; j < 9; ++j)
        {
            m_FloatStart[j] = m_FloatStart[j - 1] + m_FloatCount[j - 1];
            m_QuatStart[j] = m_QuatStart[j - 1] + m_QuatCount[j - 1];
        }

        SetCompressionRatio(1.0f);
    }

    void ZHumanState::SetCompressionRatio(float fRatio)
    {
        if (firstRun)
        {
            firstRun = 0;
            InitStaticData();
        }

        const float invRatio = 1.0f / fRatio;

        for (int i = 0; i < 64; ++i)
        {
            m_DataInfo[10 + i].m_Type = eAngle;
            m_DataInfo[10 + i].m_Prec = 0.005f * invRatio;
            m_StateId2Pos[10 + i] = static_cast<int16_t>(i + 40);
        }

        for (int i = 0; i < 10; ++i)
        {
            m_DataInfo[i].m_Type = eQuat;
            m_DataInfo[i].m_Prec = 0.0f;
            m_StateId2Pos[i] = static_cast<int16_t>(4 * i);
        }

        m_DataInfo[1].m_Prec = 10.0f;
        m_DataInfo[2].m_Prec = 10.0f;
        m_DataInfo[3].m_Prec = 10.0f;
        m_DataInfo[4].m_Prec = 10.0f;
        m_DataInfo[5].m_Prec = 10.0f;
        m_DataInfo[6].m_Prec = 10.0f;
        m_DataInfo[8].m_Prec = 10.0f;
        m_DataInfo[0].m_Prec = 14.0f;
        m_DataInfo[7].m_Prec = 14.0f;
        m_DataInfo[9].m_Prec = 14.0f;

        m_DataInfo[10].m_Prec = 0.0005f * invRatio;
        m_DataInfo[11].m_Prec = m_DataInfo[10].m_Prec;
        m_DataInfo[12].m_Prec = m_DataInfo[10].m_Prec;
        m_DataInfo[13].m_Prec = m_DataInfo[10].m_Prec;
        m_DataInfo[14].m_Prec = m_DataInfo[10].m_Prec;
        m_DataInfo[15].m_Prec = m_DataInfo[10].m_Prec;

        m_DataInfo[16].m_Prec = 0.05f * invRatio;
        m_DataInfo[17].m_Prec = 0.05f * invRatio;
        m_DataInfo[18].m_Prec = 0.05f * invRatio;
        m_DataInfo[21].m_Prec = 0.05f * invRatio;
        m_DataInfo[22].m_Prec = 0.05f * invRatio;
        m_DataInfo[23].m_Prec = 0.05f * invRatio;

        m_DataInfo[26].m_Prec = 0.0001f * invRatio;
        m_DataInfo[27].m_Prec = m_DataInfo[26].m_Prec;
        m_DataInfo[28].m_Prec = m_DataInfo[26].m_Prec;
        m_DataInfo[30].m_Prec = m_DataInfo[26].m_Prec;
        m_DataInfo[31].m_Prec = m_DataInfo[26].m_Prec;
        m_DataInfo[32].m_Prec = m_DataInfo[26].m_Prec;

        m_DataInfo[0].m_Type = eQuat;
        m_DataInfo[1].m_Type = eQuat;
        m_DataInfo[2].m_Type = eQuat;
        m_DataInfo[3].m_Type = eQuat;
        m_DataInfo[4].m_Type = eQuat;
        m_DataInfo[5].m_Type = eQuat;
        m_DataInfo[6].m_Type = eQuat;
        m_DataInfo[7].m_Type = eQuat;
        m_DataInfo[8].m_Type = eQuat;
        m_DataInfo[9].m_Type = eQuat;

        m_DataInfo[10].m_Type = eGenericFloat;
        m_DataInfo[11].m_Type = eGenericFloat;
        m_DataInfo[12].m_Type = eGenericFloat;
        m_DataInfo[13].m_Type = eGenericFloat;
        m_DataInfo[14].m_Type = eGenericFloat;
        m_DataInfo[15].m_Type = eGenericFloat;
        m_DataInfo[16].m_Type = eGenericFloat;
        m_DataInfo[17].m_Type = eGenericFloat;
        m_DataInfo[18].m_Type = eGenericFloat;
        m_DataInfo[21].m_Type = eGenericFloat;
        m_DataInfo[22].m_Type = eGenericFloat;
        m_DataInfo[23].m_Type = eGenericFloat;
        m_DataInfo[26].m_Type = eGenericFloat;
        m_DataInfo[27].m_Type = eGenericFloat;
        m_DataInfo[28].m_Type = eGenericFloat;
        m_DataInfo[30].m_Type = eGenericFloat;
        m_DataInfo[31].m_Type = eGenericFloat;
        m_DataInfo[32].m_Type = eGenericFloat;
    }

    void ZHumanState::NukeStaticData()
    {
        ZHumanState::firstRun = 1;
    }
}
