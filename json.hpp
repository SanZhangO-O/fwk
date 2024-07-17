#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <map>
#include <unordered_map>
#include <sstream>

namespace O_O
{
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

    template <typename... Args>
    void deserializeElement(const std::string &result, int &index, std::tuple<Args...> &tuple);

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
        result += "\"";
        for (auto i : t)
        {
            if (i == '\"')
            {
                result += "\\\"";
            }
            else
            {
                result += i;
            }
        }
        result += "\"";
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
        std::string temp;
        int endIndex = index;
        while (jsonData[endIndex] != quotation)
        {
            if (jsonData[endIndex] == '\\' && jsonData[endIndex + 1] == '\"')
            {
                temp += "\"";
                endIndex+=2;
            }
            else
            {
                temp += jsonData[endIndex];
                endIndex++;
            }
        }
        // t = jsonData.substr(index, endIndex - index);
        t = temp;
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
        deserializeElement(jsonData, index, t.first);
        index++;
        deserializeElement(jsonData, index, t.second);
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

    template <typename... Args>
    void deserializeElement(const std::string &jsonData, int &index, std::tuple<Args...> &tuple)
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
        deserializeElement(jsonData, index, tuple);
        std::tie(args...) = tuple;
    }
}
