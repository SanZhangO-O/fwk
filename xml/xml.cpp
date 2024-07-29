#include <iostream>
using std::cout;
using std::endl;

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace xml
{
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

    // struct escaped_gt : string< '&', 'g', 't' > {};
    // struct escaped_lt : string< '&', 'l', 't' > {};
    // struct escaped_quotation_mark : string< ''
    struct char__ : sor<range<'a', 'z'>, range<'A', 'Z'>, range<'0', '9'>>
    {
    };

    struct char_ : utf8::range<0x20, 0x10FFFF>
    {
    };
    ACTION_PRINT(char_)

    struct string_content : until<at<one<'"'>>, char_>
    {
    };
    ACTION_PRINT(string_content)

    struct element_text : seq<char__, until<at<one<'<'>>, char_>>
    {
    };
    ACTION_PRINT(element_text)

    struct string_ : seq<one<'"'>, string_content, any>
    {
        using content = string_content;
    };
    ACTION_PRINT(string_)

    struct element_name_start : sor<range<'a', 'z'>, range<'A', 'Z'>>
    {
    };
    ACTION_PRINT(element_name_start)

    struct element_name : seq<element_name_start, star<sor<range<'a', 'z'>, range<'A', 'Z'>>>>
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

    struct element_head_inline : seq<one<'<'>, element_name, star<seq<one<' '>, attribute_item>>, star<one<' '>>, element_head_inline_end>
    {
    };
    ACTION_PRINT(element_head_inline)

    struct element_head : seq<one<'<'>, element_name, star<seq<one<' '>, attribute_item>>, star<one<' '>>, one<'>'>>
    {
    };
    ACTION_PRINT(element_head)

    struct element_tail : seq<one<'<'>, one<'/'>, element_name, star<one<' '>>, one<'>'>>
    {
    };
    ACTION_PRINT(element_tail)

    struct element : sor<seq<element_head, element_tail>, seq<element_head, plus<sor<element, element_text>>, element_tail>>
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
    std::string data = R"(<A>111<B>222</B>333</A>)";
    // std::string data = R"(<A></A>)";
    string_input input(data, "from_content");
    cout << parse<xml::xml, xml::action>(input) << endl;
}
