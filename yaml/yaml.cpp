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
                    //
                    stack.pop_back();
                }
                else
                {
                    break;
                }
            }
            else
            {
                int indentOfParent = 0;
                auto gp = *(stack.cbegin() + 1);
                for (auto i : gp->children)
                {
                    if (i->type == INDENT)
                    {
                        siblingIndentFound = true;
                        indentOfSibling = i->value.size();
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

#define ACTION_NODE_COMMON(name, type_)                                                                                        \
    template <>                                                                                                                \
    struct action<name>                                                                                                        \
    {                                                                                                                          \
        template <typename ActionInput>                                                                                        \
        static void apply(const ActionInput &in, parse_stack_type &stack, std::vector<std::shared_ptr<yaml_node>> &lineBuffer) \
        {                                                                                                                      \
            cout << #name << " apply: " << in.string() << endl;                                                                \
            auto newNode = std::make_shared<yaml_node>();                                                                      \
            newNode->type = ROOT;                                                                                              \
            newNode->value = in.string();                                                                                      \
            lineBuffer.push_back(newNode);                                                                                     \
        }                                                                                                                      \
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
    struct empty_line : star<one<' '>>
    {
    };
    struct simple_line_value : string_
    {
    };
    template <>
    struct action<simple_line_value>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack, std::vector<std::shared_ptr<yaml_node>> &lineBuffer)
        {
            cout << "simple_line_value apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = SIMPLE_VALUE;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
        }
    };

    struct value : sor<array_inline, object_inline, string_with_quotation, simple_line_value>
    {
    };
    struct line_prefix_spaces : star<space_>
    {
    };
    template <>
    struct action<line_prefix_spaces>
    {
        template <typename ActionInput>
        static void apply(const ActionInput &in, parse_stack_type &stack, std::vector<std::shared_ptr<yaml_node>> &lineBuffer)
        {
            cout << "line_prefix_spaces apply: " << in.string() << endl;
            auto newNode = std::make_shared<yaml_node>();
            newNode->type = INDENT;
            newNode->value = in.string();
            stack.back()->children.push_back(newNode);
            int indent = in.string().size();
            cout << "indent " << indent << endl;
            // Back to corret level
        }
    };

    struct line_suffix_spaces : star<space_>
    {
    };
    // ACTION_NODE_COMMON(line_suffix_spaces, ROOT)
    struct line : seq<line_prefix_spaces, sor<object_name_value_line, object_name, array_item_value_line, array_item_line, begin_document_line, end_document_line, question_line, colon_line, value, empty_line>, line_suffix_spaces>
    {
    };

    // template <>
    // struct control<line>
    //     : normal<line>
    // {
    //     template <typename ParseInput>
    //     static void start(ParseInput &in, parse_stack_type &data, std::vector<std::shared_ptr<yaml_node>> &lineBuffer)
    //     {
    //         assert(lineBuffer.empty());
    //     }

    //     template <typename ParseInput, typename... States>
    //     static void success(const ParseInput &in, parse_stack_type &data, std::vector<std::shared_ptr<yaml_node>> &lineBuffer) noexcept
    //     {
    //         int indent = 0;
    //         for(auto i:lineBuffer)
    //         {
    //             if (i->type == LINE_PREFIX_SPACES)
    //             {
    //                 indent = i->value.size();
    //                 break;
    //             }
    //         }
    //         //Find parent by indent
    //         // data.top()->
    //         lineBuffer.clear();
    //     }

    //     template <typename ParseInput>
    //     static void failure(ParseInput &in, parse_stack_type &data, std::vector<std::shared_ptr<yaml_node>> &lineBuffer)
    //     {
    //         lineBuffer.clear();
    //     }
    // };
    struct new_line : one<'\n'>
    {
    };
    struct line_with_new_line : seq<new_line, line>
    {
    };
    // ACTION_PRINT(line_with_new_line)
    // CONTROL_PRINT(line_with_new_line)
    struct yaml : seq<opt<seq<line, star<line_with_new_line>>>, eof>
    {
    };
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
        cout << current->value << "`";
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

    std::stack<std::shared_ptr<yaml::yaml_node>> parseStack;
    parseStack.push(root);

    std::vector<std::shared_ptr<yaml::yaml_node>> lineBuffer;

    // std::string data = R"({[1,2,3]: [3,4,5]})";
    std::string data = R"(  AAA
  BBB)";
    // std::string data = R"({A: [1,2,3]})";
    // std::string data = R"({A: 1})";
    // std::string data = R"({A: 1})";
    // std::string data = "(a_\nb";
    string_input input(data, "from_content");
    cout << parse<yaml::yaml, yaml::action, yaml::control>(input, parseStack, lineBuffer) << endl;
    dump_yaml(root);
}
