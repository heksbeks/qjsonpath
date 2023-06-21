// MIT License Copyright (C) 2023 heksbeks https://github.com/heksbeks/qjsonpath

#ifndef QJSONPATH_H
#define QJSONPATH_H

#include <QJsonDocument>
#include <QJsonObject>

//!  QJsonPath
/*!
 * A simple class to set, get and remove attributes of JSON data in Qt.
 * Simple XPath like notation to specify JSON attributes.
 * Functions support all JSON classes: QJsonDocument, QJsonObject, QJsonArray and QJsonValue.
 *
 * Functions:
 * void QJsonPath::set(T& destValue, const QString& path, const QJsonValue& newValue);
 * QJsonValue QJsonPath::get(T& destValue, const QString& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));
 * void QJsonPath::remove(T& destValue, const QString& path);
 *
 * Type T can be QJsonDocument, QJsonObject, QJsonArray or QJsonValue.
 * Restrictions: QJsonObject cannot have an array as root, QJsonArray cannot have an object as root.
 * Function QJsonPath::set will create all parent attributes necessary if missing or overwrite them if not matching the path.
 * Assigned values can be complex, simple or null, see examples.
 * Functions QJsonPath::setSeparator and QJsonPath::unittest are not re-entrant and not thread safe, all other functions are re-entrant but not thread safe.
 *
 * Examples:
 *   QJsonPath::set(doc, "name0/name1[2]", "abc");
 *   Q_ASSERT(QJsonPath::get(doc, {"name0", "name1", 2}) == "abc");
 *   QJsonPath::remove(doc, "name0/name1[1]");
 *   QJsonPath::setSeparator('.');
 *   Q_ASSERT(QJsonPath::get(doc, "name0.name1[-1]") == "abc");
 * For more examples see QJsonPath::unittest
 *
 * Path String:
 * A path specified as a QString uses separators and brackets. A QString is converted to a QVariantList by QJsonPath::splitPath.
 * If brackets do not have correct syntax or a number in it, they are parsed as a attribute name without any warning.
 *
 * Path List:
 * Path is specified as a QVariantList. No separator is needed, names can have any character in it, even separator and brackets are allowed.
 * The list consists of strings and integers only. A string is always an attribute name in an object, an integer is always an index in an array.
 *
 * Array Indexes:
 * If an array index is negative it counts down from the top, -1 for example would be the last array element. Indexes start at zero.
 *
 */

class QJsonPath
{
public:
    /**
     * @brief Function will modify a json object inplace, setting the json attribute definied by path to newValue, creating the full path if missing.
     * @param root     [in/out] Object representing the JSON structure.
     * @param path     [in] Path expression string (default seperator is '/').
     * @param newValue [in] New assigned value.
     */
    template <class T> static void set(T& root, const QString& path, const QJsonValue& newValue)
    {
        set(root, splitPath(path), newValue);
    }

    /**
     * @brief Function will modify a json object inplace, setting the json attribute definied by path to newValue, creating the full path if missing.
     * @param root     [in/out] Object representing the JSON structure.
     * @param path     [in] Path expression list.
     * @param newValue [in] New assigned value.
     */
    static void set(QJsonValue& root, const QVariantList& path, const QJsonValue& newValue);
    static void set(QJsonObject& root, const QVariantList& path, const QJsonValue& newValue);
    static void set(QJsonArray& root, const QVariantList& path, const QJsonValue& newValue);
    static void set(QJsonDocument& root, const QVariantList& path, const QJsonValue& newValue);

    /**
     * @brief Function will retrieve a json object defined by path. The json object can also be part of a tree with child elements.
     * @param root         [in/out] Object representing the JSON structure.
     * @param path         [in] Path expression string (default seperator is '/').
     * @param defaultValue [in] Value returned when the key is not found.
     * @return Value if found, else defaultValue.
     */
    template <class T> static QJsonValue get(T& root, const QString& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined))
    {
        return get(root, splitPath(path), defaultValue);
    }

    /**
     * @brief Function will retrieve a json object defined by path. The json object can also be part of a tree with child elements.
     * @param root         [in/out] Object representing the JSON structure.
     * @param path         [in] Path expression list.
     * @param defaultValue [in] Value returned when the key is not found.
     * @return Value if found, else defaultValue.
     */
    static QJsonValue get(const QJsonValue& root, const QVariantList& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));
    static QJsonValue get(const QJsonObject& root, const QVariantList& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));
    static QJsonValue get(const QJsonArray& root, const QVariantList& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));
    static QJsonValue get(const QJsonDocument& root, const QVariantList& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));

    /**
     * @brief Function will delete a json attribute or array element defined by path inplace.
     * @param root   [in/out] Object representing the JSON structure.
     * @param path   [in] Path expression string (default seperator is '/').
     */
    template <class T> static void remove(T& root, const QString& path)
    {
        remove(root, splitPath(path));
    }

    /**
     * @brief Function will delete a json attribute or array element defined by path inplace.
     * @param root   [in/out] Object representing the JSON structure.
     * @param path   [in] Path expression list.
     */
    static void remove(QJsonValue& root, const QVariantList& path);
    static void remove(QJsonObject& root, const QVariantList& path);
    static void remove(QJsonArray& root, const QVariantList& path);
    static void remove(QJsonDocument& root, const QVariantList& path);

    /**
     * @brief Function returns current path separator (default is '/').
     * @return Current character for separator.
     */
    static QChar separator();

    /**
     * @brief Function will change path separator to newSeparator. This is only used for path strings, path lists do not have a separator.
     * @param newSeparator New character for path separator.
     */
    static void setSeparator(QChar newSeparator);

    /**
     * @brief Converts string path to a list path. Using QVariantList for path is a more flexible way of specifying a path, you can use separators or brackets in names or as a name.
     * @param path String path specifying the JSON attribute (default seperator is '/').
     */
    static QVariantList splitPath(const QString& path);

    /**
     * @brief Unit test and examples on how to use the functions.
     */
    static void unittest();
};

#endif // QJSONPATH_H
