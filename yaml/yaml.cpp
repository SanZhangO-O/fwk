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
    struct array_inline;
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
    struct space_ : sor<one<' '>>
    {
    };
    struct string_ : plus<char_>
    {
    };
    struct string_as_array_value : until<at<sor<one<','>, one<']'>>>, char_>
    {
    };
    struct left_quotation : one<'\"'>
    {
    };
    struct right_quotation : left_quotation
    {
    };
    struct string_with_quotation_content : until<at<one<'\"'>>, char_>
    {
    };
    struct string_with_quotation : seq<left_quotation, string_with_quotation_content, right_quotation>
    {
    };
    struct array_value : sor<array_inline, object_inline, string_with_quotation, string_as_array_value>
    {
    };
    struct array_inline_begin : one<'['>
    {
    };
    struct array_inline_end : one<']'>
    {
    };
    struct array_value_with_dot : seq<one<','>, array_value>
    {
    };
    struct array_inline_content : opt<seq<array_value, star<array_value_with_dot>>>
    {
    };
    struct array_inline : seq<array_inline_begin, array_inline_content, array_inline_end>
    {
    };
    struct object_inline_begin : one<'{'>
    {
    };
    struct object_inline_end : one<'}'>
    {
    };
    struct key_without_quotation : until<at<one<':'>>, char_>
    {
    };
    struct object_k : sor<object_inline, array_inline, string_with_quotation, key_without_quotation>
    {
    };
    // ACTION_PRINT(object_k)
    // CONTROL_PRINT(object_k)
    struct value_without_quotation : until<at<sor<one<','>, one<'}'>>>, char_>
    {
    };
    struct object_v : sor<object_inline, array_inline, string_with_quotation, value_without_quotation>
    {
    };
    struct object_kv_pair : seq<object_k, one<':'>, plus<one<' '>>, object_v>
    {
    };
    struct object_kv_pair_with_dot : seq<one<','>, object_kv_pair>
    {
    };
    struct object_inline_content : opt<seq<object_kv_pair, star<object_kv_pair_with_dot>>>
    {
    };
    struct object_inline : seq<object_inline_begin, object_inline_content, object_inline_end>
    {
    };

    ////////////////////////
    struct begin_document_line : seq<string<'-', '-', '-'>>
    {
    };
    struct end_document_line : seq<string<'.', '.', '.'>>
    {
    };
    // struct string_line : seq<star<space_>, plus<char_>, star<space_>>
    // {
    // };
    // ACTION_PRINT(string_line)
    // CONTROL_PRINT(string_line)

    struct object_name : seq<until<at<one<':'>>, char_>, one<':'>>
    {
    };
    ACTION_PRINT(object_name)
    CONTROL_PRINT(object_name)
    struct object_name_value_line : seq<object_name, plus<space_>, value>
    {
    };
    ACTION_PRINT(object_name_value_line)
    CONTROL_PRINT(object_name_value_line)
    // struct object_name : seq<star<space_>, object_name, star<space_>>
    // {
    // };
    // struct object_name_value_line : seq<star<space_>, until<at<one<':'>>, char_>, one<':'>, plus<space_>, plus<char_>>
    // {
    // };
    // ACTION_PRINT(object_name_value_line)
    // CONTROL_PRINT(object_name_value_line)
    struct array_item_value_line : seq<one<'-'>, plus<one<' '>>, value>
    {
    };
    ACTION_PRINT(array_item_value_line)
    CONTROL_PRINT(array_item_value_line)
    struct array_item_line : one<'-'>
    {
    };
    // struct empty_line : seq<opt<one<' '>>>
    // {
    // };

    struct value : sor<array_inline, object_inline, string_with_quotation, string_>
    {
    };
    ACTION_PRINT(value)
    CONTROL_PRINT(value)

    // struct line : sor<begin_document_line, object_name_value_line, object_name, array_item_value_line, string_line, empty_line>
    // {
    // };
    struct line : seq<sor<object_name_value_line, object_name, array_item_value_line, array_item_line, begin_document_line, end_document_line>, star<space>>
    {
    };
    // ACTION_PRINT(line)
    // CONTROL_PRINT(line)

    // struct line_with_new_line : seq<line, one<'\n'>>
    // {
    // };
    // ACTION_PRINT(line_with_new_line)
    // CONTROL_PRINT(line_with_new_line)

    // struct yaml : seq<star<line_with_new_line>, opt<line>, eof>
    // {
    // };
}

int main()
{
    // std::string data = R"({[1,2,3]: [3,4,5]})";
    std::string data = R"(- a)";
    // std::string data = R"({A: [1,2,3]})";
    // std::string data = R"({A: 1})";
    // std::string data = R"({A: 1})";
    // std::string data = "(a_\nb";
    string_input input(data, "from_content");
    cout << parse<yaml::line, yaml::action, yaml::control>(input) << endl;
}
