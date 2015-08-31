//
// Copyright (c) 2008-2015 the Clockwork project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

namespace Clockwork
{

/// JSON value type.
enum JSONValueType
{
    /// JSON null type.
    JSON_NULL = 0,
    /// JSON boolean type.
    JSON_BOOL,
    /// JSON number type.
    JSON_NUMBER,
    /// JSON string type.
    JSON_STRING,
    /// JSON array type.
    JSON_ARRAY,
    /// JSON object type.
    JSON_OBJECT,
};

class JSONValue;

/// JSON array type.
typedef Vector<JSONValue> JSONArray;
/// JSON object type.
typedef HashMap<String, JSONValue> JSONObject;
/// JSON object iterator.
typedef JSONObject::Iterator JSONObjectIterator;
/// Constant JSON object iterator.
typedef JSONObject::ConstIterator ConstJSONObjectIterator;

/// JSON value class.
class CLOCKWORK_API JSONValue
{
public:
    /// Construct null value.
    JSONValue() : 
        valueType_(JSON_NULL)
    {
    }
    /// Construct with a boolean.
    JSONValue(bool value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a integer.
    JSONValue(int value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a unsigned integer.
    JSONValue(unsigned value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a float.
    JSONValue(float value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a double.
    JSONValue(double value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a string.
    JSONValue(const String& value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a C string.
    JSONValue(const char* value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a JSON array.
    JSONValue(const JSONArray& value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Construct with a JSON object.
    JSONValue(const JSONObject& value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }    
    /// Copy-construct from another JSON value.
    JSONValue(const JSONValue& value) :
        valueType_(JSON_NULL)
    {
        *this = value;
    }
    /// Destruct.
    ~JSONValue()
    {
        SetType(JSON_NULL);
    }

    /// Set value type.
    void SetType(JSONValueType valueType);
    /// Assign from a boolean.
    JSONValue& operator =(bool rhs);
    /// Assign from an integer.
    JSONValue& operator =(int rhs);
    /// Assign from an unsigned integer.
    JSONValue& operator =(unsigned rhs);
    /// Assign from a float.
    JSONValue& operator =(float rhs);
    /// Assign from a double.
    JSONValue& operator =(double rhs);
    /// Assign from a string.
    JSONValue& operator =(const String& rhs);
    /// Assign from a C string.
    JSONValue& operator =(const char* rhs);
    /// Assign from a JSON array.
    JSONValue& operator =(const JSONArray& rhs);
    /// Assign from a JSON object.
    JSONValue& operator =(const JSONObject& rhs);
    /// Assign from another JSON value.
    JSONValue& operator =(const JSONValue& rhs);

    /// Return value type.
    JSONValueType GetType() const { return valueType_; }
    /// Check is null.
    bool IsNull() const { return valueType_ == JSON_NULL; }
    /// Check is boolean.
    bool IsBool() const { return valueType_ == JSON_BOOL; }
    /// Check is number.
    bool IsNumber() const { return valueType_ == JSON_NUMBER; }
    /// Check is string.
    bool IsString() const { return valueType_ == JSON_STRING; }
    /// Check is array.
    bool IsArray() const { return valueType_ == JSON_ARRAY; }
    /// Check is object.
    bool IsObject() const { return valueType_ == JSON_OBJECT; }

    /// Return boolean value.
    bool GetBool() const { return IsBool() ? boolValue_ : false;}
    /// Return integer value.
    int GetInt() const { return IsNumber() ? (int)numberValue_ : 0; }
    /// Return unsigned integer value.
    unsigned GetUint() const { return IsNumber() ? (unsigned)numberValue_ : 0; }
    /// Return float value.
    float GetFloat() const { return IsNumber() ? (float)numberValue_ : 0.0f; }
    /// Return double value.
    double GetDouble() const { return IsNumber() ? numberValue_ : 0.0; }
    /// Return string value.
    const String& GetString() const { return IsString() ? *stringValue_ : String::EMPTY;}
    /// Return C string value.
    const char* GetCString() const { return IsString() ? stringValue_->CString() : 0;}
    /// Return JSON array value.
    const JSONArray& GetArray() const { return IsArray() ? *arrayValue_ : emptyArray; }
    /// Return JSON object value.
    const JSONObject& GetObject() const { return IsObject() ? *objectValue_ : emptyObject; }

    // JSON array functions
    /// Return JSON value at index.
    JSONValue& operator [](unsigned index);
    /// Return JSON value at index.
    const JSONValue& operator [](unsigned index) const;
    /// Add JSON value at end.
    void Push(const JSONValue& value);
    /// Remove the last JSON value.
    void Pop();
    /// Insert an JSON value at position.
    void Insert(unsigned pos, const JSONValue& value);
    /// Erase a range of JSON values.
    void Erase(unsigned pos, unsigned length = 1);
    /// Resize array.
    void Resize(unsigned newSize);
    /// Return size of array.
    unsigned Size() const;

    // JSON object functions
    /// Return JSON value with key.
    JSONValue& operator [](const String& key);
    /// Return JSON value with key.
    const JSONValue& operator [](const String& key) const;
    /// Set JSON value with key.
    void Set(const String& key, const JSONValue& value);
    /// Return JSON value with key.
    const JSONValue& Get(const String& key) const;
    /// Erase a pair by key.
    bool Erase(const String& key);
    /// Return whether contains a pair with key.
    bool Contains(const String& key) const;
    /// Return iterator to the beginning.
    JSONObjectIterator Begin();
    /// Return iterator to the beginning.
    ConstJSONObjectIterator Begin() const;
    /// Return iterator to the end.
    JSONObjectIterator End();
    /// Return iterator to the beginning.
    ConstJSONObjectIterator End() const;

    /// Clear array or object.
    void Clear();
    
    /// Empty JSON value.
    static const JSONValue EMPTY;
    /// Empty JSON array.
    static const JSONArray emptyArray;
    /// Empty JSON object.
    static const JSONObject emptyObject;

private:
    /// Value type.
    JSONValueType valueType_;
    /// Values.
    union
    {
        /// Boolean value.
        bool boolValue_;
        /// Number value.
        double numberValue_;
        /// String value.
        String* stringValue_;
        /// Array value.
        JSONArray* arrayValue_;
        /// Object value.
        JSONObject* objectValue_;
    };
};

}