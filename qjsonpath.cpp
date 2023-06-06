// MIT License Copyright (C) 2023 heksbeks https://github.com/heksbeks/qjsonpath

#include "qjsonpath.h"
#include <QJsonArray>


//#define DEBUG_HANDLE_JSON_ATTRIBUTE

enum class HANDLE_JSON_OP
{
    SET,
    GET,
    REMOVE,
};

QChar _handleJsonAttribute_separator('/');

static void __handleJsonAttribute(QJsonValue& value, const QVariantList& path, const QJsonValue& newValue, HANDLE_JSON_OP op)
{
    if (path[0].userType() == QMetaType::Type::QString) {
        if (!value.isObject()) {
            if (op == HANDLE_JSON_OP::SET)
                value = QJsonObject();
            else if (op == HANDLE_JSON_OP::GET) {
                value = QJsonValue(QJsonValue::Undefined);
                return;
            }
            else if (op == HANDLE_JSON_OP::REMOVE)
                return;
        }
    }
    else if (path[0].userType() == QMetaType::Type::Int) {
        if (!value.isArray()) {
            if (op == HANDLE_JSON_OP::SET)
                value = QJsonArray();
            else if (op == HANDLE_JSON_OP::GET) {
                value = QJsonValue(QJsonValue::Undefined);
                return;
            }
            else if (op == HANDLE_JSON_OP::REMOVE)
                return;
        }
    }
    else {
        Q_ASSERT_X(path[0].userType() == QMetaType::Type::QString || path[0].userType() == QMetaType::Type::Int, __FUNCTION__, QString("invalid path type '%1'").arg(path[0].userType()).toUtf8());
        return;
    }

    if (value.isObject()) {
        auto obj = value.toObject();
        auto keyName = path[0].toString();
        if (op == HANDLE_JSON_OP::GET) {
            if (!obj.contains(keyName)) {
                value = QJsonValue(QJsonValue::Undefined);
                return;
            }
        }
        else if (op == HANDLE_JSON_OP::REMOVE) {
            if (!obj.contains(keyName))
                return;
        }
        QJsonValue subValue = obj[keyName];
        if (path.size() > 1)
            __handleJsonAttribute(subValue, path.mid(1), newValue, op);
        else if (op == HANDLE_JSON_OP::SET)
            subValue = newValue;
        else if (op == HANDLE_JSON_OP::REMOVE) {
            if (obj.contains(keyName)) {
                obj.remove(keyName);
                value = obj;
            }
            return;
        }
        if (op == HANDLE_JSON_OP::SET || op == HANDLE_JSON_OP::REMOVE) {
            obj[keyName] = subValue;
            value = obj;
        }
        else if (op == HANDLE_JSON_OP::GET)
            value = subValue;
        return;
    }
    else if (value.isArray()) {
        auto arr = value.toArray();
        int idx = path[0].toInt();
        if (idx < 0)
            idx = arr.size() ? arr.size() + idx : 0; // -1 is last element
        if (op == HANDLE_JSON_OP::SET) {
            while (idx >= arr.size())
                arr.append(QJsonValue());
        }
        else if (op == HANDLE_JSON_OP::GET) {
            if (idx < 0 || idx >= arr.size()) {
                value = QJsonValue(QJsonValue::Undefined);
                return;
            }
        }
        else if (op == HANDLE_JSON_OP::REMOVE) {
            if (idx < 0 || idx >= arr.size())
                return;
        }
        QJsonValue subValue = arr[idx];
        if (path.size() > 1)
            __handleJsonAttribute(subValue, path.mid(1), newValue, op);
        else if (op == HANDLE_JSON_OP::SET)
            subValue = newValue;
        else if (op == HANDLE_JSON_OP::REMOVE) {
            arr.removeAt(idx);
            value = arr;
            return;
        }
        if (op == HANDLE_JSON_OP::SET || op == HANDLE_JSON_OP::REMOVE) {
            arr[idx] = subValue;
            value = arr;
        }
        else if (op == HANDLE_JSON_OP::GET)
            value = subValue;
        return;
    }

    qDebug() << "fatal error in " __FUNCTION__ ": unknown type" << value;
    Q_ASSERT_X(value.isObject() || value.isArray(), __FUNCTION__, "unknown type");
}


static void _handleJsonAttribute(QJsonValue& value, const QVariantList& path, const QJsonValue& newValue, HANDLE_JSON_OP op)
{
    __handleJsonAttribute(value, path, newValue, op);

#ifdef DEBUG_HANDLE_JSON_ATTRIBUTE
    if (op != HANDLE_JSON_OP::GET) {
        qDebug() << (op == HANDLE_JSON_OP::SET ? "QJsonPath::set" : "QJsonPath::remove") << "path=" << path << "newValue=" << newValue;
        auto doc = value.isArray() ? QJsonDocument(value.toArray()) : QJsonDocument(value.toObject());
        qDebug().noquote() << QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    }
#endif
}

static void _handleJsonAttribute(QJsonObject& obj, const QVariantList& path, const QJsonValue& newValue, HANDLE_JSON_OP op)
{
    QJsonValue val = obj;
    _handleJsonAttribute(val, path, newValue, op);
    if (!val.isArray()) // an array cannot be converted to a QJsonObject
        obj = val.toObject();
    else
        Q_ASSERT_X(!val.isArray(), __FUNCTION__, QString("invalid result type '%1', path must result to an root object").arg(val.type()).toUtf8());
}

static void _handleJsonAttribute(QJsonArray& arr, const QVariantList& path, const QJsonValue& newValue, HANDLE_JSON_OP op)
{
    QJsonValue val = arr;
    _handleJsonAttribute(val, path, newValue, op);
    if (val.isArray()) // an object cannot be converted to a QJsonArray
        arr = val.toArray();
    else
        Q_ASSERT_X(val.isArray(), __FUNCTION__, QString("invalid result type '%1', path must result to a root array").arg(val.type()).toUtf8());
}

static void _handleJsonAttribute(QJsonDocument& doc, const QVariantList& path, const QJsonValue& newValue, HANDLE_JSON_OP op)
{
    QJsonValue val;
    if (doc.isArray())
        val = doc.array();
    else
        val = doc.object();

    _handleJsonAttribute(val, path, newValue, op);

    if (val.isArray())
        doc = QJsonDocument(val.toArray());
    else
        doc = QJsonDocument(val.toObject());
}


void QJsonPath::set(QJsonValue& root, const QVariantList& path, const QJsonValue& newValue)
{
    _handleJsonAttribute(root, path, newValue, HANDLE_JSON_OP::SET);
}
void QJsonPath::set(QJsonObject& root, const QVariantList& path, const QJsonValue& newValue)
{
    _handleJsonAttribute(root, path, newValue, HANDLE_JSON_OP::SET);
}
void QJsonPath::set(QJsonArray& root, const QVariantList& path, const QJsonValue& newValue)
{
    _handleJsonAttribute(root, path, newValue, HANDLE_JSON_OP::SET);
}
void QJsonPath::set(QJsonDocument& root, const QVariantList& path, const QJsonValue& newValue)
{
    _handleJsonAttribute(root, path, newValue, HANDLE_JSON_OP::SET);
}


QJsonValue QJsonPath::get(const QJsonValue& root, const QVariantList& path, const QJsonValue& defaultValue)
{
    QJsonValue value = root;
    _handleJsonAttribute(value, path, QJsonValue(), HANDLE_JSON_OP::GET);
    return value.isUndefined() ? defaultValue : value;
}
QJsonValue QJsonPath::get(const QJsonObject& root, const QVariantList& path, const QJsonValue& defaultValue)
{
    QJsonValue value = root;
    _handleJsonAttribute(value, path, QJsonValue(), HANDLE_JSON_OP::GET);
    return value.isUndefined() ? defaultValue : value;
}
QJsonValue QJsonPath::get(const QJsonArray& root, const QVariantList& path, const QJsonValue& defaultValue)
{
    QJsonValue value = root;
    _handleJsonAttribute(value, path, QJsonValue(), HANDLE_JSON_OP::GET);
    return value.isUndefined() ? defaultValue : value;
}
QJsonValue QJsonPath::get(const QJsonDocument& root, const QVariantList& path, const QJsonValue& defaultValue)
{
    auto value = root.isArray() ? QJsonValue(root.array()) : QJsonValue(root.object());
    _handleJsonAttribute(value, path, QJsonValue(), HANDLE_JSON_OP::GET);
    return value.isUndefined() ? defaultValue : value;
}


void QJsonPath::remove(QJsonValue& root, const QVariantList& path)
{
    _handleJsonAttribute(root, path, QJsonValue(), HANDLE_JSON_OP::REMOVE);
}
void QJsonPath::remove(QJsonObject& root, const QVariantList& path)
{
    _handleJsonAttribute(root, path, QJsonValue(), HANDLE_JSON_OP::REMOVE);
}
void QJsonPath::remove(QJsonArray& root, const QVariantList& path)
{
    _handleJsonAttribute(root, path, QJsonValue(), HANDLE_JSON_OP::REMOVE);
}
void QJsonPath::remove(QJsonDocument& root, const QVariantList& path)
{
    _handleJsonAttribute(root, path, QJsonValue(), HANDLE_JSON_OP::REMOVE);
}


QChar QJsonPath::separator()
{
    return _handleJsonAttribute_separator;
}
void QJsonPath::setSeparator(QChar newSeparator)
{
    _handleJsonAttribute_separator = newSeparator;
}


QVariantList QJsonPath::splitPath(const QString& path)
{
    const QChar bracketOpen('['), bracketClose(']');
    QVariantList p;
    int i=0, i0_name=0, i0_idx=-1;
    for (; i < path.size(); i++) {
        if (path[i] == _handleJsonAttribute_separator) {
            if (i0_name >= 0)
                p << path.mid(i0_name, i - i0_name);
            i0_name = i+1;
            i0_idx = -1;
        }
        else if (path[i] == bracketOpen)
            i0_idx = i+1;
        else if (path[i] == bracketClose && i0_idx > 0) {
            auto sIdx = path.mid(i0_idx, i - i0_idx);
            bool ok = false;
            int idx = sIdx.toInt(&ok);
            if (ok) {
                int n = i0_idx-1 - i0_name;
                if (i0_name >= 0 && n > 0)
                    p << path.mid(i0_name, n);
                p << idx;
                i0_name = -1;
                i0_idx = -1;
            }
        }
    }
    if (i0_name >= 0)
        p << path.mid(i0_name, i - i0_name);
    return p;
}


template <class T> static void _handleJsonAttribute_unittest_object(T& doc)
{
    // non existent attributes are undefined
    Q_ASSERT(QJsonPath::get(doc, "nonexistent") == QJsonValue(QJsonValue::Undefined)); // values not found are QJsonValue::Undefined, not QJsonValue::Null
    Q_ASSERT(QJsonPath::get(doc, "nonexistent", 55) == 55); // default value
    Q_ASSERT(QJsonPath::get(doc, "") == QJsonValue(QJsonValue::Undefined)); // key names can also be empty

    // simple attributes in root
    QJsonPath::set(doc, "name0", "def"); // simple string attribute
    Q_ASSERT(QJsonPath::get(doc, "") == QJsonValue(QJsonValue::Undefined)); // try again with not empty JSON structure
    QJsonPath::set(doc, "", "abc"); // key names can also be empty
    Q_ASSERT(QJsonPath::get(doc, "name0") == "def");
    Q_ASSERT(QJsonPath::get(doc, "") == "abc");

    // simple nested attributes
    QJsonPath::set(doc, "name1/name1", 20);
    Q_ASSERT(QJsonPath::get(doc, "name1/name1") == 20);
    QJsonObject obj1;
    obj1["name1"] = 20;
    Q_ASSERT(QJsonPath::get(doc, "name1") == obj1);
    Q_ASSERT(QJsonPath::get(doc, "name1/nonexistent") == QJsonValue(QJsonValue::Undefined));
    Q_ASSERT(QJsonPath::get(doc, "name1/nonexistent", "66") == "66"); // default value

    // to avoid conflicts with the separator or bracket characters use QVariantList
    QJsonPath::set(doc, {"name2", "name /2", "time [ms]"}, "x");
    Q_ASSERT(QJsonPath::get(doc, {"name2", "name /2", "time [ms]"}, "y") == "x");
    Q_ASSERT(QJsonPath::get(doc, {"name2", "name /2", "nonexistent"}) == QJsonValue(QJsonValue::Undefined));
    Q_ASSERT(QJsonPath::get(doc, {"name2", "name2", "nonexistent"}, 77) == 77);

    // complex values
    QJsonObject obj2;
    obj2["name2"] = "x";
    Q_ASSERT(QJsonPath::get(doc, "name2/name2") == obj2);
    QJsonObject obj22;
    obj22["name2"] = obj2;
    Q_ASSERT(QJsonPath::get(doc, "name2") == obj22);
    QJsonPath::set(doc, "name2/name3", obj22);
    Q_ASSERT(QJsonPath::get(doc, "name2/name3") == obj22);

    // array
    auto v = QJsonArray({ 1,2,3 });
    QJsonPath::set(doc, "name2/name21/name2", v);
    Q_ASSERT(QJsonPath::get(doc, "name2/name21/name2", "y2") == v);
    Q_ASSERT(QJsonPath::get(doc, "name2/name210/name2", obj22) == obj22);

    // array indexes
    QJsonPath::set(doc, "name3[0]", "xy");
    Q_ASSERT(QJsonPath::get(doc, "name3[0]") == "xy");
    Q_ASSERT(QJsonPath::get(doc, "name3[1]") == QJsonValue(QJsonValue::Undefined));
    Q_ASSERT(QJsonPath::get(doc, "name3[1]", "88") == "88");

    QJsonPath::set(doc, "name3[2]", "xyz"); // define only 3rd element of array, first two will be null
    Q_ASSERT(QJsonPath::get(doc, "name3[2]") == "xyz");
    Q_ASSERT(QJsonPath::get(doc, "name3[-1]") == "xyz"); // negativ indexes are from top to bottom

    // when using a list, strings are attribute names, numbers are array indexes
    QJsonPath::set(doc, {"name3", 1, "name30", "name31"}, "asd");
    Q_ASSERT(QJsonPath::get(doc, "name3[1]/name30/name31") == "asd");

    QJsonPath::set(doc, "name3[1]/name31[2]/name31", "qqq");
    Q_ASSERT(QJsonPath::get(doc, "name3[1]/name31[2]/name31") == "qqq");

    QJsonPath::set(doc, "name3[1]/name32/name31[1]", true);
    Q_ASSERT(QJsonPath::get(doc, "name3[1]/name32/name31[1]", false) == true);

    // change separator
    QJsonPath::setSeparator('.');
    Q_ASSERT(QJsonPath::get(doc, "name3[1].name31[2].name31") == "qqq");
    QJsonPath::set(doc, "name3[1].name32.name31[1]", "sep");
    Q_ASSERT(QJsonPath::get(doc, "name3[1].name32.name31[1]", 333) == "sep");
    QJsonPath::setSeparator('/');

    // null values
    QJsonPath::set(doc, "name3[1]", QJsonValue()); // replace complex entry by null
    Q_ASSERT(QJsonPath::get(doc, "name3[1]", false) == QJsonValue());

    // nested arrays
    QJsonPath::set(doc, "name3[3][3]", "qwe");
    Q_ASSERT(QJsonPath::get(doc, "name3[3][3]") == "qwe");
    Q_ASSERT(QJsonPath::get(doc, "name3[-1][-1]") == "qwe");

    // remove attributes
    QJsonPath::remove(doc, "name3[3][3]");
    Q_ASSERT(QJsonPath::get(doc, "name3[3][3]") == QJsonValue(QJsonValue::Undefined));

    QJsonPath::remove(doc, "name3[3]");
    Q_ASSERT(QJsonPath::get(doc, "name3[3]") == QJsonValue(QJsonValue::Undefined));

    QJsonPath::remove(doc, "name3");
    Q_ASSERT(QJsonPath::get(doc, "name3") == QJsonValue(QJsonValue::Undefined));

    QJsonPath::remove(doc, "name2");
    QJsonPath::remove(doc, "");
    QJsonPath::remove(doc, "name1");
    QJsonPath::remove(doc, "name0");
    Q_ASSERT(QJsonPath::get(doc, "name2") == QJsonValue(QJsonValue::Undefined));
}

template <class T> static void _handleJsonAttribute_unittest_array(T& array)
{
    // array at root, only works for QJsonArray and QJsonDocument
    QJsonPath::set(array, "[1]", "uvw");
    Q_ASSERT(QJsonPath::get(array, "[1]", false) == "uvw");

    QJsonPath::set(array, "[1]/name0/name1", 3.14);
    Q_ASSERT(QJsonPath::get(array, "[1]/name0/name1", false) == 3.14);

    QJsonPath::remove(array, "[1]");
    Q_ASSERT(QJsonPath::get(array, "[1]") == QJsonValue(QJsonValue::Undefined));
    Q_ASSERT(QJsonPath::get(array, "[0]") == QJsonValue(QJsonValue::Null));

    QJsonPath::remove(array, "[0]");
    Q_ASSERT(QJsonPath::get(array, "[0]") == QJsonValue(QJsonValue::Undefined));
}

void QJsonPath::unittest()
{
    auto sepBackup = _handleJsonAttribute_separator;
    _handleJsonAttribute_separator = QChar('/');

    QJsonValue val;
    _handleJsonAttribute_unittest_object(val);
    Q_ASSERT(val == QJsonValue(QJsonValue::Object));
    _handleJsonAttribute_unittest_array(val);
    Q_ASSERT(val == QJsonValue(QJsonValue::Array));

    QJsonObject obj;
    _handleJsonAttribute_unittest_object(obj);
    Q_ASSERT(obj == QJsonObject());

    QJsonArray arr;
    _handleJsonAttribute_unittest_array(arr);
    Q_ASSERT(arr == QJsonArray());

    QJsonDocument doc;
    _handleJsonAttribute_unittest_object(doc);
    Q_ASSERT(doc.object() == QJsonDocument().object());
    _handleJsonAttribute_unittest_array(doc);
    Q_ASSERT(doc.array() == QJsonDocument().array());

    _handleJsonAttribute_separator = sepBackup;
    qDebug() << __FUNCTION__ " finished";
}
