//
// Copyright (c) 2008-2015 the Clockwork project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:addmember
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

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../IO/Log.h"
#include "../Resource/JSONValue.h"

#include "../DebugNew.h"

namespace Clockwork
{

const JSONValue JSONValue::EMPTY;
const JSONArray JSONValue::emptyArray;
const JSONObject JSONValue::emptyObject;

JSONValue& JSONValue::operator =(bool rhs)
{
    SetType(JSON_BOOL);
    boolValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(int rhs)
{
    SetType(JSON_NUMBER, JSONNT_INT);
    numberValue_ = rhs;    

    return *this;
}

JSONValue& JSONValue::operator =(unsigned rhs)
{
    SetType(JSON_NUMBER, JSONNT_UINT);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(float rhs)
{
    SetType(JSON_NUMBER, JSONNT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(double rhs)
{
    SetType(JSON_NUMBER, JSONNT_FLOAT_DOUBLE);
    numberValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const String& rhs)
{
    SetType(JSON_STRING);
    *stringValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const char* rhs)
{
    SetType(JSON_STRING);
    *stringValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONArray& rhs)
{
    SetType(JSON_ARRAY);
    *arrayValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONObject& rhs)
{
    SetType(JSON_OBJECT);
    *objectValue_ = rhs;

    return *this;
}

JSONValue& JSONValue::operator =(const JSONValue& rhs)
{
    if (this == &rhs)
        return *this;

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
    case JSON_BOOL:
        boolValue_ = rhs.boolValue_;
        break;

    case JSON_NUMBER:
        numberValue_ = rhs.numberValue_;
        break;

    case JSON_STRING:
        *stringValue_ = *rhs.stringValue_;
        break;

    case JSON_ARRAY:
        *arrayValue_ = *rhs.arrayValue_;
        break;

    case JSON_OBJECT:
        *objectValue_ = *rhs.objectValue_;

    default:
        break;
    }

    return *this;
}

JSONValueType JSONValue::GetValueType() const
{
    return (JSONValueType)(type_ >> 16);
}

JSONNumberType JSONValue::GetNumberType() const
{
    return (JSONNumberType)(type_ & 0xffff);
}

JSONValue& JSONValue::operator [](unsigned index)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    return (*arrayValue_)[index];
}

const JSONValue& JSONValue::operator [](unsigned index) const
{
    if (GetValueType() != JSON_ARRAY)
        return EMPTY;

    return (*arrayValue_)[index];
}

void JSONValue::Push(const JSONValue& value)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    arrayValue_->Push(value);
}

void JSONValue::Pop()
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->Pop();
}

void JSONValue::Insert(unsigned pos, const JSONValue& value)
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->Insert(pos, value);
}

void JSONValue::Erase(unsigned pos, unsigned length)
{
    if (GetValueType() != JSON_ARRAY)
        return;

    arrayValue_->Erase(pos, length);
}

void JSONValue::Resize(unsigned newSize)
{
    // Convert to array type
    SetType(JSON_ARRAY);

    arrayValue_->Resize(newSize);
}

unsigned JSONValue::Size() const
{
    if (GetValueType() == JSON_ARRAY)
        return arrayValue_->Size();

    return 0;
}

JSONValue& JSONValue::operator [](const String& key)
{
    // Convert to object type
    SetType(JSON_OBJECT);

    return (*objectValue_)[key];
}

const JSONValue& JSONValue::operator [](const String& key) const
{
    if (GetValueType() != JSON_OBJECT)
        return EMPTY;

    return (*objectValue_)[key];
}

void JSONValue::Set(const String& key, const JSONValue& value)
{
    // Convert to object type
    SetType(JSON_OBJECT);

    (*objectValue_)[key] = value;
}

const JSONValue& JSONValue::Get(const String& key) const
{
    if (GetValueType() != JSON_OBJECT)
        return EMPTY;

    JSONObject::ConstIterator i = objectValue_->Find(key);
    if (i == objectValue_->End())
        return EMPTY;

    return i->second_;
}

bool JSONValue::Erase(const String& key)
{
    if (GetValueType() != JSON_OBJECT)
        return false;

    return objectValue_->Erase(key);
}

bool JSONValue::Contains(const String& key) const
{
    if  (GetValueType() != JSON_OBJECT)
        return false;

    return objectValue_->Contains(key);
}

JSONObjectIterator JSONValue::Begin()
{
    // Convert to object type.
    SetType(JSON_OBJECT);

    return objectValue_->Begin();
}

ConstJSONObjectIterator JSONValue::Begin() const
{
    if (GetValueType() != JSON_OBJECT)
        return emptyObject.Begin();

    return objectValue_->Begin();
}

JSONObjectIterator JSONValue::End()
{
    // Convert to object type.
    SetType(JSON_OBJECT);

    return objectValue_->Begin();
}

ConstJSONObjectIterator JSONValue::End() const
{
    if (GetValueType() != JSON_OBJECT)
        return emptyObject.End();

    return objectValue_->End();
}

void JSONValue::Clear()
{
    if (GetValueType() == JSON_ARRAY)
        arrayValue_->Clear();
    else if (GetValueType() == JSON_OBJECT)
        objectValue_->Clear();
}

void JSONValue::SetType(JSONValueType valueType, JSONNumberType numberType)
{
    int type = (valueType << 16) | numberType;
    if (type == type_)
        return;

    switch (GetValueType())
    {
    case JSON_STRING:
        delete stringValue_;
        break;

    case JSON_ARRAY:
        delete arrayValue_;
        break;

    case JSON_OBJECT:
        delete objectValue_;
        break;

    default:
        break;
    }

    type_ = type;

    switch (GetValueType())
    {
    case JSON_STRING:
        stringValue_ = new String();
        break;

    case JSON_ARRAY:
        arrayValue_ = new JSONArray();
        break;

    case JSON_OBJECT:
        objectValue_ = new JSONObject();
        break;

    default:
        break;
    }
}

void JSONValue::SetVariant(const Variant& variant, Context* context)
{
    if (!IsNull())
    {
        LOGWARNING("JsonValue is not null");
    }

    (*this)["type"] = variant.GetTypeName();
    (*this)["value"].SetVariantValue(variant, context);
}

void JSONValue::GetVariant(Variant& variant) const
{
    VariantType type = Variant::GetTypeFromName((*this)["type"].GetString());
    (*this)["value"].GetVariantValue(variant, type);    
}

void JSONValue::SetVariantValue(const Variant& variant, Context* context)
{
    if (!IsNull())
    {
        LOGWARNING("JsonValue is not null");
    }

    switch (variant.GetType())
    {
    case VAR_BOOL:
        *this = variant.GetBool();
        return;
    
    case VAR_INT:
        *this = variant.GetInt();
        return;

    case VAR_FLOAT:
        *this = variant.GetFloat();
        return;

    case VAR_DOUBLE:
        *this = variant.GetDouble();
        return;

    case VAR_STRING:
        *this = variant.GetString();
        return;

    case VAR_VARIANTVECTOR:
        SetVariantVector(variant.GetVariantVector(), context);
        return;

    case VAR_VARIANTMAP:
        SetVariantMap(variant.GetVariantMap(), context);
        return;

    case VAR_RESOURCEREF:
        {
            if (!context)
            {
                LOGERROR("Context must not null for ResourceRef");
                return;
            }

            const ResourceRef& ref = variant.GetResourceRef();
            *this = String(context->GetTypeName(ref.type_)) + ";" + ref.name_;
        }
        return;

    case VAR_RESOURCEREFLIST:
        {
            if (!context)
            {
                LOGERROR("Context must not null for ResourceRefList");
                return;
            }

            const ResourceRefList& refList = variant.GetResourceRefList();
            String str(context->GetTypeName(refList.type_));
            for (unsigned i = 0; i < refList.names_.Size(); ++i)
            {
                str += ";";
                str += refList.names_[i];
            }
            *this = str;
        }
        return;

    case VAR_STRINGVECTOR:
        {
            const StringVector& vector = variant.GetStringVector();
            Resize(vector.Size());
            for (unsigned i = 0; i < vector.Size(); ++i)
                (*this)[i] = vector[i];
        }
        return;

    default:
        *this = variant.ToString();
    }
}

void JSONValue::GetVariantValue(Variant& variant, VariantType type) const
{
    switch (type)
    {
    case VAR_BOOL:
        variant = GetBool();
        return;

    case VAR_INT:
        variant = GetInt();
        return;

    case VAR_FLOAT:
        variant = GetFloat();
        return;

    case VAR_DOUBLE:
        variant = GetDouble();
        return;

    case VAR_STRING:
        variant = GetString();
        return;

    case VAR_VARIANTVECTOR:
        {
            VariantVector vector;
            GetVariantVector(vector);
            variant = vector;
        }
        return;

    case VAR_VARIANTMAP:
        {
            VariantMap map;
            GetVariantMap(map);
            variant = map;
        }
        return;

    case VAR_RESOURCEREF:
        {
            ResourceRef ref;
            Vector<String> values = GetString().Split(';');
            if (values.Size() == 2)
            {
                ref.type_ = values[0];
                ref.name_ = values[1];
            }
            variant = ref;
        }
        return;

    case VAR_RESOURCEREFLIST:
        {
            ResourceRefList refList;
            Vector<String> values = GetString().Split(';');
            if (values.Size() >= 1)
            {
                refList.type_ = values[0];
                refList.names_.Resize(values.Size() - 1);
                for (unsigned i = 1; i < values.Size(); ++i)
                    refList.names_[i - 1] = values[i];
            }
            variant = refList;
        }
        return;

    case VAR_STRINGVECTOR:
        {
            StringVector vector;
            for (unsigned i = 0; i < Size(); ++i)
                vector.Push((*this)[i].GetString());
            variant = vector;
        }
        return;

    default:
        variant.FromString(type, GetString());
        return;
    }
}

void JSONValue::SetVariantMap(const VariantMap& variantMap, Context* context)
{
    SetType(JSON_OBJECT);
    for (VariantMap::ConstIterator i = variantMap.Begin(); i != variantMap.End(); ++i)
        (*this)[i->first_.ToString()].SetVariant(i->second_);
}

void JSONValue::GetVariantMap(VariantMap& variantMap) const
{
    if (!IsObject())
    {
        LOGERROR("JSONValue is not a object");
        return;
    }

    for (ConstJSONObjectIterator i = Begin(); i != End(); ++i)
    {
        StringHash key(ToUInt(i->first_));
        Variant variant;
        i->second_.GetVariant(variant);
        variantMap[key] = variant;
    }
}

void JSONValue::SetVariantVector(const VariantVector& variantVector, Context* context)
{
    SetType(JSON_ARRAY);
    for (unsigned i = 0; i < variantVector.Size(); ++i)
        (*this)[i].SetVariant(variantVector[i]);
}

void JSONValue::GetVariantVector(VariantVector& variantVector) const
{
    if (!IsArray())
    {
        LOGERROR("JSONValue is not a array");
        return;
    }

    for (unsigned i = 0; i < Size(); ++i)
    {
        Variant variant;
        (*this)[i].GetVariant(variant);
        variantVector.Push(variant);
    }
}

}