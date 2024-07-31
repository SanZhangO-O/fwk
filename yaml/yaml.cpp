#include <iostream>
#include <algorithm>
#include <stack>
#include <vector>
#include <queue>
using std::cout;
using std::endl;

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace yaml
{
    template <typename>
    struct action
    {
    };

#define ACTION_PRINT(name)                                      \
    template <>                                                 \
    struct action<name>                                         \
    {                                                           \
        template <typename ActionInput>                         \
        static void apply(const ActionInput &in)                \
        {                                                       \
            cout << #name << " apply: " << in.string() << endl; \
        }                                                       \
    };

    template <typename Rule>
    struct control
        : normal<Rule>
    {
    };

#define CONTROL_PRINT(name)                                \
    template <>                                            \
    struct control<name>                                   \
        : normal<name>                                     \
    {                                                      \
        template <typename ParseInput>                     \
        static void start(ParseInput &in)                  \
        {                                                  \
            cout << #name << " start" << endl;             \
        }                                                  \
        template <typename ParseInput, typename... States> \
        static void success(const ParseInput &in) noexcept \
        {                                                  \
            cout << #name << " success" << endl;           \
        }                                                  \
        template <typename ParseInput>                     \
        static void failure(ParseInput &in)                \
        {                                                  \
            cout << #name << " failure" << endl;           \
        }                                                  \
    };

    struct value;
    struct object_inline;
    struct escaped_char : one<'"', '\\', '/', 'b', 'f', 'n', 'r', 't'>
    {
    };
    struct escaped : sor<escaped_char>
    {
    };
    struct unescaped : utf8::range<0x20, 0x10FFFF>
    {
    };
    struct char_ : if_then_else<one<'\\'>, escaped, unescaped>
    {
    };
    struct spaces_ : sor<one<' '>>
    {
    };
    struct start_document_line : seq<string<'-', '-', '-'>, opt<star<spaces_>>>
    {
    };
    struct string_ : plus<char_>
    {
    };

    struct string_inline_array : until<at<sor<one<','>, one<']'>>>>
    {
    };
    ACTION_PRINT(string_inline_array)
    CONTROL_PRINT(string_inline_array)

    struct left_quotation : one<'\"'>
    {
    };
    struct right_quotation : left_quotation
    {
    };
    struct string_with_quotation : seq<left_quotation, until<at<one<'\"'>>, char_>, right_quotation>
    {
    };
    ACTION_PRINT(string_with_quotation)
    CONTROL_PRINT(string_with_quotation)
    struct array_inline;
    struct array_value : sor<array_inline, object_inline, string_with_quotation, string_inline_array>
    {
    };
    ACTION_PRINT(array_value)
    CONTROL_PRINT(array_value)
    struct array_inline_begin : one<'['>
    {
    };
    struct array_inline_end : one<']'>
    {
    };
    struct array_inline_item : sor<array_value>
    {
    };
    ACTION_PRINT(array_inline_item)
    CONTROL_PRINT(array_inline_item)
    struct array_inline_item_with_dot : seq<one<','>, array_value>
    {
    };
    ACTION_PRINT(array_inline_item_with_dot)
    CONTROL_PRINT(array_inline_item_with_dot)
    struct array_inline_content : opt<seq<array_inline_item, star<array_inline_item_with_dot>>>
    {
    };
    ACTION_PRINT(array_inline_content)
    CONTROL_PRINT(array_inline_content)

    struct array_inline : seq<array_inline_begin, array_inline_content, array_inline_end>
    {
    };
    ACTION_PRINT(array_inline)
    CONTROL_PRINT(array_inline)

    struct object_inline_begin : one<'{'>
    {
    };
    struct object_inline_end : one<'}'>
    {
    };

    struct object_value
    {
    };
    struct object_inline_item : seq<object_value>
    {
    };
    struct key_without_quotation : until<at<one<':'>>, char_>
    {
    };
    struct object_k : sor<string_with_quotation, key_without_quotation>
    {
    };
    ACTION_PRINT(object_k)
    CONTROL_PRINT(object_k)
    struct value_without_quotation : until<at<sor<one<','>, one<'}'>>>, char_>
    {
    };
    struct object_v : sor<object_inline, array_inline, string_with_quotation, value_without_quotation>
    {
    };
    ACTION_PRINT(object_v)
    CONTROL_PRINT(object_v)
    struct object_kv_pair : seq<object_k, one<':'>, plus<one<' '>>, object_v>
    {
    };
    struct object_item : seq<object_kv_pair>
    {
    };
    ACTION_PRINT(object_item)
    CONTROL_PRINT(object_item)
    struct object_item_with_dot : seq<one<','>, object_kv_pair>
    {
    };
    struct object_inline_content : opt<seq<object_item, star<object_item_with_dot>>>
    {
    };
    ACTION_PRINT(object_inline_content)
    CONTROL_PRINT(object_inline_content)
    struct object_inline : seq<object_inline_begin, object_inline_content, object_inline_end>
    {
    };
    ACTION_PRINT(object_inline)
    CONTROL_PRINT(object_inline)

    struct string_line : seq<star<spaces_>, plus<char_>, star<spaces_>>
    {
    };
    ACTION_PRINT(string_line)
    CONTROL_PRINT(string_line)

    struct object_name : seq<plus<char_>, one<':'>, star<spaces_>>
    {
    };
    ACTION_PRINT(object_name)
    CONTROL_PRINT(object_name)
    struct object_name_line : seq<star<spaces_>, object_name, star<spaces_>>
    {
    };
    struct object_name_value_line : seq<star<spaces_>, until<at<one<':'>>, char_>, one<':'>, plus<spaces_>, plus<char_>>
    {
    };
    ACTION_PRINT(object_name_value_line)
    CONTROL_PRINT(object_name_value_line)
    struct array_item_line : seq<opt<spaces_>, one<'-'>, plus<one<' '>>, plus<sor<char_>>, star<space>>
    {
    };
    ACTION_PRINT(array_item_line)
    CONTROL_PRINT(array_item_line)
    struct empty_line : seq<opt<one<' '>>>
    {
    };

    struct value : sor<array_inline, object_inline, string_>
    {
    };
    ACTION_PRINT(value)
    CONTROL_PRINT(value)

    struct line : sor<start_document_line, object_name_value_line, object_name_line, array_item_line, string_line, empty_line>
    {
    };
    ACTION_PRINT(line)
    CONTROL_PRINT(line)

    struct line_with_new_line : seq<line, one<'\n'>>
    {
    };
    ACTION_PRINT(line_with_new_line)
    CONTROL_PRINT(line_with_new_line)

    struct yaml : seq<star<line_with_new_line>, opt<line>, eof>
    {
    };
}

int main()
{
    std::string data = R"([1,{B: 2}])";
    // std::string data = R"({A: [1,2,3]})";
    // std::string data = R"({A: 1})";
    // std::string data = R"({A: 1})";
    // std::string data = "(a_\nb";
    string_input input(data, "from_content");
    cout << parse<yaml::value, yaml::action, yaml::control>(input) << endl;
}
