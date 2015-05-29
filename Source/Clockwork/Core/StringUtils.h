#pragma once

#include "../Core/Variant.h"

namespace Clockwork
{

/// Parse a bool from a string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
CLOCKWORK_API bool ToBool(const String& source);
/// Parse a bool from a C string. Check for the first non-empty character (converted to lowercase) being either 't', 'y' or '1'.
CLOCKWORK_API bool ToBool(const char* source);
/// Parse a float from a string.
CLOCKWORK_API float ToFloat(const String& source);
/// Parse a float from a C string.
CLOCKWORK_API float ToFloat(const char* source);
/// Parse an integer from a string.
CLOCKWORK_API int ToInt(const String& source);
/// Parse an integer from a C string.
CLOCKWORK_API int ToInt(const char* source);
/// Parse an unsigned integer from a string.
CLOCKWORK_API unsigned ToUInt(const String& source);
/// Parse an unsigned integer from a C string.
CLOCKWORK_API unsigned ToUInt(const char* source);
/// Parse a Color from a string.
CLOCKWORK_API Color ToColor(const String& source);
/// Parse a Color from a C string.
CLOCKWORK_API Color ToColor(const char* source);
/// Parse an IntRect from a string.
CLOCKWORK_API IntRect ToIntRect(const String& source);
/// Parse an IntRect from a C string.
CLOCKWORK_API IntRect ToIntRect(const char* source);
/// Parse an IntVector2 from a string.
CLOCKWORK_API IntVector2 ToIntVector2(const String& source);
/// Parse an IntVector2 from a C string.
CLOCKWORK_API IntVector2 ToIntVector2(const char* source);
/// Parse a Quaternion from a string. If only 3 components specified, convert Euler angles (degrees) to quaternion.
CLOCKWORK_API Quaternion ToQuaternion(const String& source);
/// Parse a Quaternion from a C string. If only 3 components specified, convert Euler angles (degrees) to quaternion.
CLOCKWORK_API Quaternion ToQuaternion(const char* source);
/// Parse a Rect from a string.
CLOCKWORK_API Rect ToRect(const String& source);
/// Parse a Rect from a C string.
CLOCKWORK_API Rect ToRect(const char* source);
/// Parse a Vector2 from a string.
CLOCKWORK_API Vector2 ToVector2(const String& source);
/// Parse a Vector2 from a C string.
CLOCKWORK_API Vector2 ToVector2(const char* source);
/// Parse a Vector3 from a string.
CLOCKWORK_API Vector3 ToVector3(const String& source);
/// Parse a Vector3 from a C string.
CLOCKWORK_API Vector3 ToVector3(const char* source);
/// Parse a Vector4 from a string.
CLOCKWORK_API Vector4 ToVector4(const String& source, bool allowMissingCoords = false);
/// Parse a Vector4 from a C string.
CLOCKWORK_API Vector4 ToVector4(const char* source, bool allowMissingCoords = false);
/// Parse a float, Vector or Matrix variant from a string.
CLOCKWORK_API Variant ToVectorVariant(const String& source);
/// Parse a float, Vector or Matrix variant from a C string.
CLOCKWORK_API Variant ToVectorVariant(const char* source);
/// Parse a Matrix3 from a string.
CLOCKWORK_API Matrix3 ToMatrix3(const String& source);
/// Parse a Matrix3 from a C string.
CLOCKWORK_API Matrix3 ToMatrix3(const char* source);
/// Parse a Matrix3x4 from a string.
CLOCKWORK_API Matrix3x4 ToMatrix3x4(const String& source);
/// Parse a Matrix3x4 from a C string.
CLOCKWORK_API Matrix3x4 ToMatrix3x4(const char* source);
/// Parse a Matrix4 from a string.
CLOCKWORK_API Matrix4 ToMatrix4(const String& source);
/// Parse a Matrix4 from a C string.
CLOCKWORK_API Matrix4 ToMatrix4(const char* source);
/// Convert a pointer to string (returns hexadecimal.)
CLOCKWORK_API String ToString(void* value);
/// Convert an unsigned integer to string as hexadecimal.
CLOCKWORK_API String ToStringHex(unsigned value);
/// Convert a byte buffer to a string.
CLOCKWORK_API void BufferToString(String& dest, const void* data, unsigned size);
/// Convert a string to a byte buffer.
CLOCKWORK_API void StringToBuffer(PODVector<unsigned char>& dest, const String& source);
/// Convert a C string to a byte buffer.
CLOCKWORK_API void StringToBuffer(PODVector<unsigned char>& dest, const char* source);
/// Return an index to a string list corresponding to the given string, or a default value if not found. The string list must be empty-terminated.
CLOCKWORK_API unsigned GetStringListIndex(const String& value, const String* strings, unsigned defaultIndex, bool caseSensitive = false);
/// Return an index to a string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
CLOCKWORK_API unsigned GetStringListIndex(const char* value, const String* strings, unsigned defaultIndex, bool caseSensitive = false);
/// Return an index to a C string list corresponding to the given C string, or a default value if not found. The string list must be empty-terminated.
CLOCKWORK_API unsigned GetStringListIndex(const char* value, const char** strings, unsigned defaultIndex, bool caseSensitive = false);
/// Return a formatted string.
CLOCKWORK_API String ToString(const char* formatString, ...);
/// Return whether a char is an alphabet letter.
CLOCKWORK_API bool IsAlpha(unsigned ch);
/// Return whether a char is a digit.
CLOCKWORK_API bool IsDigit(unsigned ch);
/// Return char in uppercase.
CLOCKWORK_API unsigned ToUpper(unsigned ch);
/// Return char in lowercase.
CLOCKWORK_API unsigned ToLower(unsigned ch);

}