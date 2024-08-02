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
        ROOT,
        LINE_PREFIX_SPACES,
        SIMPLE_VALUE,
        INDENT
    };

    struct yaml_node
    {
        int type;
        std::string value;
        std::vector<std::shared_ptr<yaml_node>> children;
    };
    using parse_stack_type = std::vector<std::shared_ptr<yaml_node>>;
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

    template <typename Rule>
    struct control
        : normal<Rule>
    {
    };

#define CONTROL_PRINT(name)                                                         \
    template <>                                                                     \
    struct control<name>                                                            \
        : normal<name>                                                              \
    {                                                                               \
        template <typename ParseInput>                                              \
        static void start(ParseInput &in, parse_stack_type &stack)                  \
        {                                                                           \
            cout << #name << " start" << endl;                                      \
        }                                                                           \
        template <typename ParseInput, typename... States>                          \
        static void success(const ParseInput &in, parse_stack_type &stack) noexcept \
        {                                                                           \
            cout << #name << " success" << endl;                                    \
        }                                                                           \
        template <typename ParseInput>                                              \
        static void failure(ParseInput &in, parse_stack_type &stack)                \
        {                                                                           \
            cout << #name << " failure" << endl;                                    \
        }                                                                           \
    };

#define ACTION_NODE_COMMON(name, type_)                                   \
    template <>                                                           \
    struct action<name>                                                   \
    {                                                                     \
        template <typename ActionInput>                                   \
        static void apply(const ActionInput &in, parse_stack_type &stack) \
        {                                                                 \
            cout << #name << " apply: " << in.string() << endl;           \
            auto newNode = std::make_shared<yaml_node>();                 \
            newNode->type = ROOT;                                         \
            newNode->value = in.string();                                 \
            stack.back()->children.push_back(newNode);                    \
        }                                                                 \
    };

    struct value_single_line;
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
    template <>
    struct action<string_as_array_value>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "string_as_array_value apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
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
    template <>
    struct action<string_with_quotation_content>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "string_with_quotation_content apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct array_inline_value_space : plus<space_>
    {
    };
    template <>
    struct action<array_inline_value_space>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "array_inline_value_space apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct array_inline_value : seq<opt<array_inline_value_space>, sor<array_inline, object_inline, string_with_quotation, string_as_array_value>, opt<array_inline_value_space>>
    {
    };
    ACTION_PRINT(array_inline_value)
    struct array_inline_begin : one<'['>
    {
    };
    template <>
    struct action<array_inline_begin>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "array_inline_begin apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.push_back(newNode);
        }
    };
    struct array_inline_end : one<']'>
    {
    };
    template <>
    struct action<array_inline_end>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "array_inline_end: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.pop_back();
        }
    };
    struct dot : one<','>
    {
    };
    template <>
    struct action<dot>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "dot: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct array_value_with_dot : seq<dot, array_inline_value>
    {
    };
    ACTION_PRINT(array_value_with_dot)
    struct array_inline_content : opt<seq<array_inline_value, star<array_value_with_dot>>>
    {
    };
    ACTION_PRINT(array_inline_content)
    struct array_inline : seq<array_inline_begin, array_inline_content, array_inline_end>
    {
    };
    // ACTION_NODE_COMMON(array_inline, ROOT)
    struct object_inline_begin : one<'{'>
    {
    };
    template <>
    struct action<object_inline_begin>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "object_inline_begin apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.push_back(newNode);
        }
    };
    struct object_inline_end : one<'}'>
    {
    };
    template <>
    struct action<object_inline_end>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "object_inline_end apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.pop_back();
        }
    };
    struct key_without_quotation : until<at<sor<one<':'>, one<','>, one<'}'>>>, char_>
    {
    };
    struct object_inline_k : sor<object_inline, array_inline, string_with_quotation, key_without_quotation>
    {
    };
    template <>
    struct action<object_inline_k>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "object_inline_k apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.push_back(newNode);
        }
    };
    struct value_without_quotation : until<at<sor<one<','>, one<'}'>>>, char_>
    {
    };
    struct object_inline_v : sor<object_inline, array_inline, string_with_quotation, value_without_quotation>
    {
    };
    template <>
    struct action<object_inline_v>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "object_inline_v apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            stack.pop_back();
        }
    };
    struct colon : one<':'>
    {
    };
    template <>
    struct action<colon>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "colon apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct spaces_optional : star<one<' '>>
    {
    };
    template <>
    struct action<spaces_optional>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "spaces_optional apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct spaces_required : plus<one<' '>>
    {
    };
    template <>
    struct action<spaces_required>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "spaces_required apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct object_kv_pair : seq<object_inline_k, opt<spaces_optional, colon, opt<spaces_required, object_inline_v>>>
    {
    };
    ACTION_PRINT(object_kv_pair)
    struct dot_in_object : dot {};
    template <>
    struct action<dot_in_object>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "dot_in_object apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = ROOT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };
    struct object_kv_pair_with_dot : seq<dot_in_object, object_kv_pair>
    {
    };
    ACTION_PRINT(object_kv_pair_with_dot)
    struct object_inline_content : opt<seq<object_kv_pair, star<object_kv_pair_with_dot>>>
    {
    };
    ACTION_PRINT(object_inline_content)
    struct object_inline : seq<object_inline_begin, object_inline_content, object_inline_end>
    {
    };
    ACTION_PRINT(object_inline)

    ////////////////////////
    struct begin_document_line : seq<string<'-', '-', '-'>>
    {
    };
    struct end_document_line : seq<string<'.', '.', '.'>>
    {
    };
    struct question_line : one<'?'>
    {
    };
    // ACTION_PRINT(question_line)
    // CONTROL_PRINT(question_line)
    struct colon_line : one<':'>
    {
    };
    // ACTION_PRINT(colon_line)
    // CONTROL_PRINT(colon_line)
    struct object_name : seq<char_, until<at<one<':'>>, char_>, one<':'>>
    {
    };
    // ACTION_PRINT(object_name)
    // CONTROL_PRINT(object_name)
    struct object_name_value_line : seq<object_name, plus<space_>, value_single_line>
    {
    };
    // ACTION_PRINT(object_name_value_line)
    // CONTROL_PRINT(object_name_value_line)
    struct array_item_value_line : seq<one<'-'>, plus<one<' '>>, value_single_line>
    {
    };
    ACTION_PRINT(array_item_value_line)
    struct array_item_line : one<'-'>
    {
    };
    ACTION_PRINT(array_item_line)
    struct empty_line : star<one<' '>>
    {
    };
    ACTION_PRINT(empty_line)
    struct simple_line_value : string_
    {
    };
    template <>
    struct action<simple_line_value>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "simple_line_value apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = SIMPLE_VALUE;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };

    struct value_single_line : sor<array_inline, object_inline, string_with_quotation, string_>
    {
    };
    ACTION_PRINT(value_single_line)

    void find_and_back_to_parent(parse_stack_type &stack, int indent)
    {
        while (true)
        {
            bool siblingIndentFound = false;
            int indentOfSibling = 0;
            for (auto i : stack.back()->children)
            {
                if (i->type == INDENT)
                {
                    siblingIndentFound = true;
                    indentOfSibling = i->value.size();
                    break;
                }
            }
            if (siblingIndentFound)
            {
                if (indentOfSibling > indent)
                {
                    if (stack.size() > 1)
                    {
                        stack.pop_back();
                    }
                    else
                    {
                        assert(false && "Back indent error");
                    }
                }
                else
                {
                    cout << "find_and_back_to_parent break1" << endl;
                    break;
                }
            }
            else
            {
                if (stack.size() == 1)
                {
                    cout << "find_and_back_to_parent break2" << endl;
                    break;
                }
                int indentOfParent = 0;
                auto gp = *(stack.cbegin() + 1);
                for (auto i : gp->children)
                {
                    if (i->type == INDENT)
                    {
                        siblingIndentFound = true;
                        indentOfSibling = i->value.size();
                        cout << "find_and_back_to_parent break3" << endl;
                        break;
                    }
                }
                if (indentOfParent > indent)
                {
                    stack.pop_back();
                }
            }
        }
    }
    struct line_prefix_spaces : star<space_>
    {
    };
    template <>
    struct action<line_prefix_spaces>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack)
        {
            cout << "line_prefix_spaces apply: " << in.string() << endl;
            int indent = in.string().size();
            cout << "indent " << indent << endl;
            find_and_back_to_parent(stack, indent);
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = INDENT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };

    struct line_suffix_spaces : star<space_>
    {
    };
    ACTION_PRINT(line_suffix_spaces)
    struct line : seq<line_prefix_spaces, sor<array_inline, object_inline, object_name_value_line, object_name, array_item_value_line, array_item_line, string_with_quotation, star<char_>>, line_suffix_spaces>
    {
    };
    ACTION_PRINT(line)

    struct new_line : one<'\n'>
    {
    };
    struct line_with_new_line : seq<new_line, line>
    {
    };
    ACTION_PRINT(line_with_new_line)
    // ACTION_PRINT(line_with_new_line)
    // CONTROL_PRINT(line_with_new_line)
    struct yaml : seq<opt<seq<line, star<line_with_new_line>>>, eof>
    {
    };
    ACTION_PRINT(yaml)
}

void dump_yaml(std::shared_ptr<yaml::yaml_node> node)
{
    std::stack<std::shared_ptr<yaml::yaml_node>> stack;
    stack.push(node);
    cout << "dump_yaml: " << endl;
    while (!stack.empty())
    {
        auto current = stack.top();
        stack.pop();
        // cout << current->value << "`";
        cout << current->value;
        for (auto it = current->children.rbegin(); it != current->children.rend(); it++)
        {
            stack.push(*it);
        }
    }
    cout << endl;
}

int main()
{
    std::shared_ptr<yaml::yaml_node> root = std::make_shared<yaml::yaml_node>();
    root->type = yaml::ROOT;
    root->value = "";

    yaml::parse_stack_type parseStack;
    parseStack.push_back(root);

    // std::vector<std::shared_ptr<yaml::yaml_node>> lineBuffer;

    std::string data = R"({A:,B,C})";
    string_input input(data, "from_content");
    cout << parse<yaml::yaml, yaml::action, yaml::control>(input, parseStack) << endl;
    dump_yaml(root);
}
