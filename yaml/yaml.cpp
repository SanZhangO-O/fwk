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
    enum node_type
    {
        ROOT
    };

    struct yaml_node
    {
        int type;
        std::string value;
        std::vector<std::shared_ptr<yaml_node>> children;
    };
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
    struct value_without_quotation : until<at<sor<one<','>, one<'}'>>>, char_>
    {
    };
    struct object_v : sor<object_inline, array_inline, string_with_quotation, value_without_quotation>
    {
    };
    struct object_kv_pair : seq<object_k, plus<one<' '>>, one<':'>, plus<one<' '>>, object_v>
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
    struct question_line : one<'?'> {};
    // ACTION_PRINT(question_line)
    // CONTROL_PRINT(question_line)
    struct colon_line : one<':'> {};
    // ACTION_PRINT(colon_line)
    // CONTROL_PRINT(colon_line)
    struct object_name : seq<char_, until<at<one<':'>>, char_>, one<':'>>
    {
    };
    // ACTION_PRINT(object_name)
    // CONTROL_PRINT(object_name)
    struct object_name_value_line : seq<object_name, plus<space_>, value>
    {
    };
    // ACTION_PRINT(object_name_value_line)
    // CONTROL_PRINT(object_name_value_line)
    struct array_item_value_line : seq<one<'-'>, plus<one<' '>>, value>
    {
    };
    struct array_item_line : one<'-'>
    {
    };
    struct empty_line : star<one<' '>> {};
    // ACTION_PRINT(empty_line)
    // CONTROL_PRINT(empty_line)
    struct value : sor<array_inline, object_inline, string_with_quotation, string_>
    {
    };
    struct line : seq<star<space_>, sor<object_name_value_line, object_name, array_item_value_line, array_item_line, begin_document_line, end_document_line, question_line, colon_line, value, empty_line>, star<space_>>
    {
    };
    // ACTION_PRINT(line)
    // CONTROL_PRINT(line)
    struct line_with_new_line : seq<one<'\n'>, line>
    {
    };
    // ACTION_PRINT(line_with_new_line)
    // CONTROL_PRINT(line_with_new_line)
    struct yaml : seq<opt<seq<line, star<line_with_new_line>>>, eof>
    {
    };
}

int main()
{
    // std::string data = R"({[1,2,3]: [3,4,5]})";
    std::string data = R"(AAA)";
    // std::string data = R"({A: [1,2,3]})";
    // std::string data = R"({A: 1})";
    // std::string data = R"({A: 1})";
    // std::string data = "(a_\nb";
    string_input input(data, "from_content");
    cout << parse<yaml::yaml, yaml::action, yaml::control>(input) << endl;
}
