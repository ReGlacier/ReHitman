#pragma once


namespace Glacier
{
    class StringStreamLogger_t
    {
    public:
        // vtbl
        virtual void Print(const char* pszText) = 0;
        virtual ~StringStreamLogger_t();
        // TODO: Finish me

        // methods
        // members
    };

    class StdStrLogger_t : public StringStreamLogger_t
    {
    public:
        // vtbl
        void Print(const char* pszText) override;
        ~StdStrLogger_t() override;
        // TODO: Finish me
    };

    class Win32FileLogger_t : public StringStreamLogger_t
    {
    public:
        // vtbl
        void Print(const char* pszText) override;
        ~Win32FileLogger_t() override;
        // TODO: Finish me
    };
    // Verified alloc 0x6C
}
