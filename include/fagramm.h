#pragma once

#define Assert_Check(cond)
#define Assert_Fail()

#define Check_ValidArg(  cond, ...) if(!(cond)) return __VA_ARGS__
#define Check_ValidState(cond, ...) if(!(cond)) return __VA_ARGS__

#include <vector>
#include <string>

namespace fagramm
{
namespace id
{
enum symbol : int;
}
using symbol_id = id::symbol;

using std::size_t;

enum class token_type : int
{
    ident,
    string,
    number,
    keyword,
    punctuation,
};
struct token_info
{
    symbol_id   id;
    const char* str;
};
struct token_data
{
    token_type type;
    symbol_id  id;
    size_t     pos;
    size_t     len;
};

using tokens_t = std::vector<token_data>;

enum class parse_error : int
{
    None,
    InvalidPunctuation,
    InvalidKeyword,
    DuplicatePunctuations,
    DuplicateKeywords,
    InvalidArguments,
    UnknownCharacter,
    UnknownPunctuation,
    InvalidLeadingZero,
    MissingStringCloseQuote,
    InvalidLoopArguments,
    MismatchLoopNextPairs,
    NextWithoutLoop,
    SymbolWithoutRule,
    UnpreparedGramar,
    GrammarCheckFailed,
    WrongTokenType,
};
struct result_t
{
    parse_error err;
    symbol_id   id;
    size_t      pos;
    explicit operator bool() const { return (err == parse_error::None); }
};

class tokenizer
{
    tokenizer           (const tokenizer&) noexcept = delete;
    tokenizer& operator=(const tokenizer&) noexcept = delete;

public:
    tokenizer           (tokenizer&&) noexcept = default;
    tokenizer& operator=(tokenizer&&) noexcept = default;

    tokenizer() = default;
   ~tokenizer() = default;

public:
    template<class T>
    tokenizer(T&& t)
    {
    //  [[maybe_unused]]
        result_t result = reset(t.punctuations, std::size(t.punctuations), t.keywords, std::size(t.keywords), t.tokenizer_flags);

        Check_ValidState(result);
    }

public:
    enum : unsigned
    {
        Flag_Default                 = 0,
        Flag_Case_Sensitive_Keywords = (1 << 0),
    };
    bool flag_is_set(unsigned flag) const
    {
        return ((m_flags & flag) != 0);
    }

public:
    void clear();

    result_t reset(
        const token_info* punctuations = nullptr,
        size_t punctuations_count = 0,
        const token_info* keywords = nullptr,
        size_t keywords_count = 0,
        unsigned flags = Flag_Default
        );

    result_t tokenize(
        tokens_t& tokens,
        const char* str,
        size_t len = size_t(-1)
        )
        const;

private:
    static int compare_strings(
        bool case_sensitive,
        const char* str1,
        size_t len1,
        const char* str2,
        size_t len2
        );
    static bool is_valid_punctuation(const char* str);
    static bool is_valid_keyword(const char* str);

    result_t reset_punctuations(
        const token_info* punctuations,
        size_t punctuations_count
        );
    result_t reset_keywords(
        const token_info* keywords,
        size_t keywords_count
        );

    bool find_punctuation(symbol_id& id, const char* str, size_t len) const;
    bool find_keyword    (symbol_id& id, const char* str, size_t len) const;

private:
    struct context
    {
        tokens_t*   tokens;
        const char* begin;
        const char* pos;
        parse_error err;
    };

    void remove_whitespace(const char*& str, const char* end, context& ctx) const;

    bool check_string(const char*& str, const char* end, context& ctx) const;
    bool check_number(const char*& str, const char* end, context& ctx) const;
    bool check_ident (const char*& str, const char* end, context& ctx) const;
    bool check_punct (const char*& str, const char* end, context& ctx) const;

private:
    struct token_desc
    {
        symbol_id   id;
        const char* str;
        size_t      len;
    };
    std::vector<token_desc> m_punctuations;
    std::vector<token_desc> m_keywords;

    size_t   m_max_punct_len = 0;
    unsigned m_flags         = Flag_Default;

public:
    static parse_error extract_token_number(const char* str, const token_data& token,  float& number);
    static parse_error extract_token_number(const char* str, const token_data& token, double& number);

    static parse_error extract_token_string(
        const char* str,
        const token_data& token,
        std::string& out,
        bool unescape  = false,
        bool addQuotes = false
        );

    static std::string stringize_tokens(
        const char* str,
        const token_data* token_begin,
        const token_data* token_end
        );
};

class rules;

class rule
{
    friend class rules;

    rules* m_rules;

    rule(rules& r) : m_rules(&r) {}

public:
    rule loop(size_t min_repeats, size_t max_repeats = size_t(-1));
    rule next();
    rule symbol(symbol_id id);

    rule ident();
    rule string();
    rule number();

    rule keyword(symbol_id id);
    rule punctuation(symbol_id id);
};

class rules
{
    friend class rule;

    rules           (const rules&) noexcept = delete;
    rules& operator=(const rules&) noexcept = delete;

protected:
    rules           (rules&&) noexcept = default;
    rules& operator=(rules&&) noexcept = default;

    rules() = default;
   ~rules() = default;

public:
    rule add(symbol_id id)
    {
        m_start_index = npos;
        m_chunks.push_back({chunk_type::start, id});
        return rule(*this);
    }

protected:
    static constexpr size_t npos = size_t(-1);

    size_t m_start_index = npos;

    enum class chunk_type : int
    {
        ident       = int(token_type::ident),
        string      = int(token_type::string),
        number      = int(token_type::number),
        keyword     = int(token_type::keyword),
        punctuation = int(token_type::punctuation),

        symbol,
        start,
        loop,
        next,
        rule,
    };
    struct chunk_data
    {
        chunk_type type;
        symbol_id  id;
        size_t     arg1;
        size_t     arg2;
    };
    std::vector<chunk_data> m_chunks;
};

inline rule rule::loop(size_t min_repeats, size_t max_repeats)
{
    m_rules->m_chunks.push_back({rules::chunk_type::loop, symbol_id(0), min_repeats, max_repeats});
    return *this;
}
inline rule rule::next()
{
    m_rules->m_chunks.push_back({rules::chunk_type::next, symbol_id(0), 0, 0});
    return *this;
}
inline rule rule::symbol(symbol_id id)
{
    m_rules->m_chunks.push_back({rules::chunk_type::symbol, id, 0, 0});
    return *this;
}
inline rule rule::ident()
{
    m_rules->m_chunks.push_back({rules::chunk_type::ident, symbol_id(0), 0, 0});
    return *this;
}
inline rule rule::string()
{
    m_rules->m_chunks.push_back({rules::chunk_type::string, symbol_id(0), 0, 0});
    return *this;
}
inline rule rule::number()
{
    m_rules->m_chunks.push_back({rules::chunk_type::number, symbol_id(0), 0, 0});
    return *this;
}
inline rule rule::keyword(symbol_id id)
{
    m_rules->m_chunks.push_back({rules::chunk_type::keyword, id, 0, 0});
    return *this;
}
inline rule rule::punctuation(symbol_id id)
{
    m_rules->m_chunks.push_back({rules::chunk_type::punctuation, id, 0, 0});
    return *this;
}

class grammar : protected rules
{
    grammar           (const grammar&) noexcept = delete;
    grammar& operator=(const grammar&) noexcept = delete;

public:
    grammar           (grammar&&) noexcept = default;
    grammar& operator=(grammar&&) noexcept = default;

public:
    template<typename T>
    grammar(T&& t)
    {
        t.add_rules(*this);

    //  [[maybe_unused]]
        result_t result = prepare(t.start_symbol);

        Check_ValidState(result);
    }

public:
    void clear();
    rule add_rule(symbol_id id);

public:
    result_t prepare(symbol_id start_id);

    result_t check(
        const tokens_t& tokens,
        size_t index = 0,
        size_t count = npos
        ) const;

private:
    struct loop_data
    {
        size_t cur_repeats;
        size_t min_repeats;
        size_t max_repeats;
        size_t first_index;
        const token_data* token;
    };
    using loop_stack_t = std::vector<loop_data>;

private:
    size_t find_symbol_with_id(symbol_id id) const;
    size_t find_or_add_symbol(symbol_id id);

    bool verify_rule(const token_data*& token, const token_data* end, size_t symbol_index, loop_stack_t& loop_stack) const;

    bool verify_token(const token_data*& token, const token_data* end, token_type type) const;
    bool verify_token(const token_data*& token, const token_data* end, token_type type, symbol_id id) const;

private:
    struct rule_data
    {
        bool operator<(const rule_data& other) const
        {
            return (id != other.id) ? (id < other.id) : (order < other.order);
        }
        symbol_id id;
        unsigned  order;
        size_t    first_chunk;
        size_t    last_chunk;
    };
    struct symbol_data
    {
        bool operator<(const symbol_data& other) const
        {
            return (id < other.id);
        }
        symbol_id id;
        size_t    first_rule;
        size_t    last_rule;
    };
    std::vector<rule_data>   m_rules;
    std::vector<symbol_data> m_symbols;
};

}
