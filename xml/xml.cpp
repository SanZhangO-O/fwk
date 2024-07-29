#include <iostream>
#include <vector>
using std::cout;
using std::endl;

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace xml
{
    enum node_type {
        ROOT
    };
    struct xml_node {
        int type;
        std::string value;
        std::vector<std::shared_ptr<xml_node>> children;
    };

    template <typename>
    struct action
    {
    };
#define ACTION_PRINT(name)                                \
    template <>                                           \
    struct action<name>                                   \
    {                                                     \
        template <typename ActionInput>                   \
        static void apply(const ActionInput &in)          \
        {                                                 \
            cout << #name << ": " << in.string() << endl; \
        }                                                 \
    };

    struct char_ : not_one<'<'>
    {
    };
    ACTION_PRINT(char_)

    struct string_content : until<at<one<'"'>>, char_>
    {
    };
    ACTION_PRINT(string_content)

    struct string_ : seq<one<'"'>, string_content, one<'"'>>
    {
    };
    ACTION_PRINT(string_)

    struct element_text : seq<char_, until<at<one<'<'>>, char_>>
    {
    };
    ACTION_PRINT(element_text)

    struct element_name_start : sor<range<'a', 'z'>, range<'A', 'Z'>>
    {
    };
    ACTION_PRINT(element_name_start)

    struct element_name : seq<element_name_start, star<sor<range<'a', 'z'>, range<'A', 'Z'>, range<'0', '9'>>>>
    {
    };
    ACTION_PRINT(element_name)

    struct element_head_inline_end : string<'/', '>'>
    {
    };
    ACTION_PRINT(element_head_inline_end)

    struct attribute_item : seq<element_name, one<'='>, string_>
    {
    };
    ACTION_PRINT(attribute_item)

    struct element_start_lt : one<'<'>
    {
    };
    ACTION_PRINT(element_start_lt)

    struct element_head_inline : seq<element_start_lt, element_name, star<seq<plus<one<' '>>, attribute_item>>, star<one<' '>>, element_head_inline_end>
    {
    };
    ACTION_PRINT(element_head_inline)

    struct element_head : seq<element_start_lt, element_name, star<seq<plus<one<' '>>, attribute_item>>, star<one<' '>>, one<'>'>>
    {
    };
    ACTION_PRINT(element_head)

    struct element_tail : seq<string<'<', '/'>, element_name, star<one<' '>>, one<'>'>>
    {
    };
    ACTION_PRINT(element_tail)

    struct element : sor<element_head_inline, seq<element_head, element_tail>, seq<element_head, plus<sor<element, element_text>>, element_tail>>
    {
    };
    ACTION_PRINT(element)

    struct xml_prefix : star<sor<one<' '>, one<'\t'>, one<'\n'>>>
    {
    };
    ACTION_PRINT(xml_prefix)

    struct xml_suffix : xml_prefix
    {
    };
    ACTION_PRINT(xml_suffix)

    struct xml : seq<xml_prefix, element, xml_suffix, eof>
    {
    };
    ACTION_PRINT(xml)
}

int main()
{
    // std::string data = R"(<A>111<BBB>1</BBB>222</A>)";
    // std::string data = R"(<A>111<B>222</B>333</A>)";
    // std::string data = R"(<A></A>)";
    // std::string data = R"(<A attr1="1" ></A>)";
    std::string data = R"(<A b1213="1"/>)";
    string_input input(data, "from_content");
    cout << parse<xml::xml, xml::action>(input) << endl;
}
