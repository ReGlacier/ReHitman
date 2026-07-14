#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/ZToken.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Visitor interface used while walking serialized data streams.
     *
     * Serializer readers call these callbacks for primitive values, references,
     * containers, objects, arrays, skip marks, and stream end notifications. The base
     * implementation is intentionally empty so specialized visitors can override only
     * the events they care about.
     */
    struct ISerializerVisitor
    {
        /** @brief Visits an inline unsigned integer token payload. */
        virtual void Data(const ZToken token, const unsigned int) {}
        /** @brief Visits a double value. */
        virtual void Data(const ZToken token, double*) {}
        /** @brief Visits a float value. */
        virtual void Data(const ZToken token, float*) {}
        /** @brief Visits a signed 32-bit integer value. */
        virtual void Data(const ZToken token, int32_t*) {}
        /** @brief Visits an unsigned 32-bit integer value. */
        virtual void Data(const ZToken token, uint32_t*) {}
        /** @brief Visits a signed 16-bit integer value. */
        virtual void Data(const ZToken token, int16_t*) {}
        /** @brief Visits an unsigned 16-bit integer value. */
        virtual void Data(const ZToken token, uint16_t*) {}
        /** @brief Visits a signed 8-bit integer value. */
        virtual void Data(const ZToken token, int8_t*) {}
        /** @brief Visits an unsigned 8-bit integer value. */
        virtual void Data(const ZToken token, uint8_t*) {}
        /** @brief Visits a boolean value. */
        virtual void Data(const ZToken token, bool*) {};
        /** @brief Visits a string value. */
        virtual void Data(const ZToken token, const char*) {};
        /** @brief Visits a bitfield described by up to 32 string labels. */
        virtual void Bitfield(const ZToken token, const char *(*)[32]) {} // WTF???
        /** @brief Visits a serialized object/reference id. */
        virtual void Reference(const ZToken token, uint32_t) {}
        /** @brief Visits a container with an element count. */
        virtual void Container(const ZToken token, const uint32_t) {}
        /** @brief Called when an object begins. */
        virtual void BeginObject(const ZToken token) {}
        /** @brief Called when an object ends. */
        virtual void EndObject(const ZToken token) {}
        /** @brief Called when an array begins. */
        virtual void BeginArray(const ZToken token, const uint32_t) {}
        /** @brief Called when an array ends. */
        virtual void EndArray(const ZToken token) {}
        /** @brief Called when a stream skip marker is encountered. */
        virtual void Skip() {}
        /** @brief Called at the end of the stream walk. */
        virtual void End() {}
    };

    /**
     * @brief Default no-op serializer visitor.
     */
    struct ZSerializerVisitor : public ISerializerVisitor
    {};

    /**
     * @brief Visitor that skips over a nested object subtree.
     *
     * The visitor tracks object nesting depth. Data callbacks assert when invoked after
     * the skip depth has already reached zero, which indicates the stream parser found
     * data where it expected to still be inside an object.
     */
    struct ZSerializerVisitor_Skip : public ISerializerVisitor
    {
        // vtbl
        void Data(const ZToken token, const unsigned int) override;
        void Data(const ZToken token, double*) override;
        void Data(const ZToken token, float*) override;
        void Data(const ZToken token, int32_t*) override;
        void Data(const ZToken token, uint32_t*) override;
        void Data(const ZToken token, int16_t*) override;
        void Data(const ZToken token, uint16_t*) override;
        void Data(const ZToken token, int8_t*) override;
        void Data(const ZToken token, uint8_t*) override;
        void Data(const ZToken token, bool*)  override;
        void Data(const ZToken token, const char*)  override;
        void Bitfield(const ZToken token, const char *(*)[32]) override;
        void Reference(const ZToken token, uint32_t) override;
        void Container(const ZToken token, const uint32_t) override;
        void BeginObject(const ZToken token) override;
        void EndObject(const ZToken token) override;
        void End() override;

        // methods
        /** @brief Starts skipping with the supplied object nesting depth. */
        ZSerializerVisitor_Skip(uint32_t lDepth);

        // members
        /** @brief Current object nesting depth being skipped. */
        uint32_t m_Depth { 0u };
    };

    /**
     * @brief Skip visitor used for one object.
     *
     * Converts to true while there is still object depth left to skip.
     */
    struct ZSerializerVisitor_SkipObject : public ZSerializerVisitor_Skip
    {
        // methods
        /** @brief Creates a skip-object visitor with zero initial depth. */
        ZSerializerVisitor_SkipObject() 
            : ZSerializerVisitor_Skip(0)
        {
        }

        /** @brief Returns true while the visitor is still inside the skipped object. */
        explicit operator bool() const;
    };

    /**
     * @brief Skip visitor that stops when the next skip mark is found.
     *
     * It starts with depth 1 and remains truthy until Skip() is called. Ending the root
     * object before a skip mark is considered an unexpected end of object.
     */
    struct ZSerializerVisitor_SkipToNextMark : public ZSerializerVisitor_Skip
    {
        // vtbl
        void Skip() override;
        void EndObject(const ZToken token) override;

        // methods
        /** @brief Creates a visitor that skips from the current object to the next mark. */
        ZSerializerVisitor_SkipToNextMark();
        /** @brief Returns true until a skip mark has been found. */
        explicit operator bool() const;

        // members
        /** @brief Set to true once Skip() is observed. */
        bool m_SkipFound { false };
    };
}
