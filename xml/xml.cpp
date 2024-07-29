#include <iostream>
#include <algorithm>
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
        ATTR_NAME,
        ELEMENT_NAME,
        ELEMENT_HEAD_END,
        ELEMENT_HEAD_INLINE_END
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
#define ACTION_PRINT(name)                                                \
    template <>                                                           \
    struct action<name>                                                   \
    {                                                                     \
        template <typename ActionInput>                                   \
        static void apply(const ActionInput &in, parse_stack_type &stack) \
        {                                                                 \
            cout << #name << " apply: " << in.string() << endl;           \
        }                                                                 \
    };

    struct char_ : not_one<'<'>
    {
    };
    // ACTION_PRINT(char_)

    struct string_content : until<at<one<'"'>>, char_>
    {
    };
    template <>
    struct action<string_content>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "string_content apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct attribute_value_left_quotation : one<'"'>{};
    template <>
    struct action<attribute_value_left_quotation>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "attribute_value_left_quotation apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct attribute_value_right_quotation : one<'"'>{};
    template <>
    struct action<attribute_value_right_quotation>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "attribute_value_right_quotation apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct string_with_quotation : seq<attribute_value_left_quotation, string_content, attribute_value_right_quotation>
    {
    };
    ACTION_PRINT(string_with_quotation)

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
    template <>
    struct action<element_name>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "element_name apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };


    struct element_head_inline_end : string<'/', '>'>
    {
    };
    template <>
    struct action<element_head_inline_end>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "element_head_inline_end apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_HEAD_INLINE_END;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct spaces_in_element_required : plus<one<' '>>
    {
    };
    template <>
    struct action<spaces_in_element_required>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "spaces_in_element_required apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_HEAD_INLINE_END;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct spaces_in_element_optional : star<one<' '>>
    {
    };
    template <>
    struct action<spaces_in_element_optional>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "spaces_in_element_optional apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_HEAD_INLINE_END;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct attribute_name : element_name
    {
    };
    template <>
    struct action<attribute_name>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "attribute_name apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct attribute_value : string_with_quotation
    {
    };

    struct attribute_equal : one<'='>
    {
    };
    template <>
    struct action<attribute_equal>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "attribute_equal apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ATTR_NAME;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };


    struct attribute_item : seq<attribute_name, attribute_equal, attribute_value>
    {
    };
    ACTION_PRINT(attribute_item)

    struct element_start_lt : one<'<'>
    {
    };
    ACTION_PRINT(element_start_lt)

    struct element_head_inline : seq<element_start_lt, element_name, star<seq<spaces_in_element_required, attribute_item>>, spaces_in_element_optional, element_head_inline_end>
    {
    };
    ACTION_PRINT(element_head_inline)

    struct element_head_end : one<'>'>
    {
    };
    template <>
    struct action<element_head_end>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "element_head_end apply: " << in.string() << endl;
            auto topNode = stack.top();
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_HEAD_END;
            newNode->value = in.string();
            topNode->children.push_back(newNode);
        }
    };

    struct element_head : seq<element_start_lt, element_name, star<seq<spaces_in_element_required, attribute_item>>, spaces_in_element_optional, element_head_end>
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
            cout << "element_start_lt start: push stack" << endl;
            auto newNode = std::make_shared<xml_node>();
            newNode->type = BEGIN_LT;
            newNode->value = "<";
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
        static void success(const ParseInput &in, parse_stack_type &data) noexcept
        {
            cout << "element success: pop stack" << endl;
            data.pop();
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            cout << "element failure: pop stack" << endl;
            auto parent = data.top();
            auto current = data.top();
            data.pop();
            auto it = std::remove(parent->children.begin(), parent->children.end(), current);
            parent->children.erase(it);
        }
    };
}

void dump_xml_tree(std::shared_ptr<xml::xml_node> node)
{
    std::stack<std::shared_ptr<xml::xml_node>> stack;
    stack.push(node);
    cout << "dump_xml_tree: " << endl;
    while (!stack.empty())
    {
        auto current = stack.top();
        stack.pop();
        cout << current->value;
        for (auto it = current->children.rbegin(); it != current->children.rend(); it++)
        {
            stack.push(*it);
        }
    }
}

int main()
{
    // std::string data = R"(<A>111<BBB>1</BBB>222</A>)";
    // std::string data = R"(<A>111<B>222</B>333</A>)";
    // std::string data = R"(<A/>)";
    // std::string data = R"(<A attr1="1" >123</A>)";
    std::string data = R"(<A>123</A>)";
    // std::string data = R"(<A b="1"/>)";

    std::shared_ptr<xml::xml_node> root = std::make_shared<xml::xml_node>();
    root->type = xml::ROOT;
    root->value = "";

    std::stack<std::shared_ptr<xml::xml_node>> parseStack;
    parseStack.push(root);
    string_input input(data, "from_content");
    cout << parse<xml::xml, xml::action, xml::control>(input, parseStack) << endl;
    dump_xml_tree(root);
}
