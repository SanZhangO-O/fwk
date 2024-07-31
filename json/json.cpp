#include <iostream>
#include <algorithm>
#include <stack>
#include <vector>
#include <queue>
using std::cout;
using std::endl;

#include <tao/pegtl.hpp>

using namespace tao::pegtl;

namespace json
{
    struct digits : plus<digit>
    {
    };
    struct exp : seq<one<'e', 'E'>, opt<one<'-', '+'>>, digits>
    {
    };
    struct frac : seq<one<'.'>, digits>
    {
    };
    struct int_ : sor<one<'0'>, plus<digit>>
    {
    };
    struct number : seq<opt<one<'-'>>, int_, opt<frac>, opt<exp>>
    {
    };

    struct unicode : list<seq<one<'u'>, rep<4, xdigit>>, one<'\\'>>
    {
    };
    struct escaped_char : one<'"', '\\', '/', 'b', 'f', 'n', 'r', 't'>
    {
    };
    struct escaped : sor<escaped_char, unicode>
    {
    };
    struct unescaped : utf8::range<0x20, 0x10FFFF>
    {
    };
    struct char_ : if_then_else<one<'\\'>, escaped, unescaped>
    {
    };

    struct string_content : until<at<one<'"'>>, char_>
    {
    };

    struct string_with_quotation : seq<one<'\"'>, string_content, one<'\"'>>
    {
    };
    struct value;
    struct key_content : until<at<one<'"'>>, char_>
    {
    };
    struct key : seq<one<'"'>, key_content, any>
    {
    };
    struct ws : one<' ', '\t', '\n', '\r'>
    {
    };
    template <typename R, typename P = ws>
    struct padr : seq<R, star<P>>
    {
    };

    struct member_value : padr<value>
    {
    };
    struct name_separator : pad<one<':'>, ws>
    {
    };
    struct member : seq<key, name_separator, member_value>
    {
    };
    struct next_member : seq<member>
    {
    };
    struct value_separator : padr<one<','>>
    {
    };
    struct object_content : opt<member, star<value_separator, next_member>>
    {
    };
    struct begin_object : padr<one<'{'>>
    {
    };
    struct end_object : one<'}'>
    {
    };
    struct begin_array : padr< one< '[' > > {
    };
    struct end_array : one<']'>
    {
    };
    struct array_element : padr<value>
    {
    };
    struct array_element;
    struct next_array_element : seq<array_element>
    {
    };
    struct array_content : opt<array_element, star<value_separator, next_array_element>>
    {
    };
    struct array : seq<begin_array, array_content, end_array>
    {
    };
    struct object : seq<begin_object, object_content, end_object>
    {
    };
    struct value : sor<number, string_with_quotation, object, array>
    {
    };

    struct json : seq<value, eof>
    {
    };

}

int main()
{
    std::string data = R"({"A":[1, "2"]})";
    string_input input(data, "from_content");
    cout << parse<json::json>(input) << endl;
}
