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
        ELEMENT_BEGIN_EMPTY,
        ELEMENT_NAME,
        SPACES_IN_ELEMENT,
        ATTRIBUTE_NAME,
        ATTRIBUTE_EQUAL,
        ATTRIBUTE_VALUE_LEFT_QUOTATION,
        ATTRIBUTE_STRING_CONTENT,
        ATTRIBUTE_VALUE_RIGHT_QUOTATION,
        ELEMENT_HEAD_START_LT,
        ELEMENT_HEAD_END,
        ELEMENT_HEAD_INLINE_END,
        ELEMENT_TEXT,
        ELEMENT_TAIL_START,
        ELEMENT_TAIL_END
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

#define ACTION_NODE_COMMON(name, type_)                                   \
    template <>                                                           \
    struct action<name>                                                   \
    {                                                                     \
        template <typename ActionInput>                                   \
        static void apply(const ActionInput &in, parse_stack_type &stack) \
        {                                                                 \
            auto topNode = stack.top();                                   \
            auto newNode = std::make_shared<xml_node>();                  \
            newNode->type = type_;                                        \
            newNode->value = in.string();                                 \
            topNode->children.push_back(newNode);                         \
        }                                                                 \
    };

    struct char_ : not_one<'<'>
    {
    };

    struct string_content : until<at<one<'"'>>, char_>
    {
    };
    ACTION_NODE_COMMON(string_content, ATTRIBUTE_STRING_CONTENT)

    struct attribute_value_left_quotation : one<'"'>
    {
    };
    ACTION_NODE_COMMON(attribute_value_left_quotation, ATTRIBUTE_VALUE_LEFT_QUOTATION)

    struct attribute_value_right_quotation : one<'"'>
    {
    };
    ACTION_NODE_COMMON(attribute_value_right_quotation, ATTRIBUTE_VALUE_RIGHT_QUOTATION)

    struct string_with_quotation : seq<attribute_value_left_quotation, string_content, attribute_value_right_quotation>
    {
    };

    struct element_text : seq<char_, until<at<one<'<'>>, char_>>
    {
    };
    ACTION_NODE_COMMON(element_text, ELEMENT_TEXT)

    struct element_name_start : sor<range<'a', 'z'>, range<'A', 'Z'>>
    {
    };

    struct element_name : seq<element_name_start, star<sor<range<'a', 'z'>, range<'A', 'Z'>, range<'0', '9'>>>>
    {
    };
    ACTION_NODE_COMMON(element_name, ELEMENT_NAME)

    struct element_head_inline_end : string<'/', '>'>
    {
    };
    ACTION_NODE_COMMON(element_head_inline_end, ELEMENT_HEAD_INLINE_END)

    struct spaces_in_element_required : plus<one<' '>>
    {
    };
    ACTION_NODE_COMMON(spaces_in_element_required, SPACES_IN_ELEMENT)

    struct spaces_in_element_optional : star<one<' '>>
    {
    };
    ACTION_NODE_COMMON(spaces_in_element_optional, SPACES_IN_ELEMENT)

    struct attribute_name : element_name
    {
    };
    ACTION_NODE_COMMON(attribute_name, ATTRIBUTE_NAME)

    struct attribute_value : string_with_quotation
    {
    };

    struct attribute_equal : one<'='>
    {
    };
    ACTION_NODE_COMMON(attribute_equal, ATTRIBUTE_EQUAL)

    struct attribute_item : seq<attribute_name, spaces_in_element_optional, attribute_equal, spaces_in_element_optional, attribute_value>
    {
    };

    struct element_start_lt : one<'<'>
    {
    };
    ACTION_NODE_COMMON(element_start_lt, ELEMENT_HEAD_START_LT)
    struct element_head_inline : seq<element_start_lt, element_name, star<seq<spaces_in_element_required, attribute_item>>, spaces_in_element_optional, element_head_inline_end>
    {
    };

    struct element_head_end : one<'>'>
    {
    };
    ACTION_NODE_COMMON(element_head_end, ELEMENT_HEAD_END)
    struct element_head : seq<element_start_lt, element_name, star<seq<spaces_in_element_required, attribute_item>>, spaces_in_element_optional, element_head_end>
    {
    };

    struct element_tail_start : string<'<', '/'>
    {
    };
    ACTION_NODE_COMMON(element_tail_start, ELEMENT_TAIL_START)

    struct element_tail_end : one<'>'>
    {
    };
    ACTION_NODE_COMMON(element_tail_end, ELEMENT_TAIL_END)

    struct element_tail : seq<element_tail_start, element_name, spaces_in_element_optional, element_tail_end>
    {
    };

    struct element_empty : seq<element_head, element_tail>
    {
    };

    struct element;
    struct element_with_content : seq<element_head, plus<sor<element, element_text>>, element_tail>
    {
    };

    struct element : sor<element_head_inline, element_empty, element_with_content>
    {
    };

    struct xml_prefix : star<sor<one<' '>, one<'\t'>, one<'\n'>>>
    {
    };

    struct xml_suffix : xml_prefix
    {
    };

    struct xml : seq<xml_prefix, element, xml_suffix, eof>
    {
    };

    template <typename Rule>
    struct control
        : normal<Rule>
    {
    };

    template <>
    struct control<element_with_content>
        : normal<element_with_content>
    {
        template <typename ParseInput>
        static void start(ParseInput &in, parse_stack_type &data)
        {
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_BEGIN_EMPTY;
            data.top()->children.push_back(newNode);
            data.push(newNode);
        }

        template <typename ParseInput, typename... States>
        static void success(const ParseInput &in, parse_stack_type &data) noexcept
        {
            data.pop();
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            auto current = data.top();
            data.pop();
            auto parent = data.top();
            auto it = std::remove(parent->children.begin(), parent->children.end(), current);
            parent->children.erase(it);
        }
    };

    template <>
    struct control<element_empty>
        : normal<element_empty>
    {
        template <typename ParseInput>
        static void start(ParseInput &in, parse_stack_type &data)
        {
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_BEGIN_EMPTY;
            data.top()->children.push_back(newNode);
            data.push(newNode);
        }

        template <typename ParseInput, typename... States>
        static void success(const ParseInput &in, parse_stack_type &data) noexcept
        {
            data.pop();
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            auto current = data.top();
            data.pop();
            auto parent = data.top();
            auto it = std::remove(parent->children.begin(), parent->children.end(), current);
            parent->children.erase(it);
        }
    };

    template <>
    struct control<element_head_inline>
        : normal<element_head_inline>
    {
        template <typename ParseInput>
        static void start(ParseInput &in, parse_stack_type &data)
        {
            auto newNode = std::make_shared<xml_node>();
            newNode->type = ELEMENT_BEGIN_EMPTY;
            data.top()->children.push_back(newNode);
            data.push(newNode);
        }

        template <typename ParseInput, typename... States>
        static void success(const ParseInput &in, parse_stack_type &data) noexcept
        {
            data.pop();
        }

        template <typename ParseInput>
        static void failure(ParseInput &in, parse_stack_type &data)
        {
            auto current = data.top();
            data.pop();
            auto parent = data.top();
            auto it = std::remove(parent->children.begin(), parent->children.end(), current);
            parent->children.erase(it);
        }
    };
}

void dump_xml(std::shared_ptr<xml::xml_node> node)
{
    std::stack<std::shared_ptr<xml::xml_node>> stack;
    stack.push(node);
    cout << "dump_xml: " << endl;
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
    cout << endl;
}

std::vector<std::shared_ptr<xml::xml_node>> get_children_nodes(std::shared_ptr<xml::xml_node> node)
{
    std::vector<std::shared_ptr<xml::xml_node>> result;
    for (auto i : node->children)
    {
        if (i->type == xml::ELEMENT_BEGIN_EMPTY)
        {
            result.push_back(i);
        }
    }
    return result;
}

const std::string get_element_name(std::shared_ptr<xml::xml_node> node)
{
    for (auto i : node->children)
    {
        if (i->type == xml::ELEMENT_NAME)
        {
            return i->value;
        }
    }
    assert(false && "No name");
}

const std::vector<std::string> get_all_nodes(std::shared_ptr<xml::xml_node> node)
{
    std::vector<std::string> result;
    for (auto i : node->children)
    {
        if (i->type == xml::ELEMENT_TEXT)
        {
            result.push_back(i->value);
        }
    }
    return result;
}

const std::vector<std::pair<std::string, std::string>> get_all_attributes(std::shared_ptr<xml::xml_node> node)
{
    std::vector<std::pair<std::string, std::string>> result;
    for (auto i : node->children)
    {
        if (i->type == xml::ATTRIBUTE_NAME)
        {
            result.push_back(std::pair(i->value, ""));
        }
        else if (i->type == xml::ATTRIBUTE_STRING_CONTENT)
        {
            result.back().second = i->value;
        }
    }
    return result;
}

void update_attribute(std::shared_ptr<xml::xml_node> node, const std::string &name, const std::string &value)
{
    bool name_found = false;
    for (auto i = 0; i < node->children.size(); i++)
    {
        if (!name_found && node->children[i]->type == xml::ATTRIBUTE_NAME && node->children[i]->value == name)
        {
            name_found = true;
        }
        if (name_found && node->children[i]->type == xml::ATTRIBUTE_STRING_CONTENT)
        {
            node->children[i]->value = value;
            break;
        }
    }
}

void update_text(std::shared_ptr<xml::xml_node> node, const std::string &value)
{
    bool updated = false;
    for (auto i : node->children)
    {
        if (i->type == xml::ELEMENT_TEXT)
        {
            i->value = value;
            updated = true;
            break;
        }
    }
}

int main()
{
    std::string data = R"(
<A>
    111
    <BBB>1</BBB>
    <C attr1="1" attr2="2"/>
</A>)";

    std::shared_ptr<xml::xml_node> root = std::make_shared<xml::xml_node>();
    root->type = xml::ROOT;
    root->value = "";

    std::stack<std::shared_ptr<xml::xml_node>> parseStack;
    parseStack.push(root);
    string_input input(data, "from_content");
    cout << parse<xml::xml, xml::action, xml::control>(input, parseStack) << endl;
    dump_xml(root);
    cout << "---" << endl;
    auto children_of_root = get_children_nodes(root);
    assert(children_of_root.size() == 1);
    auto node_A = children_of_root[0];
    cout << get_element_name(node_A) << endl;
    update_text(node_A, "updated text");
    auto text_list = get_all_nodes(node_A);
    cout << "---" << endl;
    for (auto i : text_list)
    {
        cout << i << endl;
        cout << "---" << endl;
    }
    auto children_of_A = get_children_nodes(node_A);
    assert(children_of_A.size() == 2);
    auto node_BBB = children_of_A[0];
    auto node_C = children_of_A[1];
    auto all_attributes_of_C = get_all_attributes(node_C);
    for (auto i : all_attributes_of_C)
    {
        cout << i.first << " - " << i.second << endl;
    }
    update_attribute(node_C, "attr1", "111");
    dump_xml(root);
}
