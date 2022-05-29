#include "fagramm.h"

#include <algorithm>
#include <cstring>
#include <cctype>

namespace fagramm
{

void tokenizer::clear()
{
    m_flags = Flag_Default;

    m_punctuations.clear();
    m_keywords    .clear();
}

result_t tokenizer::reset(
    const token_info* punctuations,
    size_t punctuations_count,
    const token_info* keywords,
    size_t keywords_count,
    unsigned flags
    )
{
    clear();

    m_flags = flags;

    result_t result;

    if( !bool(result = reset_punctuations(punctuations, punctuations_count)) ||
        !bool(result = reset_keywords    (keywords    , keywords_count    ))
        )
    {
        clear();
    }
    return result;
}

result_t tokenizer::tokenize(
    tokens_t& tokens,
    const char* str,
    size_t len
    )
    const
{
    Check_ValidArg(str != nullptr, {parse_error::InvalidArguments});

    context ctx {&tokens, str, str, parse_error::None};

    const char* end = ((str + len) < str)
        ? (const char*)(-1)
        : (str + len);

    while((str < end) && (ctx.err == parse_error::None))
    {
        remove_whitespace(str, end, ctx);

        if(check_string(str, end, ctx)) continue;
        if(check_number(str, end, ctx)) continue;
        if(check_ident (str, end, ctx)) continue;
        if(check_punct (str, end, ctx)) continue;

        if(*str != 0)
        {
            ctx.err = parse_error::UnknownCharacter;
            ctx.pos = str;
        }
        break;
    }

    result_t result {ctx.err, symbol_id(0), size_t(ctx.pos - ctx.begin)};

    return result;
}

int tokenizer::compare_strings(
    bool case_sensitive,
    const char* str1,
    size_t len1,
    const char* str2,
    size_t len2
    )
{
    size_t len = std::min(len1, len2);

    int dif;

    if(case_sensitive)
    {
        dif = std::strncmp(str1, str2, len);

        if(dif != 0) return dif;
    }
    else while(len-- == 0)
    {
        dif = std::toupper(*str1++) - std::toupper(*str2++);

        if(dif != 0) return dif;
    }
    return (int)(len1 - len2);
}

bool tokenizer::is_valid_punctuation(const char* str)
{
    return (str != nullptr) && (*str != 0);
}

bool tokenizer::is_valid_keyword(const char* str)
{
    return (str != nullptr) && (*str != 0);
}

result_t tokenizer::reset_punctuations(
    const token_info* punctuations,
    size_t punctuations_count
    )
{
    if((punctuations == nullptr) || (punctuations_count == 0)) return {parse_error::None};

    for(size_t index = 0; index < punctuations_count; ++index)
    {
        if(!is_valid_punctuation(punctuations[index].str)) return {parse_error::InvalidPunctuation, symbol_id(0), index};

        m_punctuations.push_back({punctuations[index].id, punctuations[index].str, std::strlen(punctuations[index].str)});
    }

    bool has_duplicates = false;
    std::sort(m_punctuations.begin(), m_punctuations.end(), [&has_duplicates] (const token_desc& punctuation1, const token_desc& punctuation2)
    {
        const int res = compare_strings(true, punctuation1.str, punctuation1.len, punctuation2.str, punctuation2.len);
        has_duplicates = has_duplicates || (res == 0);
        return (res < 0);
    });
    if(has_duplicates) return {parse_error::DuplicatePunctuations};

    auto it = std::max_element(m_punctuations.begin(), m_punctuations.end(), [] (const token_desc& punctuation1, const token_desc& punctuation2)
    {
        return (punctuation1.len < punctuation2.len);
    });
    Assert_Check(it != m_punctuations.end());

    m_max_punct_len = it->len;

    return {parse_error::None};
}

result_t tokenizer::reset_keywords(
    const token_info* keywords,
    size_t keywords_count
    )
{
    if((keywords == nullptr) || (keywords_count == 0)) return {parse_error::None};

    for(size_t index = 0; index < keywords_count; ++index)
    {
        if(!is_valid_keyword(keywords[index].str)) return {parse_error::InvalidKeyword, symbol_id(0), index};

        m_keywords.push_back({keywords[index].id, keywords[index].str, std::strlen(keywords[index].str)});
    }

    const bool case_sensitive_keywords = flag_is_set(Flag_Case_Sensitive_Keywords);

    bool has_duplicates = false;

    std::sort(m_keywords.begin(), m_keywords.end(), [case_sensitive_keywords, &has_duplicates] (const token_desc& keyword1, const token_desc& keyword2)
    {
        const int res = compare_strings(case_sensitive_keywords, keyword1.str, keyword1.len, keyword2.str, keyword2.len);
        has_duplicates = has_duplicates || (res == 0);
        return (res < 0);
    });
    if(has_duplicates) return {parse_error::DuplicateKeywords};

    return {parse_error::None};
}

bool tokenizer::find_punctuation(symbol_id& id, const char* str, size_t len) const
{
    const token_desc punctuation {symbol_id(0), str, len};

    const bool found = std::binary_search(m_punctuations.begin(), m_punctuations.end(), punctuation, [&id] (const token_desc& punctuation1, const token_desc& punctuation2)
    {
        const int res = compare_strings(true, punctuation1.str, punctuation1.len, punctuation2.str, punctuation2.len);
        if(res == 0) id = symbol_id(int(punctuation1.id) + int(punctuation2.id));
        return (res < 0);
    });
    return found;
}

bool tokenizer::find_keyword(symbol_id& id, const char* str, size_t len) const
{
    const bool case_sensitive_keywords = flag_is_set(Flag_Case_Sensitive_Keywords);

    const token_desc keyword {symbol_id(0), str, len};

    const bool found = std::binary_search(m_keywords.begin(), m_keywords.end(), keyword, [&id, case_sensitive_keywords] (const token_desc& keyword1, const token_desc& keyword2)
    {
        const int res = compare_strings(case_sensitive_keywords, keyword1.str, keyword1.len, keyword2.str, keyword2.len);
        if(res == 0) id = symbol_id(int(keyword1.id) + int(keyword2.id));
        return (res < 0);
    });
    return found;
}

void tokenizer::remove_whitespace(const char*& str, const char* end, context&/* ctx*/) const
{
    for( ; (str < end) && std::isspace(*str); ++str);
}

bool tokenizer::check_string(const char*& str, const char* end, context& ctx) const
{
    if(*str != '"') return false;

    const char* start = str++;

    for( ; (str < end); ++str)
    {
        switch(*str)
        {
            case 0:
                ctx.err = parse_error::MissingStringCloseQuote;
                ctx.pos = str;
                return true;

            case '\"': ++str; break;
            case '\\': ++str; continue;

            default: continue;
        }
        break;
    }
    if(str == end)
    {
        ctx.err = parse_error::MissingStringCloseQuote;
        ctx.pos = str;
        return true;
    }

    const size_t pos = size_t(start - ctx.begin);
    const size_t len = size_t(str   - start);

    ctx.tokens->push_back({token_type::string, symbol_id(0), pos, len});

    return true;
}

bool tokenizer::check_number(const char*& str, const char* end, context& ctx) const
{
    if(!std::isdigit(*str)) return false;

    const char* start = str++;

    if((*start == '0') && (str < end) && std::isdigit(*str))
    {
        ctx.err = parse_error::InvalidLeadingZero;
        ctx.pos = str;
        return true;
    }

    for( ; (str < end) && std::isdigit(*str); ++str);

    if((*str == '.') && ((str + 1) < end) && std::isdigit(str[1]))
    {
        for(++str; (str < end) && std::isdigit(*str); ++str);
    }

    const size_t pos = size_t(start - ctx.begin);
    const size_t len = size_t(str   - start);

    ctx.tokens->push_back({token_type::number, symbol_id(0), pos, len});

    return true;
}

bool tokenizer::check_ident(const char*& str, const char* end, context& ctx) const
{
    if(!std::isalnum(*str)) return false;

    const char* start = str++;

    for( ; (str < end) && std::isalnum(*str); ++str);

    const size_t pos = size_t(start - ctx.begin);
    const size_t len = size_t(str   - start);

    symbol_id id;

    if(find_keyword(id, start, len))
    {
        ctx.tokens->push_back({token_type::keyword, id, pos, len});
    }
    else
    {
        ctx.tokens->push_back({token_type::ident, symbol_id(0), pos, len});
    }
    return true;
}

bool tokenizer::check_punct(const char*& str, const char* end, context& ctx) const
{
    if(!(std::ispunct(*str) && (*str != '"'))) return false;

    const char* start = str++;

    size_t len = 1;

    for( ; (len < m_max_punct_len) && (str < end) && std::ispunct(*str) && (*str != '"'); ++str, ++len);

    for(symbol_id id; len > 0; --str, --len)
    {
        if(find_punctuation(id, start, len))
        {
            const size_t pos = size_t(start - ctx.begin);

            ctx.tokens->push_back({token_type::punctuation, id, pos, len});
            break;
        }
    }
    if(len == 0)
    {
        ctx.err = parse_error::UnknownPunctuation;
        ctx.pos = str;
        return true;
    }
    return true;
}

void grammar::clear()
{
    m_start_index = npos;

    m_chunks .clear();
    m_rules  .clear();
    m_symbols.clear();
}

rule grammar::add_rule(symbol_id id)
{
    return add(id);
}

result_t grammar::prepare(symbol_id start_id)
{
    m_start_index = npos;

    m_rules  .clear();
    m_symbols.clear();

    size_t prev_rule_index = npos;
    size_t loops_count     = 0;

    for(size_t index = 0; index < m_chunks.size(); ++index)
    {
        const chunk_data& chunk = m_chunks[index];

        switch(chunk.type)
        {
            default: continue;

            case chunk_type::start: break;

            case chunk_type::loop:
                if((chunk.arg2 == 0) || (chunk.arg2 < chunk.arg1))
                {
                    return {parse_error::InvalidLoopArguments, chunk.id};
                }
                ++loops_count;
                continue;

            case chunk_type::next:
                if(loops_count == 0)
                {
                    return {parse_error::NextWithoutLoop, chunk.id};
                }
                --loops_count;
                continue;
        }
        if(prev_rule_index != npos)
        {
            Assert_Check(index > 0);
            m_rules[prev_rule_index].last_chunk = (index - 1);
        }
        if(loops_count != 0)
        {
            return {parse_error::MismatchLoopNextPairs, chunk.id};
        }
        symbol_data& symbol = m_symbols[find_or_add_symbol(chunk.id)];

        prev_rule_index = m_rules.size();

        m_rules.push_back({chunk.id, (unsigned)symbol.first_rule++, index + 1, npos});
    }
    if(prev_rule_index != npos)
    {
        Assert_Check(m_chunks.size() > 0);
        m_rules[prev_rule_index].last_chunk = (m_chunks.size() - 1);
    }
    std::sort(m_rules.begin(), m_rules.end());

    for(size_t index = 0; index < m_rules.size(); ++index)
    {
        rule_data& rule = m_rules[index];

        const size_t symbol_index = find_symbol_with_id(rule.id);
        Assert_Check(symbol_index != npos);

        symbol_data& symbol = m_symbols[symbol_index];

        if(symbol.first_rule > symbol.last_rule)
        {
            symbol.first_rule = index;
        }
        symbol.last_rule = symbol.first_rule + rule.order;
    }

    for(chunk_data& chunk : m_chunks)
    {
        if(chunk.type != chunk_type::symbol) continue;

        const size_t index = find_symbol_with_id(chunk.id);

        if(index == npos)
        {
            return {parse_error::SymbolWithoutRule, chunk.id};
        }
        chunk.type = chunk_type::rule;
        chunk.arg1 = index;
    }

    {
        const size_t index = find_symbol_with_id(start_id);

        if(index == npos)
        {
            return {parse_error::SymbolWithoutRule, start_id};
        }
        m_start_index = index;
    }
 
    return {parse_error::None, symbol_id(0)};
}

result_t grammar::check(
    const tokens_t& tokens,
    size_t index,
    size_t count
    )
    const
{
    Check_ValidState(m_start_index != npos, {parse_error::UnpreparedGramar});

    if(count == npos) count = tokens.size();

    Check_ValidArg(index < tokens.size(), {parse_error::InvalidArguments});

    const token_data* token = tokens.data() + index;

    const token_data* end = (index + count >= tokens.size())
        ? (tokens.data() + tokens.size())
        : (tokens.data() + (index + count));

    if(!verify_rule(token, end, m_start_index))
    {
        return {parse_error::GrammarCheckFailed};
    }

    return {parse_error::None};
}

size_t grammar::find_symbol_with_id(symbol_id id) const
{
    symbol_data symbol {id};

    auto it = std::lower_bound(m_symbols.begin(), m_symbols.end(), symbol);

    if(it == m_symbols.end()) return npos;
    if(id != it->id)          return npos;

    return (size_t)std::distance(m_symbols.begin(), it);
}
size_t grammar::find_or_add_symbol(symbol_id id)
{
    size_t index = find_symbol_with_id(id);

    if(index == npos)
    {
        m_symbols.push_back({id, 0, 0});
        std::sort(m_symbols.begin(), m_symbols.end());
        index = find_symbol_with_id(id);
        Assert_Check(index != npos);
    }
    return index;
}

bool grammar::verify_rule(const token_data*& token, const token_data* end, size_t symbol_index) const
{
    const size_t local_loop_stack_index = m_loop_stack.size();

    const symbol_data& symbol = m_symbols[symbol_index];

    const token_data* start_token = token;

    for(size_t rule_index = symbol.first_rule; rule_index <= symbol.last_rule; ++rule_index, token = start_token)
    {
        const rule_data& rule = m_rules[rule_index];

        size_t chunk_index = rule.first_chunk;

        for( ; chunk_index <= rule.last_chunk; ++chunk_index)
        {
            const chunk_data& chunk = m_chunks[chunk_index];

            switch(chunk.type)
            {
                case chunk_type::rule: if(verify_rule(token, end, chunk.arg1)) continue; break;

                case chunk_type::ident : if(verify_token(token, end, token_type::ident )) continue; break;
                case chunk_type::string: if(verify_token(token, end, token_type::string)) continue; break;
                case chunk_type::number: if(verify_token(token, end, token_type::number)) continue; break;

                case chunk_type::punctuation: if(verify_token(token, end, token_type::punctuation, chunk.id)) continue; break;
                case chunk_type::keyword    : if(verify_token(token, end, token_type::keyword    , chunk.id)) continue; break;

                case chunk_type::loop:
                    {
                        const size_t min_repeats = chunk.arg1;
                        const size_t max_repeats = chunk.arg2;

                        Assert_Check(max_repeats != 0);
                        Assert_Check(min_repeats <= max_repeats);

                        m_loop_stack.push_back({0, chunk.arg1, chunk.arg2, chunk_index, token});
                    }
                    continue;

                case chunk_type::next:
                    {
                        Assert_Check(!m_loop_stack.empty());

                        loop_data& current_loop = m_loop_stack.back();

                        if((current_loop.max_repeats == npos) && (token == current_loop.token))
                        {
                            //TODO:ERROR:Infinite loop detected
                            Assert_Fail();
                        }
                        if(++current_loop.cur_repeats == current_loop.max_repeats)
                        {
                            m_loop_stack.pop_back();
                        }
                        else
                        {
                            current_loop.token = token;

                            chunk_index = current_loop.first_index;
                        }
                    }
                    continue;

                default: Assert_Fail();
            }
            if(m_loop_stack.size() > local_loop_stack_index)
            {
                loop_data& current_loop = m_loop_stack.back();

                if(current_loop.min_repeats <= current_loop.cur_repeats)
                {
                    while(m_chunks[++chunk_index].type != chunk_type::next) Assert_Check(chunk_index < rule.last_chunk);
                    token = current_loop.token;
                    m_loop_stack.pop_back();
                    continue;
                }

                for( ; m_loop_stack.size() > local_loop_stack_index; m_loop_stack.pop_back());
            }
            break;
        }
        if(chunk_index > rule.last_chunk) return true;
    }
    token = start_token;
    return false;
}

bool grammar::verify_token(const token_data*& token, const token_data* end, token_type type) const
{
    if((token < end) && (token->type == type))
    {
        ++token;
        return true;
    }
    else
    {
        //error info
        return false;
    }
}

bool grammar::verify_token(const token_data*& token, const token_data* end, token_type type, symbol_id id) const
{
    if((token < end) && (token->type == type) && (token->id == id))
    {
        ++token;
        return true;
    }
    else
    {
        //error info
        return false;
    }
}

}
