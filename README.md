# QJsonPath

A simple class to set, get and remove attributes of JSON data in Qt.
Simple XPath like notation to specify JSON attributes.
Functions support all JSON classes: QJsonDocument, QJsonObject, QJsonArray and QJsonValue.

## Functions
```c++
void QJsonPath::set(T& destValue, const QString& path, const QJsonValue& newValue);
QJsonValue QJsonPath::get(T& destValue, const QString& path, const QJsonValue& defaultValue = QJsonValue(QJsonValue::Undefined));
void QJsonPath::remove(T& destValue, const QString& path);
```

Type T can be QJsonDocument, QJsonObject, QJsonArray or QJsonValue.
Restrictions: QJsonObject cannot have an array as root, QJsonArray cannot have an object as root.
Function QJsonPath::set will create all parent attributes necessary if missing or overwrite them if not matching the path.
Assigned values can be complex, simple or null, see examples.
Functions QJsonPath::setSeparator and QJsonPath::unittest are not re-entrant and not thread safe, all other functions are re-entrant but not thread safe.

## Examples
```c++
QJsonPath::set(doc, "name0/name1[2]", "abc");
Q_ASSERT(QJsonPath::get(doc, {"name0", "name1", 2}) == "abc");
QJsonPath::remove(doc, "name0/name1[1]");
QJsonPath::setSeparator('.');
Q_ASSERT(QJsonPath::get(doc, "name0.name1[-1]") == "abc");
Q_ASSERT(QJsonPath::get(doc, "name0.name1[0]") == QJsonValue(QJsonValue::Null));
Q_ASSERT(QJsonPath::get(doc, "name0.name1[99]") == QJsonValue(QJsonValue::Undefined));
```

## Path String
A path specified as a QString uses separators and brackets. A QString is converted to a QVariantList by QJsonPath::splitPath.
If brackets do not have correct syntax or a number in it, they are parsed as a attribute name without any warning.

## Path List
Path is specified as a QVariantList. No separator is needed, names can have any character in it, even separator and brackets are allowed.
The list consists of strings and integers only. A string is always an attribute name in an object, an integer is always an index in an array.

## Array Indexes
If an array index is negative it counts down from the top, -1 for example would be the last array element. Indexes start at zero.


## More examples (QJsonPath::unittest)

```c++
QJsonDocument doc; // also works with QJsonValue and QJsonObject in the same way

// values not found are QJsonValue::Undefined, not QJsonValue::Null
Q_ASSERT(QJsonPath::get(doc, "nonexistent") == QJsonValue(QJsonValue::Undefined));
Q_ASSERT(QJsonPath::get(doc, "nonexistent", 55) == 55); // default value
// key names can also be empty
Q_ASSERT(QJsonPath::get(doc, "") == QJsonValue(QJsonValue::Undefined));

// simple attributes in root
QJsonPath::set(doc, "name0", "def"); // simple string attribute
Q_ASSERT(QJsonPath::get(doc, "") == QJsonValue(QJsonValue::Undefined));
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
obj2["name2a"] = "km";
QJsonObject obj22;
obj22["name2b"] = obj2;
QJsonPath::set(doc, "name2/name3", obj22);
Q_ASSERT(QJsonPath::get(doc, "name2/name3") == obj22);
Q_ASSERT(QJsonPath::get(doc, "name2/name3/name2b/name2a") == "km");

// array
QJsonArray v{ 1, 2, 3 };
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
Q_ASSERT(QJsonPath::get(doc, "name3[1]") == QJsonValue(QJsonValue::Null));

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



QJsonArray array;

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
```
