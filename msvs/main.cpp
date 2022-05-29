#include "fagramm.h"

namespace fagramm
{
namespace id
{
enum symbol : int
{
    NON_SYMBOL,

    //Terminal symbols - punctuations
    P_LPAREN,
    P_RPAREN,
    P_COMMA,

    //Terminal symbols - keywords
    K_ADD,
    K_INTERSECT,
    K_XOR,
    K_SUBTRACT,
    K_EXPAND,
    K_CONTRACT,

    //Non-terminal symbols
    S_EXPRESSION,
    S_SET_EXPRESSION,
    S_SET_OPERATION,
    S_SCALE_EXPRESSION,
    S_SCALE_OPERATION,
    S_PARAMETER,
    S_MARGIN,
};
}
}
using namespace fagramm::id;

struct structure_expression
{
    //
    // Tokenizer data
    //
    static constexpr unsigned tokenizer_flags = (0 |
        fagramm::tokenizer::Flag_Case_Sensitive_Keywords
        );

    static constexpr fagramm::token_info punctuations[] = {
        {P_LPAREN, "("},
        {P_RPAREN, ")"},
        {P_COMMA , ","},
    };
    static constexpr fagramm::token_info keywords[] = {
        {K_ADD      , "ADD"      },
        {K_INTERSECT, "INTERSECT"},
        {K_XOR      , "XOR"      },
        {K_SUBTRACT , "SUBTRACT" },
        {K_EXPAND   , "EXPAND"   },
        {K_CONTRACT , "CONTRACT" },
    };

    //
    // Grammar data
    //
    static constexpr symbol start_symbol = S_EXPRESSION;

    static void add_rules(fagramm::rules& rules)
    {
        rules.add(S_EXPRESSION).symbol(S_SET_EXPRESSION);
        rules.add(S_EXPRESSION).symbol(S_SCALE_EXPRESSION);

        rules.add(S_SET_EXPRESSION)
            .symbol(S_SET_OPERATION)
            .punctuation(P_LPAREN)
            .symbol(S_PARAMETER)
            .loop(1)
            .punctuation(P_COMMA)
            .symbol(S_PARAMETER)
            .next()
            .punctuation(P_RPAREN)
            ;
        rules.add(S_SCALE_EXPRESSION)
            .symbol(S_SCALE_OPERATION)
            .punctuation(P_LPAREN)
            .symbol(S_PARAMETER)
            .symbol(S_MARGIN)
            .punctuation(P_RPAREN)
            ;

        rules.add(S_MARGIN).loop(6,6).punctuation(P_COMMA).number().next();
        rules.add(S_MARGIN).loop(3,3).punctuation(P_COMMA).number().next();
        rules.add(S_MARGIN).loop(1,1).punctuation(P_COMMA).number().next();

        rules.add(S_SET_OPERATION).keyword(K_ADD);
        rules.add(S_SET_OPERATION).keyword(K_INTERSECT);
        rules.add(S_SET_OPERATION).keyword(K_XOR);
        rules.add(S_SET_OPERATION).keyword(K_SUBTRACT);

        rules.add(S_SCALE_OPERATION).keyword(K_EXPAND);
        rules.add(S_SCALE_OPERATION).keyword(K_CONTRACT);

        rules.add(S_PARAMETER).string();
        rules.add(S_PARAMETER).symbol(S_EXPRESSION);
    }
};

static fagramm::tokenizer s_tokenizer(structure_expression{});
static fagramm::grammar   s_grammar  (structure_expression{});

fagramm::tokens_t tokens { 200 };

int ret = 0;

bool test_expression(const char* expression)
{
    tokens.clear();

    auto result1 = s_tokenizer.tokenize(tokens, expression);

    if(!result1)
    {
        return false;
    }

    auto result2 = s_grammar.check(tokens);

    if(!result2)
    {
        return false;
    }

    return true;
}

int main()
{
    test_expression(R"(ADD("abc", "test"))");
    test_expression(R"(EXPAND("abc", 1.2))");
    test_expression(R"(CONTRACT(ADD(CONTRACT("abc", 1.2, 2.3, 3.4), EXPAND("abc", 1.2)), 1.2, 1.2, 1.2, 1.2, 1.2, 1.2))");

    test_expression(R"(XAR("abc", "test"))");
    test_expression(R"(CONTRACT("abc", 0.5, 0.3))");

    return 0;
}
