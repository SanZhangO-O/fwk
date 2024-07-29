#include <iostream>
#include <stack>
#include <vector>
#include <queue>
using std::cout;
using std::endl;

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace xml
{
    enum node_type
    {
        ROOT,
        BEGIN_LT,
        ATTR_NAME
    };
    struct xml_node
    {
        int type;
        std::string value;
        std::vector<std::shared_ptr<xml_node>> children;
    };
    using parse_stack_type = std::stack<std::shared_ptr<xml_node>>;

    template <typename>
    struct action
    {
    };
#define ACTION_PRINT(name)                                                                     \
    template <>                                                                                \
    struct action<name>                                                                        \
    {                                                                                          \
        template <typename ActionInput>                                                        \
        static void apply(const ActionInput &in, parse_stack_type &stack) \
        {                                                                                      \
            cout << #name << " apply: " << in.string() << endl;                                      \
        }                                                                                      \
    };

    struct char_ : not_one<'<'>
    {
    };
    // ACTION_PRINT(char_)

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

    struct attribute_name : element_name
    {
    };
    // ACTION_PRINT(attribute_name)
    template <>
    struct action<attribute_name>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "attribute_name" << " apply: " << in.string() << endl;
            auto topNode = stack.top();
            // topNode->children
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct attribute_value : string_
    {
    };
    ACTION_PRINT(attribute_value)

    struct attribute_item : seq<attribute_name, one<'='>, attribute_value>
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


    template <typename Rule>
    struct control
        : normal<Rule>
    {
    };

    template <>
    struct control<element_start_lt>
        : normal<element_start_lt>
    {
        template <typename ParseInput>
        static void start(ParseInput &in, parse_stack_type &data)
        {
            cout << "element_start_lt start: " << endl;
            auto newNode = std::make_shared<xml_node>();
            newNode->type = BEGIN_LT;
            data.top()->children.push_back(newNode);
            data.push(newNode);
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            cout << "element_start_lt failure: " << endl;
        }
    };

    template <>
    struct control<element>
        : normal<element>
    {
        template <typename ParseInput>
        static void start(ParseInput &in, parse_stack_type &data)
        {
            cout << "element start: " << endl;
        }

        template <typename ParseInput, typename... States>
        static void success(const ParseInput & in, parse_stack_type &data) noexcept
        {
            cout << "element success: " << endl;
            data.pop();
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            cout << "element failure: " << endl;
        }
    };
}

void dump_xml_tree(std::shared_ptr<xml::xml_node> node)
{
    std::queue<std::shared_ptr<xml::xml_node>> queue;
    queue.push(node);
    while(!queue.empty())
    {
        auto current = queue.front();
        queue.pop();
        cout << current->value;
        for(auto i:current->children)
        {
            queue.push(i);
        }
    }
}

int main()
{
    // std::string data = R"(<A>111<BBB>1</BBB>222</A>)";
    // std::string data = R"(<A>111<B>222</B>333</A>)";
    std::string data = R"(<A/>)";
    // std::string data = R"(<A attr1="1" >123</A>)";
    // std::string data = R"(<A b1213="1"/>)";

    std::shared_ptr<xml::xml_node> root = std::make_shared<xml::xml_node>();
    root->type = xml::ROOT;
    root->value = "ROOT";
    std::stack<std::shared_ptr<xml::xml_node>> parseStack;
    parseStack.push(root);
    string_input input(data, "from_content");
    cout << parse<xml::xml, xml::action, xml::control>(input, parseStack) << endl;
    dump_xml_tree(root);

}
