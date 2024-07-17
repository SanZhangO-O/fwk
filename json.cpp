#include <string>
#include <iostream>
#include <functional>
#include <map>
#include <unordered_map>
#include <sstream>

using std::cout;
using std::endl;

template <typename T>
struct is_basic_element
{
    static constexpr bool value = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<std::string, T> ||
                                  std::is_same_v<const char *, std::decay_t<T>> || std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>;
};

template <typename T>
struct is_number
{
    static constexpr bool value = std::is_integral_v<T> || std::is_floating_point_v<T>;
};

template <typename T>
struct is_std_pair : std::false_type
{
};

template <typename T, typename U>
struct is_std_pair<std::pair<T, U>> : std::true_type
{
};

template <typename T>
struct is_map_type : std::false_type
{
};

template <typename T, typename U>
struct is_map_type<std::map<T, U>> : std::true_type
{
};

template <typename T, typename U>
struct is_map_type<std::unordered_map<T, U>> : std::true_type
{
};

template <typename T>
struct has_begin_end
{
    template <typename U>
    static constexpr auto check_begin(U *u) -> decltype(u->begin(), std::true_type());

    template <typename>
    static constexpr std::false_type check_begin(...);

    template <typename U>
    static constexpr auto check_end(U *u) -> decltype(u->end(), std::true_type());

    template <typename>
    static constexpr std::false_type check_end(...);

    static constexpr bool value = std::is_same<decltype(check_begin<T>(nullptr)), std::true_type>::value &&
                                  std::is_same<decltype(check_end<T>(nullptr)), std::true_type>::value;
};

template <typename... Args>
void serializeElement(std::string &result, const std::tuple<Args...> &tuple);

template <typename T>
std::enable_if_t<has_begin_end<T>::value && !is_basic_element<T>::value && !is_map_type<T>::value>
serializeElement(std::string &result, const T &t);

template <typename T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>
serializeElement(std::string &result, const T &t)
{
    result += std::to_string(t);
}

template <typename T>
std::enable_if_t<std::is_same_v<std::string, T>>
serializeElement(std::string &result, const T &t)
{
    result += ("\"" + t + "\"");
}

template <typename T>
std::enable_if_t<std::is_same_v<const char *, std::decay_t<T>>>
serializeElement(std::string &result, const T &t)
{
    result += ("\"" + std::string(t) + "\"");
}

template <typename T>
std::enable_if_t<std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>>
serializeElement(std::string &result, const T &t)
{
    result += ("\"" + std::string(t) + "\"");
}

template <typename T>
std::enable_if_t<is_std_pair<T>::value>
serializeElement(std::string &result, const T &t)
{
    serializeElement(result, t.first);
    result += ":";
    serializeElement(result, t.second);
}

template <typename T>
std::enable_if_t<is_map_type<T>::value>
serializeElement(std::string &result, const T &t)
{
    result += "{";
    auto it = t.begin();
    for (auto i = 0; i < t.size(); i++)
    {
        serializeElement(result, *it);
        if (i != t.size() - 1)
        {
            result += ",";
        }
        it++;
    }
    result += "}";
}

template <int I, typename... Args>
typename std::enable_if_t<I == sizeof...(Args)>
setupTupleElement(std::string &result, const std::tuple<Args...> &tuple)
{
}

template <int I, typename... Args>
typename std::enable_if_t<I != sizeof...(Args) && I == sizeof...(Args) - 1>
setupTupleElement(std::string &result, const std::tuple<Args...> &tuple)
{
    serializeElement(result, std::get<I>(tuple));
    setupTupleElement<I + 1>(result, tuple);
}

template <int I, typename... Args>
typename std::enable_if_t<I != sizeof...(Args) && I != sizeof...(Args) - 1>
setupTupleElement(std::string &result, const std::tuple<Args...> &tuple)
{
    serializeElement(result, std::get<I>(tuple));
    result += ",";
    setupTupleElement<I + 1>(result, tuple);
}

template <typename... Args>
void serializeElement(std::string &result, const std::tuple<Args...> &tuple)
{
    result += "[";
    setupTupleElement<0>(result, tuple);
    result += "]";
}

template <typename T>
std::enable_if_t<has_begin_end<T>::value && !is_basic_element<T>::value && !is_map_type<T>::value>
serializeElement(std::string &result, const T &t)
{
    result += "[";
    for (auto it = t.begin(); it != t.end(); it++)
    {
        serializeElement(result, *it);
        if (it != t.end() - 1)
        {
            result += ",";
        }
    }
    result += "]";
}

template <typename... Args>
typename std::enable_if_t<sizeof...(Args) == 0, std::string>
encode(const Args &...args)
{
    return "[]";
}

template <typename... Args>
typename std::enable_if_t<sizeof...(Args) != 0, std::string>
encode(const Args &...args)
{
    std::string result;
    serializeElement(result, std::make_tuple(args...));
    return result;
}

// deserialize
void deserializeString(const std::string &jsonData, int &index, std::string &t, const char quotation)
{
    int endIndex = index;
    while (jsonData[endIndex] != quotation)
    {
        endIndex++;
    }
    t = jsonData.substr(index, endIndex - index);
    index = endIndex;
    index++;
}

template <typename T>
typename std::enable_if_t<std::is_same_v<T, std::string>>
deserializeElement(const std::string &jsonData, int &index, T &t)
{
    auto quotation = jsonData[index];
    index++;
    deserializeString(jsonData, index, t, quotation);
}

template <typename T>
typename std::enable_if_t<is_number<T>::value>
deserializeElement(const std::string &jsonData, int &index, T &t)
{
    int endIndex = index;
    while (jsonData[endIndex] != ',' && jsonData[endIndex] != ']')
    {
        endIndex++;
    }
    std::string temp = jsonData.substr(index, endIndex - index);
    index = endIndex;
    std::stringstream(temp) >> t;
}

template <typename T>
std::enable_if_t<has_begin_end<T>::value && !is_basic_element<T>::value && !is_map_type<T>::value>
deserializeElement(const std::string &jsonData, int &index, T &t)
{
    index++;
    while (jsonData[index] != ']')
    {
        typename T::value_type newValue;
        deserializeElement(jsonData, index, newValue);
        t.push_back(newValue);
        if (jsonData[index] == ',')
        {
            index++;
        }
    }
    index++;
}

template <typename T>
std::enable_if_t<is_std_pair<T>::value>
deserializeElement(const std::string &jsonData, int &index, T &t)
{
    cout << "PAIR1 " << index << " " << jsonData[index] << endl;
    deserializeElement(jsonData, index, t.first);
    cout << "PAIR2 " << index << " " << jsonData[index] << " " << t.first << endl;
    index++;
    cout << "PAIR3 " << index << " " << jsonData[index] << endl;
    deserializeElement(jsonData, index, t.second);
    cout << "PAIR4 " << index << " " << jsonData[index] << endl;
    // index++;
    // cout << "PAIR5 " << index << " " << jsonData[index] << endl;
}

template <typename T>
std::enable_if_t<is_map_type<T>::value>
deserializeElement(const std::string &jsonData, int &index, T &t)
{
    index++;
    while (jsonData[index] != '}')
    {
        std::pair<typename T::key_type, typename T::mapped_type> newValue;
        deserializeElement(jsonData, index, newValue);
        t.insert(newValue);
        if (jsonData[index] == ',')
        {
            index++;
        }
    }
    index++;
}

template <int I, typename... Args>
std::enable_if_t<I == sizeof...(Args)>
deserializeTupleElement(const std::string &jsonData, int &index, std::tuple<Args...> &tuple)
{
}

template <int I, typename... Args>
std::enable_if_t<I != sizeof...(Args)>
deserializeTupleElement(const std::string &jsonData, int &index, std::tuple<Args...> &tuple)
{
    deserializeElement(jsonData, index, std::get<I>(tuple));
    index++;
    deserializeTupleElement<I + 1>(jsonData, index, tuple);
}

template <int I, typename... Args>
typename std::enable_if_t<I == sizeof...(Args)>
deserializeTuple(const std::string &jsonData, int &index, std::tuple<Args...> &tuple)
{
}

template <int I, typename... Args>
typename std::enable_if_t<I != sizeof...(Args)>
deserializeTuple(const std::string &jsonData, int &index, std::tuple<Args...> &tuple)
{
    index++;
    deserializeTupleElement<0>(jsonData, index, tuple);
}

template <typename... Args>
typename std::enable_if_t<sizeof...(Args) == 0>
decode(const std::string &jsonData, int &index, Args &...args)
{
}

template <typename... Args>
typename std::enable_if_t<sizeof...(Args) != 0>
decode(const std::string &jsonData, int &index, Args &...args)
{
    auto tuple = std::make_tuple(args...);
    deserializeTuple<0>(jsonData, index, tuple);
    std::tie(args...) = tuple;
}

// User code
// Serialize
struct Struct1
{
    int i;
    std::string s;
};

void serializeElement(std::string &result, const Struct1 &v)
{
    result += encode(v.i, v.s);
}

void deserializeElement(const std::string &jsonData, int &index, Struct1 &t)
{
    decode(jsonData, index, t.i, t.s);
}

struct Struct2
{
    Struct1 struct1;
    std::string ss;
};

void serializeElement(std::string &result, const Struct2 &v)
{
    result += encode(v.struct1, v.ss);
}

void deserializeElement(const std::string &jsonData, int &index, Struct2 &t)
{
    decode(jsonData, index, t.struct1, t.ss);
}

int main()
{
    // std::string jsonString = encode(std::vector<int>{111, 222, 333}, 1, 2, 3, std::vector<int>{444, 555, 666}, 1.2, std::string("STR1"));
    // std::string jsonString = encode(std::string("STR1"), std::string("STR2"));
    std::string jsonString = encode(std::map<std::string, std::string>{{"A", "B"}, {"C", "D"}}, std::map<std::string, std::string>{{"E", "F"}}, std::vector<int>{444, 555, 666}, 1, 2, 3);
    cout << jsonString << endl;
    std::map<std::string, std::string> recvMap1, recvMap2;

    std::vector<int> recvVec1, recvVec2;
    int recvInt1, recvInt2, recvInt3;
    // double recvDouble1;
    // std::string recvStr1, recvStr2;
    // decode(jsonString, recvVec1, recvInt1, recvInt2, recvInt3, recvVec2, recvDouble1, recvStr1);
    int index = 0;
    decode(jsonString, index, recvMap1, recvMap2, recvVec1, recvInt1, recvInt2, recvInt3);
    for (auto i : recvMap1)
    {
        cout << i.first << " " << i.second << endl;
    }
    for (auto i : recvMap2)
    {
        cout << i.first << " " << i.second << endl;
    }
    // decode(jsonString, index, recvStr1, recvStr2);
    cout << "recvVec1 " << endl;
    for (auto i : recvVec1)
    {
        cout << i << endl;
    }
    // cout << "recvVec2 " << endl;
    // for (auto i : recvVec2)
    // {
    //     cout << i << endl;
    // }
    cout << "recvInt1 " << recvInt1 << endl;
    cout << "recvInt2 " << recvInt2 << endl;
    cout << "recvInt3 " << recvInt3 << endl;
    // cout << "recvDouble1 " << recvDouble1 << endl;
    // cout << "recvStr1 " << recvStr1 << endl;
    // cout << "recvStr2 " << recvStr2 << endl;

    // cout << encode(1, 2, 3.4, true, std::string("AAA"), "BBB", std::map<std::string, std::string>{{"A", "1"}, {"B", "2"}}) << endl;

    ///////////////////////
    // Struct1 struct1;
    // struct1.i = 123;
    // struct1.s = "AAA";
    // {
    //     std::string result;
    //     serializeElement(result, struct1);
    //     cout << result << endl;
    //     result.clear();
    //     cout << encode(struct1) << endl;
    //     Struct1 recvStruct1;
    //     int index = 0;
    //     decode(encode(struct1), index, recvStruct1);
    //     cout << "recvStruct1 " << recvStruct1.i << " " << recvStruct1.s << endl;
    // }

    // {
    //     std::string result;
    //     Struct2 struct2;
    //     struct2.struct1 = struct1;
    //     struct2.ss = "BBB";
    //     serializeElement(result, struct2);
    //     cout << result << endl;
    //     cout << encode(struct2) << endl;
    //     int index = 0;
    //     Struct2 recvStruct2;
    //     decode(encode(struct2), index, recvStruct2);
    //     cout << "recvStruct2 " << recvStruct2.struct1.i << " " << recvStruct2.struct1.s << " " << recvStruct2.ss << endl;
    // }
}
