## Intro

During my current professional task (structure formulas processing and calculation) I realized that formula parsing algorithms that I worked with are not quite effective. In my weekend time I reviewed some popular Open Source libraries for grammar checking in [libs.garden](https://libs.garden/cpp/search?q=expression%20grammar&sort=popular).
I didn't like any of them anyway. They are too template heavy, lambda heavy, too generic, etc. So I decided to write my own Open Source Context-Free Grammar checker library with minimal dependencies and not too many "fency" staff like templates, lambdas, meaningless operator overloading, etc. The Tokenizer was the easy part. The grammar checker was a little more sophisticated but I did it anyway.

### The result
- my new context-free grammar checker Open Source Library: [fagramm](https://github.com/ElemagEx/fagramm)
- single header file - 390 lines
- single implementation file - 615 lines
- minimum dependencies: `<vector>`, `<algorithm>`, `<cstring>`, `<cctype>`
- sample grammar for structure formula parsing and checking

#### Here is some sample source source:

1. Define your constants for terminal and non-terminal symbols
```C++
namespace fagramm::id {
enum symbol : int
{
    NON_SYMBOL,
    //Terminal symbols - punctuations
    P_LPAREN, P_RPAREN, P_COMMA,
    //Terminal symbols - keywords
    K_ADD, K_INTERSECT, K_XOR, K_SUBTRACT, K_EXPAND, K_CONTRACT,
    //Non-terminal symbols
    S_EXPRESSION, S_SET_EXPRESSION, S_SET_OPERATION,
    S_SCALE_EXPRESSION, S_SCALE_OPERATION,
    S_ARGUMENT, S_MARGIN,
};
}
using namespace fagramm::id;
```
2. Describe your grammar
```C++
// All description needed for structure formula parsing and checking
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
            .symbol(S_ARGUMENT)
        //  .loop(1) - uncomment this loop(...) and below next() will allow to operation to have arbitrary number of arguments
            .punctuation(P_COMMA)
            .symbol(S_ARGUMENT)
        //  .next()
            .punctuation(P_COMMA)
            .symbol(S_ARGUMENT)
            .punctuation(P_RPAREN)
            ;
        rules.add(S_SCALE_EXPRESSION)
            .symbol(S_SCALE_OPERATION)
            .punctuation(P_LPAREN)
            .symbol(S_ARGUMENT)
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

        rules.add(S_ARGUMENT).string();
        rules.add(S_ARGUMENT).symbol(S_EXPRESSION);
    }
};
```
3. Use the tokenizer and grammar

```C++
// One-time preparation of tokenizer and grammar ...
static fagramm::tokenizer s_tokenizer(structure_expression{});
static fagramm::grammar   s_grammar  (structure_expression{});
// ... and multiple parsing and checking
bool grammar_check(const char* formula)
{
	std::vector<token_data> tokens;
	return s_tokenizer.tokenize(tokens, formula) && s_grammar.check(tokens);
}
```

I think the advantages are obvious:
- fast and safe parsing and checking
- easy grammar changes
- ...

Personally for me grammars are my old (univerity time) love (together with Prolog and Lisp) and I like the way how the library turned out. I have many ideas to extend the library and I plan to develop it from time to time in my free time.

I will be very glad of you decide to use this library. Enjoy :)
