#include <Glacier/Materials/BS_Runtime.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>


namespace
{
    using namespace Glacier;
    using namespace Glacier::BS_Runtime;

    struct MaterialDefDesc
    {
        std::string Name;
        int ID = 0;
    };

    struct PropertyDefDesc
    {
        std::string Name;
        int Type = -1;
        int Offset = -1;
        int EnumIndex = -1;
    };

    struct EnumDefDesc
    {
        std::string Name;
        std::vector<std::string> Values;
    };

    struct BuiltDB
    {
        std::vector<uint8_t> Buffer;
        uint32_t DataRegion = 0;
    };

    // Builds an in-memory MaterialDescription database matching the layout the
    // reversed BS_Runtime code expects. SRoot/ZReference offsets are relative to
    // the bytestream base; ZArray::m_Data and ZStream::m_Data are stored as
    // absolute pointers (the code dereferences them directly).
    BuiltDB BuildMaterialDB(
        const std::vector<MaterialDefDesc>& materials,
        const std::vector<PropertyDefDesc>& properties,
        const std::vector<EnumDefDesc>& enums,
        const std::vector<uint8_t>& materialData,
        uint32_t materialSize)
    {
        const uint32_t nMaterials = static_cast<uint32_t>(materials.size());
        const uint32_t nProperties = static_cast<uint32_t>(properties.size());
        const uint32_t nEnums = static_cast<uint32_t>(enums.size());

        const uint32_t root = 0;
        const uint32_t materialDefArray = 0x18;
        const uint32_t materialDefs = materialDefArray + 8;
        const uint32_t propertyDefArray = materialDefs + nMaterials * 8;
        const uint32_t propertyDefs = propertyDefArray + 8;
        const uint32_t enumDefArray = propertyDefs + nProperties * 16;
        const uint32_t enumDefs = enumDefArray + 8;

        std::vector<std::string> strings;
        auto stringId = [&strings](const std::string& s) {
            const auto it = std::find(strings.begin(), strings.end(), s);
            if (it != strings.end())
                return static_cast<int>(it - strings.begin());
            strings.push_back(s);
            return static_cast<int>(strings.size() - 1);
        };

        std::vector<int> materialNameIds;
        materialNameIds.reserve(materials.size());
        for (const auto& m : materials)
            materialNameIds.push_back(stringId(m.Name));

        std::vector<int> propertyNameIds;
        propertyNameIds.reserve(properties.size());
        for (const auto& p : properties)
            propertyNameIds.push_back(stringId(p.Name));

        std::vector<int> enumNameIds;
        enumNameIds.reserve(enums.size());
        for (const auto& e : enums)
            enumNameIds.push_back(stringId(e.Name));

        std::vector<int> enumValueNameIds;
        for (const auto& e : enums)
            for (const auto& v : e.Values)
                enumValueNameIds.push_back(stringId(v));

        // Enum value arrays are laid out one after another: each is a ZArray
        // header (8 bytes) followed by its SEnumValueDef elements (8 bytes each).
        uint32_t valueArrayCursor = enumDefs + nEnums * 8;
        std::vector<uint32_t> enumValueArrayOffsets;
        std::vector<uint32_t> enumValueSizes;
        for (const auto& e : enums)
        {
            enumValueArrayOffsets.push_back(valueArrayCursor);
            enumValueSizes.push_back(static_cast<uint32_t>(e.Values.size()));
            valueArrayCursor += 8 + static_cast<uint32_t>(e.Values.size()) * 8;
        }

        const uint32_t stringTable = valueArrayCursor;
        const uint32_t stringRefs = stringTable + 8;
        const uint32_t stringsStart = stringRefs + static_cast<uint32_t>(strings.size()) * 4;

        uint32_t stringsData = stringsStart;
        for (const auto& s : strings)
            stringsData += static_cast<uint32_t>(s.size()) + 1;

        const uint32_t materialDataStream = stringsData;
        const uint32_t dataRegion = materialDataStream + 4;

        BuiltDB db;
        db.Buffer.resize(dataRegion + materialData.size());
        db.DataRegion = dataRegion;

        uint8_t* const base = db.Buffer.data();
        auto writeAt = [base](uint32_t offset, const void* data, size_t size) {
            std::memcpy(base + offset, data, size);
        };
        auto writeU32 = [&writeAt](uint32_t offset, uint32_t value) {
            writeAt(offset, &value, sizeof(value));
        };
        auto writePtr = [base, &writeAt](uint32_t offset, uint32_t target) {
            const uintptr_t addr = reinterpret_cast<uintptr_t>(base) + target;
            writeAt(offset, &addr, sizeof(addr));
        };

        // SRoot
        writeU32(root + 0x00, materialDefArray);
        writeU32(root + 0x04, propertyDefArray);
        writeU32(root + 0x08, enumDefArray);
        writeU32(root + 0x0C, materialSize);
        writeU32(root + 0x10, stringTable);
        writeU32(root + 0x14, materialDataStream);

        // Material definitions
        writeU32(materialDefArray + 0x00, nMaterials);
        writePtr(materialDefArray + 0x04, materialDefs);
        for (uint32_t i = 0; i < nMaterials; ++i)
        {
            const uint32_t off = materialDefs + i * 8;
            writeU32(off + 0x00, static_cast<uint32_t>(materialNameIds[i]));
            writeU32(off + 0x04, static_cast<uint32_t>(materials[i].ID));
        }

        // Property definitions
        writeU32(propertyDefArray + 0x00, nProperties);
        writePtr(propertyDefArray + 0x04, propertyDefs);
        for (uint32_t i = 0; i < nProperties; ++i)
        {
            const uint32_t off = propertyDefs + i * 16;
            writeU32(off + 0x00, static_cast<uint32_t>(propertyNameIds[i]));
            writeU32(off + 0x04, static_cast<uint32_t>(properties[i].Type));
            writeU32(off + 0x08, static_cast<uint32_t>(properties[i].Offset));
            writeU32(off + 0x0C, static_cast<uint32_t>(properties[i].EnumIndex));
        }

        // Enum definitions and their value arrays
        writeU32(enumDefArray + 0x00, nEnums);
        writePtr(enumDefArray + 0x04, enumDefs);
        uint32_t enumValueNameCursor = 0;
        for (uint32_t i = 0; i < nEnums; ++i)
        {
            const uint32_t nValues = enumValueSizes[i];
            const uint32_t valueArrayHeader = enumValueArrayOffsets[i];
            const uint32_t valueElements = valueArrayHeader + 8;

            const uint32_t off = enumDefs + i * 8;
            writeU32(off + 0x00, static_cast<uint32_t>(enumNameIds[i]));
            writeU32(off + 0x04, valueArrayHeader);

            writeU32(valueArrayHeader + 0x00, nValues);
            writePtr(valueArrayHeader + 0x04, valueElements);

            for (uint32_t v = 0; v < nValues; ++v)
            {
                const uint32_t voff = valueElements + v * 8;
                writeU32(voff + 0x00, static_cast<uint32_t>(enumValueNameIds[enumValueNameCursor + v]));
                writeU32(voff + 0x04, v);
            }

            enumValueNameCursor += nValues;
        }

        // String DB
        writeU32(stringTable + 0x00, static_cast<uint32_t>(strings.size()));
        writePtr(stringTable + 0x04, stringRefs);
        uint32_t stringCursor = stringsStart;
        for (size_t i = 0; i < strings.size(); ++i)
        {
            writeU32(stringRefs + static_cast<uint32_t>(i) * 4, stringCursor);
            std::memcpy(base + stringCursor, strings[i].c_str(), strings[i].size() + 1);
            stringCursor += static_cast<uint32_t>(strings[i].size()) + 1;
        }

        // Material data stream (ZStream) and payload
        writePtr(materialDataStream, dataRegion);
        std::memcpy(base + dataRegion, materialData.data(), materialData.size());

        return db;
    }

    std::vector<uint8_t> BytesOf(const std::vector<int32_t>& values)
    {
        std::vector<uint8_t> bytes(values.size() * sizeof(int32_t));
        std::memcpy(bytes.data(), values.data(), bytes.size());
        return bytes;
    }
}

TEST(BS_Runtime_RemapGeoms, NonMapScenePropertiesAreRemappedOthersUntouched)
{
    auto db = BuildMaterialDB(
        { { "M1", 1 }, { "M2", 2 } },
        { { "Scene", PT_ResID_Scene, 0, -1 }, { "Int", PT_Int, 8, -1 } },
        {},
        BytesOf({ 3, 5, 7, 9 }), // scene mat1, scene mat2, int mat1, int mat2
        4);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint32_t remaps[] = { 10, 20, 30, 40, 50, 60, 70, 80 };
    desc.RemapGeoms(const_cast<uint32_t*>(remaps));

    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 1, 0), 30); // 3 -> remaps[2]
    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 2, 0), 50); // 5 -> remaps[4]
    EXPECT_EQ(*desc.GetProperty<int>(PT_Int, 1, 1), 7);          // untouched
    EXPECT_EQ(*desc.GetProperty<int>(PT_Int, 2, 1), 9);          // untouched
}

TEST(BS_Runtime_RemapGeoms, NegativeValueBecomesZero)
{
    auto db = BuildMaterialDB(
        { { "M1", 1 } },
        { { "Scene", PT_ResID_Scene, 0, -1 } },
        {},
        BytesOf({ -1 }),
        4);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint32_t remaps[] = { 10, 20 };
    desc.RemapGeoms(const_cast<uint32_t*>(remaps));

    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 1, 0), 0); // -1 -> 0
}

TEST(BS_Runtime_RemapGeoms, MapScenePropertiesRemapEveryEnumEntry)
{
    // Map data layout at the property offset:
    //   [stride dword][value for enum 0][enum 1][enum 2]...
    std::vector<uint8_t> data(16, 0);
    const uint32_t stride = 4;
    std::memcpy(data.data(), &stride, sizeof(stride));
    const int32_t enumValues[] = { 2, -1, 6 };
    for (size_t i = 0; i < 3; ++i)
        std::memcpy(data.data() + 4 + i * sizeof(int32_t), &enumValues[i], sizeof(int32_t));

    auto db = BuildMaterialDB(
        { { "M1", 1 } },
        { { "MapScene", PT_ResID_Scene, 0, 0 } },
        { { "E", { "A", "B", "C" } } },
        data,
        16);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint32_t remaps[] = { 10, 20, 30, 40, 50, 60, 70 };
    desc.RemapGeoms(const_cast<uint32_t*>(remaps));

    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 1, 0, 0), 20); // 2 -> remaps[1]
    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 1, 0, 1), 0);  // -1 -> 0
    EXPECT_EQ(*desc.GetProperty<int>(PT_ResID_Scene, 1, 0, 2), 60); // 6 -> remaps[5]
}

TEST(BS_Runtime_RemapGeoms, EmptyDatabaseDoesNothing)
{
    auto db = BuildMaterialDB({}, {}, {}, {}, 0);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint32_t remaps[] = { 10 };
    EXPECT_NO_THROW(desc.RemapGeoms(const_cast<uint32_t*>(remaps)));
}

TEST(BS_Runtime_GetProperty_, NonMapReturnsSlotInsideMaterialData)
{
    auto db = BuildMaterialDB(
        { { "M1", 1 }, { "M2", 2 } },
        { { "Scene", PT_ResID_Scene, 4, -1 } },
        {},
        std::vector<uint8_t>(8, 0),
        4);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint8_t* dataBase = db.Buffer.data() + db.DataRegion;
    EXPECT_EQ(desc.GetProperty_(1, 0), dataBase + 4 + 0); // (matId - 1) * materialSize
    EXPECT_EQ(desc.GetProperty_(2, 0), dataBase + 4 + 4);
}

TEST(BS_Runtime_GetProperty_, MapReturnsEntryByStrideAndEnumId)
{
    std::vector<uint8_t> data(16, 0);
    const uint32_t stride = 4;
    std::memcpy(data.data(), &stride, sizeof(stride));

    auto db = BuildMaterialDB(
        { { "M1", 1 } },
        { { "MapScene", PT_ResID_Scene, 0, 0 } },
        { { "E", { "A", "B", "C" } } },
        data,
        16);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    const uint8_t* dataBase = db.Buffer.data() + db.DataRegion;
    EXPECT_EQ(desc.GetProperty_(1, 0, 0), dataBase + 4 + 4 * 0);
    EXPECT_EQ(desc.GetProperty_(1, 0, 1), dataBase + 4 + 4 * 1);
    EXPECT_EQ(desc.GetProperty_(1, 0, 2), dataBase + 4 + 4 * 2);
}

TEST(BS_Runtime_NameLookup, PropertyMaterialAndEnumByName)
{
    auto db = BuildMaterialDB(
        { { "Metal", 1 } },
        { { "Scene", PT_ResID_Scene, 0, -1 } },
        { { "EType", { "low", "High" } } },
        std::vector<uint8_t>(4, 0),
        4);

    ZMaterialDescriptionDB desc;
    desc.Init(db.Buffer.data());

    EXPECT_EQ(desc.GetPropertyId("Scene"), 0);
    EXPECT_EQ(desc.GetMaterialDescriptionId("Metal").m_Value, 1);
    EXPECT_STREQ(desc.GetMaterialDescriptionName(TMaterialDescID{ 1 }), "Metal");
    EXPECT_EQ(desc.GetEnumId("EType", "High").m_Value, 1);
}
