#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class CSharedCom;
    struct CComRead;

    /**
     * @struct CComRead
     * @brief A lightweight read-only proxy that provides implicit type conversions
     *        for values stored in a @ref CSharedCom (or its subclasses).
     *
     * @details
     * `CComRead` is the return type of @ref CSharedCom::Get and
     * @ref CSharedCom::operator[]. It stores a reference to the COM container
     * and the value name, deferring the actual lookup until an implicit
     * conversion operator is invoked. This enables concise syntax such as:
     *
     * @code
     * CSharedCom* pCom = ...;
     * int    health = (*pCom)["iHealth"];
     * float  speed  = pCom->Get("fSpeed");
     * bool   alive  = (*pCom)["bAlive"];
     * @endcode
     *
     * Each conversion operator calls the appropriate `GetVal` overload on
     * the associated @ref CSharedCom. If the value is not found, a safe
     * default is returned (0 / 0.0f / false / nullptr).
     *
     * @note The `mutable` qualifier on @ref m_pCom is a temporary measure
     *       used during the reimplementation effort; it will be removed once
     *       the full `CSharedCom` interface is finalized.
     */
    struct CComRead
	{
		// --- methods ---------------------------------------------------------

		/**
		 * @brief Constructs a CComRead bound to a specific COM and value name.
		 *
		 * @param pCom  Pointer to the COM container (must not be null during conversion).
		 * @param pName Null-terminated name of the value to read.
		 *
		 * @code
		 * CComRead reader(pMyCom, "fSpeed");
		 * float speed = reader; // invokes operator float()
		 * @endcode
		 */
		CComRead(CSharedCom* pCom, const char* pName);

		/**
		 * @brief Copy constructor. Creates a shallow copy sharing the same
		 *        COM pointer and value name.
		 *
		 * @param copy The source CComRead to copy from.
		 */
		CComRead(const CComRead& copy);

		/**
		 * @brief Implicit conversion to `uint32_t`.
		 *
		 * @return The value as an unsigned 32-bit integer (read via `GetVal(int*)` and cast).
		 *
		 * @code
		 * CComRead reader(pCom, "iFlags");
		 * uint32_t flags = reader;
		 * @endcode
		 */
		operator uint32_t() const;

		/**
		 * @brief Implicit conversion to `int32_t`.
		 *
		 * @return The value as a signed 32-bit integer.
		 *
		 * @code
		 * int ammo = (*pCom)["iAmmo"];
		 * @endcode
		 */
		operator int32_t() const;

		/**
		 * @brief Implicit conversion to `float`.
		 *
		 * @return The value as a single-precision float.
		 *
		 * @code
		 * float speed = pCom->Get("fSpeed");
		 * @endcode
		 */
		operator float() const;

		/**
		 * @brief Implicit conversion to `char*`.
		 *
		 * @return Pointer to the raw string data, or `nullptr` if not found.
		 *
		 * @code
		 * const char* name = (*pCom)["sPlayerName"];
		 * printf("Player: %s\n", name ? name : "unknown");
		 * @endcode
		 */
		operator char*() const;

		/**
		 * @brief Implicit conversion to `bool`.
		 *
		 * @return The value as a boolean.
		 *
		 * @code
		 * if ((*pCom)["bEnabled"])
		 *     printf("Feature is on\n");
		 * @endcode
		 */
		operator bool() const;

		// --- members ---------------------------------------------------------

		/**
		 * @brief Pointer to the COM container that owns the value.
		 *
		 * @note Marked `mutable` to allow const CComRead objects to call
		 *       non-const `GetVal` on the COM during conversion.
		 */
		mutable CSharedCom* m_pCom;

		/**
		 * @brief Null-terminated name of the value to read.
		 */
		const char* m_pName;
	};
}
