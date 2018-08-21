// This file was generated on Mon Aug 20, 2018 21:52 (UTC+02) by REx v5.47 which is Copyright (c) 1979-2017 by Gunther Rademacher <grd@gmx.net>
// REx command line: csparser.ebnf -name csparser -tree -cpp -faster

#ifndef CSPARSER_HPP
#define CSPARSER_HPP

#include <vector>
#include <stdio.h>
#include <string>

class csparser
{
public:
  class EventHandler;

  csparser(const wchar_t *string, EventHandler *t)
  {
    initialize(string, t);
  }

  virtual ~csparser()
  {
  }

  class EventHandler
  {
  public:
    virtual ~EventHandler() {}

    virtual void reset(const wchar_t *string) = 0;
    virtual void startNonterminal(const wchar_t *name, int begin) = 0;
    virtual void endNonterminal(const wchar_t *name, int end) = 0;
    virtual void terminal(const wchar_t *name, int begin, int end) = 0;
    virtual void whitespace(int begin, int end) = 0;
  };

  class Symbol
  {
  public:
    virtual ~Symbol() {}

    const wchar_t *name;
    int begin;
    int end;

    virtual void send(EventHandler *e) = 0;

  protected:
    Symbol(const wchar_t *name, int begin, int end)
    {
      this->name = name;
      this->begin = begin;
      this->end = end;
    }
  };

  class Terminal : public Symbol
  {
  public:
    Terminal(const wchar_t *name, int begin, int end)
    : Symbol(name, begin, end)
    {}

    void send(EventHandler *e)
    {
      e->terminal(name, begin, end);
    }
  };

  class Nonterminal : public Symbol
  {
  public:
    std::vector<Symbol *> *children;

    Nonterminal(const wchar_t *name, int begin, int end, std::vector<Symbol *> *children)
    : Symbol(name, begin, end)
    {
      this->children = children;
    }

    ~Nonterminal()
    {
      for (std::vector<Symbol *>::iterator child = children->begin(); child != children->end(); ++child)
        delete *child;
      delete children;
    }

    void send(EventHandler *e)
    {
      e->startNonterminal(name, begin);
      int pos = begin;
      for (std::vector<Symbol *>::iterator i = children->begin(); i != children->end(); ++i)
      {
        Symbol *c = *i;
        if (pos < c->begin) e->whitespace(pos, c->begin);
        c->send(e);
        pos = c->end;
      }
      if (pos < end) e->whitespace(pos, end);
      e->endNonterminal(name, end);
    }
  };

  class TopDownTreeBuilder : public EventHandler
  {
  public:
    TopDownTreeBuilder()
    {
      input = 0;
      stack.clear();
      top = -1;
    }

    void reset(const wchar_t *input)
    {
      this->input = input;
      top = -1;
    }

    void startNonterminal(const wchar_t *name, int begin)
    {
      Nonterminal *nonterminal = new Nonterminal(name, begin, begin, new std::vector<Symbol *>());
      if (top++ >= 0) addChild(nonterminal);
      if ((size_t) top >= stack.size())
        stack.resize(stack.size() == 0 ? 64 : stack.size() << 1);
      stack[top] = nonterminal;
    }

    void endNonterminal(const wchar_t *name, int end)
    {
      stack[top]->end = end;
      if (top > 0) --top;
    }

    void terminal(const wchar_t *name, int begin, int end)
    {
      addChild(new Terminal(name, begin, end));
    }

    void whitespace(int begin, int end)
    {
    }

    void serialize(EventHandler *e)
    {
      e->reset(input);
      stack[0]->send(e);
    }

  private:
    void addChild(Symbol *s)
    {
      Nonterminal *current = stack[top];
      current->children->push_back(s);
    }

    const wchar_t *input;
    std::vector<Nonterminal *> stack;
    int top;
  };

  class ParseException
  {
  private:
    int begin, end, offending, expected, state;
    friend class csparser;

  protected:
    ParseException(int b, int e, int s, int o, int x)
    : begin(b), end(e), offending(o), expected(x), state(s)
    {
    }

  public:
    const wchar_t *getMessage() const
    {
      return offending < 0
           ? L"lexical analysis failed"
           : L"syntax error";
    }

    int getBegin() const {return begin;}
    int getEnd() const {return end;}
    int getState() const {return state;}
    int getOffending() const {return offending;}
    int getExpected() const {return expected;}
  };

  void initialize(const wchar_t *source, EventHandler *parsingEventHandler)
  {
    eventHandler = parsingEventHandler;
    input = source;
    reset(0, 0, 0);
  }

  const wchar_t *getInput() const
  {
    return input;
  }

  int getTokenOffset() const
  {
    return b0;
  }

  int getTokenEnd() const
  {
    return e0;
  }

  void reset(int l, int b, int e)
  {
            b0 = b; e0 = b;
    l1 = l; b1 = b; e1 = e;
    l2 = 0;
    end = e;
    eventHandler->reset(input);
  }

  void reset()
  {
    reset(0, 0, 0);
  }

  static const wchar_t *getOffendingToken(ParseException e)
  {
    return e.getOffending() < 0 ? 0 : TOKEN[e.getOffending()];
  }

  static void getExpectedTokenSet(const ParseException &e, const wchar_t **set, int size)
  {
    if (e.expected < 0)
    {
      getTokenSet(- e.state, set, size);
    }
    else if (size == 1)
    {
      set[0] = 0;
    }
    else if (size > 1)
    {
      set[0] = TOKEN[e.expected];
      set[1] = 0;
    }
  }

  std::wstring getErrorMessage(const ParseException &e)
  {
    std::wstring message(e.getMessage());
    wchar_t buffer[11];
    const wchar_t *found = getOffendingToken(e);
    if (found != 0)
    {
      message += L", found ";
      message += found;
    }
    const wchar_t *expected[64];
    getExpectedTokenSet(e, expected, sizeof expected / sizeof *expected);
    message += L"\nwhile expecting ";
    const wchar_t *delimiter(expected[1] ? L"[" : L"");
    for (const wchar_t **x = expected; *x; ++x)
    {
      message += delimiter;
      message += *x;
      delimiter = L", ";
    }
    message += expected[1] ? L"]\n" : L"\n";
    int size = e.getEnd() - e.getBegin();
    if (size != 0 && found == 0)
    {
      message += L"after successfully scanning ";
//      swprintf(buffer, L"%d", size);
//      message += buffer;
      message += L" characters beginning ";
    }
    int line = 1;
    int column = 1;
    for (int i = 0; i < e.getBegin(); ++i)
    {
      if (input[i] == L'\n')
      {
        ++line;
        column = 1;
      }
      else
      {
        ++column;
      }
    }
    message += L"at line ";
//    swprintf(buffer, L"%d", line);
//    message += buffer;
    message += L", column ";
//    swprintf(buffer, L"%d", column);
//    message += buffer;
    message += L":\n...";
    const wchar_t *w = input + e.getBegin();
    for (int i = 0; i < 64 && *w; ++i)
    {
      message += *w++;
    }
    message += L"...";
    return message;
  }

  void parse_CS()
  {
    eventHandler->startNonterminal(L"CS", e0);
    for (;;)
    {
      lookahead1W(13);              // S^WS | EOF | '"' | '$' | '//' | '['
      if (l1 == 10)                 // EOF
      {
        break;
      }
      whitespace();
      parse_Expr();
    }
    consume(10);                    // EOF
    eventHandler->endNonterminal(L"CS", e0);
  }

private:

  void parse_Expr()
  {
    eventHandler->startNonterminal(L"Expr", e0);
    switch (l1)
    {
    case 13:                        // '$'
      parse_var();
      lookahead1W(0);               // Operator | S^WS | '//'
      consume(3);                   // Operator
      lookahead1W(9);               // S^WS | '"' | '//' | '['
      switch (l1)
      {
      case 11:                      // '"'
        whitespace();
        parse_literal();
        break;
      default:
        whitespace();
        parse_URI();
        break;
      }
      break;
    default:
      parse_items();
      if (l1 == 3)                  // Operator
      {
        whitespace();
        parse_statement();
        for (;;)
        {
          if (l1 != 3)              // Operator
          {
            break;
          }
          whitespace();
          parse_statement();
        }
      }
      break;
    }
    lookahead1W(17);                // S^WS | EOF | '"' | '$' | '//' | ';' | '['
    if (l1 == 17)                   // ';'
    {
      whitespace();
      parse_Separator();
    }
    eventHandler->endNonterminal(L"Expr", e0);
  }

  void parse_statement()
  {
    eventHandler->startNonterminal(L"statement", e0);
    consume(3);                     // Operator
    lookahead1W(9);                 // S^WS | '"' | '//' | '['
    whitespace();
    parse_items();
    eventHandler->endNonterminal(L"statement", e0);
  }

  void parse_var()
  {
    eventHandler->startNonterminal(L"var", e0);
    consume(13);                    // '$'
    lookahead1W(1);                 // nstring | S^WS | '//'
    consume(4);                     // nstring
    eventHandler->endNonterminal(L"var", e0);
  }

  void parse_items()
  {
    eventHandler->startNonterminal(L"items", e0);
    parse_item();
    for (;;)
    {
      lookahead1W(18);              // Operator | S^WS | EOF | '"' | '$' | ',' | '//' | ';' | '['
      if (l1 != 14)                 // ','
      {
        break;
      }
      consume(14);                  // ','
      lookahead1W(9);               // S^WS | '"' | '//' | '['
      whitespace();
      parse_item();
    }
    eventHandler->endNonterminal(L"items", e0);
  }

  void parse_item()
  {
    eventHandler->startNonterminal(L"item", e0);
    switch (l1)
    {
    case 19:                        // '['
      parse_URI();
      break;
    default:
      parse_literal();
      break;
    }
    eventHandler->endNonterminal(L"item", e0);
  }

  void parse_Separator()
  {
    eventHandler->startNonterminal(L"Separator", e0);
    consume(17);                    // ';'
    eventHandler->endNonterminal(L"Separator", e0);
  }

  void parse_literal()
  {
    eventHandler->startNonterminal(L"literal", e0);
    consume(11);                    // '"'
    lookahead1W(3);                 // astring | S^WS | '//'
    consume(6);                     // astring
    lookahead1W(5);                 // S^WS | '"' | '//'
    consume(11);                    // '"'
    eventHandler->endNonterminal(L"literal", e0);
  }

  void parse_URI()
  {
    eventHandler->startNonterminal(L"URI", e0);
    consume(19);                    // '['
    lookahead1W(11);                // scheme | nstring | path_delim | S^WS | '//'
    if (l1 == 2)                    // scheme
    {
      consume(2);                   // scheme
    }
    lookahead1W(8);                 // nstring | path_delim | S^WS | '//'
    switch (l1)
    {
    case 4:                         // nstring
      whitespace();
      parse_hostport();
      break;
    default:
      whitespace();
      parse_segment();
      break;
    }
    for (;;)
    {
      lookahead1W(12);              // path_delim | S^WS | '#' | '//' | '?' | ']'
      if (l1 != 7)                  // path_delim
      {
        break;
      }
      whitespace();
      parse_segment();
    }
    if (l1 == 18)                   // '?'
    {
      whitespace();
      parse_query();
    }
    lookahead1W(10);                // S^WS | '#' | '//' | ']'
    if (l1 == 12)                   // '#'
    {
      whitespace();
      parse_fragment();
    }
    lookahead1W(6);                 // S^WS | '//' | ']'
    consume(20);                    // ']'
    eventHandler->endNonterminal(L"URI", e0);
  }

  void parse_hostport()
  {
    eventHandler->startNonterminal(L"hostport", e0);
    parse_host();
    lookahead1W(16);                // path_delim | S^WS | '#' | '//' | ':' | '?' | ']'
    if (l1 == 16)                   // ':'
    {
      consume(16);                  // ':'
      lookahead1W(4);               // digit | S^WS | '//'
      whitespace();
      parse_port();
    }
    eventHandler->endNonterminal(L"hostport", e0);
  }

  void parse_host()
  {
    eventHandler->startNonterminal(L"host", e0);
    consume(4);                     // nstring
    eventHandler->endNonterminal(L"host", e0);
  }

  void parse_port()
  {
    eventHandler->startNonterminal(L"port", e0);
    consume(8);                     // digit
    for (;;)
    {
      lookahead1W(15);              // path_delim | digit | S^WS | '#' | '//' | '?' | ']'
      if (l1 != 8)                  // digit
      {
        break;
      }
      consume(8);                   // digit
    }
    eventHandler->endNonterminal(L"port", e0);
  }

  void parse_segment()
  {
    eventHandler->startNonterminal(L"segment", e0);
    consume(7);                     // path_delim
    lookahead1W(2);                 // string | S^WS | '//'
    consume(5);                     // string
    lookahead1W(12);                // path_delim | S^WS | '#' | '//' | '?' | ']'
    switch (l1)
    {
    case 7:                         // path_delim
      lookahead2W(14);              // string | path_delim | S^WS | '#' | '//' | '?' | ']'
      break;
    default:
      lk = l1;
      break;
    }
    if (lk == 231                   // path_delim path_delim
     || lk == 391                   // path_delim '#'
     || lk == 583                   // path_delim '?'
     || lk == 647)                  // path_delim ']'
    {
      consume(7);                   // path_delim
    }
    eventHandler->endNonterminal(L"segment", e0);
  }

  void parse_query()
  {
    eventHandler->startNonterminal(L"query", e0);
    consume(18);                    // '?'
    lookahead1W(2);                 // string | S^WS | '//'
    consume(5);                     // string
    eventHandler->endNonterminal(L"query", e0);
  }

  void parse_fragment()
  {
    eventHandler->startNonterminal(L"fragment", e0);
    consume(12);                    // '#'
    lookahead1W(2);                 // string | S^WS | '//'
    consume(5);                     // string
    eventHandler->endNonterminal(L"fragment", e0);
  }

  void try_comments()
  {
    consumeT(15);                   // '//'
    lookahead1W(3);                 // astring | S^WS | '//'
    consumeT(6);                    // astring
    for (;;)
    {
      lookahead1W(7);               // END | astring | S^WS | '//'
      if (l1 != 6)                  // astring
      {
        break;
      }
      consumeT(6);                  // astring
    }
  }

  void try_whitespace()
  {
    switch (l1)
    {
    case 9:                         // S^WS
      consumeT(9);                  // S^WS
      break;
    default:
      try_comments();
      break;
    }
  }

  void consume(int t)
  {
    if (l1 == t)
    {
      whitespace();
      eventHandler->terminal(TOKEN[l1], b1, e1);
      b0 = b1; e0 = e1; l1 = l2; if (l1 != 0) {
      b1 = b2; e1 = e2; l2 = 0; }
    }
    else
    {
      error(b1, e1, 0, l1, t);
    }
  }

  void consumeT(int t)
  {
    if (l1 == t)
    {
      b0 = b1; e0 = e1; l1 = l2; if (l1 != 0) {
      b1 = b2; e1 = e2; l2 = 0; }
    }
    else
    {
      error(b1, e1, 0, l1, t);
    }
  }

  void skip(int code)
  {
    int b0W = b0; int e0W = e0; int l1W = l1;
    int b1W = b1; int e1W = e1;

    l1 = code; b1 = begin; e1 = end;
    l2 = 0;

    try_whitespace();

    b0 = b0W; e0 = e0W; l1 = l1W; if (l1 != 0) {
    b1 = b1W; e1 = e1W; }
  }

  void whitespace()
  {
    if (e0 != b1)
    {
      eventHandler->whitespace(e0, b1);
      e0 = b1;
    }
  }

  int matchW(int set)
  {
    int code;
    for (;;)
    {
      code = match(set);
      if (code != 9)                // S^WS
      {
        if (code != 15)             // '//'
        {
          break;
        }
        skip(code);
      }
    }
    return code;
  }

  void lookahead1W(int set)
  {
    if (l1 == 0)
    {
      l1 = matchW(set);
      b1 = begin;
      e1 = end;
    }
  }

  void lookahead2W(int set)
  {
    if (l2 == 0)
    {
      l2 = matchW(set);
      b2 = begin;
      e2 = end;
    }
    lk = (l2 << 5) | l1;
  }

  int error(int b, int e, int s, int l, int t)
  {
    throw ParseException(b, e, s, l, t);
  }

  int lk, b0, e0;
  int l1, b1, e1;
  int l2, b2, e2;
  EventHandler *eventHandler;

  const wchar_t *input;
  int begin;
  int end;

  int match(int tokenSetId)
  {
    begin = end;
    int current = end;
    int result = INITIAL[tokenSetId];
    int state = 0;

    for (int code = result & 63; code != 0; )
    {
      int charclass;
      int c0 = input[current];
      ++current;
      if (c0 < 0x80)
      {
        charclass = MAP0[c0];
      }
      else if (c0 < 0xd800)
      {
        int c1 = c0 >> 5;
        charclass = MAP1[(c0 & 31) + MAP1[(c1 & 31) + MAP1[c1 >> 5]]];
      }
      else
      {
        charclass = 0;
      }

      state = code;
      int i0 = (charclass << 6) + code - 1;
      code = TRANSITION[(i0 & 7) + TRANSITION[i0 >> 3]];
      if (code > 63)
      {
        result = code;
        code &= 63;
        end = current;
      }
    }

    result >>= 6;
    if (result == 0)
    {
      end = current - 1;
      int c1 = input[end];
      if (c1 >= 0xdc00 && c1 < 0xe000) --end;
      return error(begin, end, state, -1, -1);
    }

    if (input[begin] == 0) end = begin;
    return (result & 31) - 1;
  }

  class MalformedInputException
  {
  public:
    MalformedInputException(size_t offset) : offset(offset) {}
    size_t getOffset() const {return offset;}

  private:
    size_t offset;
  };

  class Utf8Encoder
  {
  public:
    static std::string encode(const wchar_t *unencoded)
    {
      return encode(unencoded, wcslen(unencoded));
    }

    static std::string encode(const wchar_t *unencoded, size_t size)
    {
      std::string encoded;
      encoded.reserve(size + 3);

      for (size_t i = 0; i < size; ++i)
      {
        if (encoded.size() + 4 >= encoded.capacity()) encoded.reserve(encoded.capacity() * 2);

        int w = unencoded[i];
        if (w < 0x80)
        {
          encoded += w;
        }
        else if (w < 0x800)
        {
          encoded += 0xc0 | (w >> 6);
          encoded += 0x80 | (w & 0x3f);
        }
        else if (w < 0xd800)
        {
          encoded += 0xe0 | ( w          >> 12);
          encoded += 0x80 | ((w & 0xfff) >>  6);
          encoded += 0x80 | ( w &  0x3f       );
        }
        else if (w < 0xe000)
        {
          if (++i >= size)
          {
            throw MalformedInputException(i - 1);
          }
          int w2 = unencoded[i];
          if (w2 < 0xdc00 || w2 > 0xdfff)
          {
            throw MalformedInputException(i - 1);
          }
          w = (((w  & 0x3ff) << 10) | (w2 & 0x3ff)) + 0x10000;
          encoded += 0xf0 | ( w            >> 18);
          encoded += 0x80 | ((w & 0x3ffff) >> 12);
          encoded += 0x80 | ((w &   0xfff) >>  6);
          encoded += 0x80 | ( w &    0x3f       );
        }
        else if (w < 0x10000)
        {
          encoded += 0xe0 | ( w          >> 12);
          encoded += 0x80 | ((w & 0xfff) >>  6);
          encoded += 0x80 | ( w &  0x3f       );
        }
        else if (w < 0x110000)
        {
          encoded += 0xf0 | ( w            >> 18);
          encoded += 0x80 | ((w & 0x3ffff) >> 12);
          encoded += 0x80 | ((w &   0xfff) >>  6);
          encoded += 0x80 | ( w &    0x3f       );
        }
        else
        {
          throw MalformedInputException(i);
        }
      }
      return encoded;
    }
  };

  static void getTokenSet(int tokenSetId, const wchar_t **set, int size)
  {
    int s = tokenSetId < 0 ? - tokenSetId : INITIAL[tokenSetId] & 63;
    for (int i = 0; i < 21; i += 32)
    {
      int j = i;
      for (unsigned int f = ec(i >> 5, s); f != 0; f >>= 1, ++j)
      {
        if ((f & 1) != 0)
        {
          if (size > 1)
          {
            set[0] = TOKEN[j];
            ++set;
            --size;
          }
        }
      }
    }
    if (size > 0)
    {
      set[0] = 0;
    }
  }

  static int ec(int t, int s)
  {
    int i0 = t * 35 + s - 1;
    return EXPECTED[i0];
  }

  static const int MAP0[];
  static const int MAP1[];
  static const int INITIAL[];
  static const int TRANSITION[];
  static const int EXPECTED[];
  static const wchar_t *TOKEN[];
};

const int csparser::MAP0[] =
{
/*   0 */ 27, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
/*  36 */ 5, 0, 0, 0, 0, 0, 0, 0, 6, 0, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 11, 0, 12, 13, 14, 0, 15, 15, 15, 15,
/*  69 */ 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 0, 17, 0, 7, 0,
/*  97 */ 15, 15, 15, 15, 18, 19, 15, 20, 21, 15, 15, 22, 15, 15, 15, 23, 15, 15, 24, 25, 15, 15, 15, 15, 15, 15, 0, 26,
/* 125 */ 0, 0, 0
};

const int csparser::MAP1[] =
{
/*   0 */ 54, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
/*  27 */ 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
/*  54 */ 90, 136, 199, 168, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
/*  76 */ 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 27, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
/* 103 */ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
/* 140 */ 5, 0, 0, 0, 0, 0, 0, 0, 6, 0, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 11, 0, 12, 13, 14, 0, 15, 15, 15, 15,
/* 173 */ 18, 19, 15, 20, 21, 15, 15, 22, 15, 15, 15, 23, 15, 15, 24, 25, 15, 15, 15, 15, 15, 15, 0, 26, 0, 0, 0, 15,
/* 201 */ 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 0, 17,
/* 229 */ 0, 7
};

const int csparser::INITIAL[] =
{
/*  0 */ 1, 2, 3, 4, 5, 6, 7, 132, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
};

const int csparser::TRANSITION[] =
{
/*   0 */ 244, 244, 244, 244, 244, 244, 244, 244, 224, 224, 229, 244, 244, 244, 244, 244, 240, 244, 397, 243, 244, 244,
/*  22 */ 244, 244, 290, 253, 266, 244, 244, 244, 244, 244, 244, 519, 244, 244, 244, 244, 244, 244, 244, 382, 276, 244,
/*  44 */ 244, 244, 244, 244, 244, 244, 287, 244, 244, 244, 244, 244, 345, 232, 399, 298, 305, 244, 244, 244, 314, 319,
/*  66 */ 327, 244, 378, 244, 244, 244, 363, 279, 399, 298, 305, 244, 244, 244, 244, 306, 244, 245, 357, 244, 244, 244,
/*  88 */ 244, 244, 371, 244, 244, 244, 244, 244, 240, 244, 414, 243, 244, 244, 244, 244, 390, 244, 431, 243, 244, 244,
/* 110 */ 244, 244, 436, 258, 416, 243, 244, 244, 244, 244, 491, 496, 399, 298, 305, 244, 244, 244, 244, 407, 424, 244,
/* 132 */ 244, 244, 244, 244, 268, 444, 244, 244, 244, 244, 244, 244, 491, 496, 399, 452, 305, 244, 244, 244, 491, 333,
/* 154 */ 399, 298, 305, 244, 244, 244, 491, 339, 399, 298, 305, 244, 244, 244, 491, 496, 399, 460, 305, 244, 244, 244,
/* 176 */ 491, 496, 399, 468, 305, 244, 244, 244, 491, 496, 399, 476, 305, 244, 244, 244, 491, 496, 399, 298, 484, 244,
/* 198 */ 244, 244, 491, 496, 399, 504, 305, 244, 244, 244, 436, 244, 416, 243, 244, 244, 244, 244, 244, 349, 512, 244,
/* 220 */ 244, 244, 244, 244, 659, 659, 659, 659, 659, 659, 659, 659, 0, 0, 0, 0, 0, 408, 0, 0, 276, 0, 0, 473, 0, 0, 0,
/* 247 */ 0, 0, 0, 0, 0, 34, 768, 0, 0, 0, 768, 0, 0, 0, 1216, 0, 1216, 1216, 1216, 768, 768, 0, 0, 0, 0, 0, 0, 1344, 0,
/* 276 */ 896, 896, 0, 0, 0, 0, 0, 0, 408, 576, 0, 0, 960, 0, 0, 0, 0, 0, 0, 768, 0, 0, 473, 343, 343, 343, 343, 343,
/* 304 */ 343, 343, 0, 0, 0, 0, 0, 0, 0, 1088, 21, 21, 21, 21, 21, 21, 21, 533, 533, 21, 533, 533, 533, 21, 21, 0, 0,
/* 331 */ 1024, 0, 0, 0, 346, 0, 0, 408, 0, 0, 347, 0, 0, 408, 0, 0, 408, 473, 0, 0, 0, 0, 704, 0, 0, 0, 34, 0, 0, 0, 0,
/* 362 */ 0, 0, 0, 408, 473, 576, 0, 0, 0, 1152, 1152, 0, 0, 0, 0, 0, 0, 35, 192, 0, 0, 0, 0, 0, 896, 0, 0, 0, 278, 0,
/* 392 */ 0, 473, 0, 0, 0, 0, 276, 0, 0, 0, 0, 0, 0, 343, 408, 1280, 0, 0, 0, 1280, 0, 0, 0, 276, 0, 256, 0, 0, 0, 0, 0,
/* 423 */ 0, 1280, 1280, 0, 0, 0, 0, 0, 0, 278, 0, 0, 0, 256, 0, 0, 473, 0, 0, 0, 0, 0, 1344, 0, 1344, 0, 1344, 1344,
/* 451 */ 1344, 473, 343, 343, 343, 343, 352, 343, 343, 473, 348, 343, 343, 343, 343, 343, 343, 473, 343, 343, 350, 343,
/* 473 */ 343, 343, 343, 473, 343, 343, 343, 343, 343, 353, 343, 352, 0, 0, 0, 0, 0, 0, 0, 343, 408, 473, 0, 0, 0, 343,
/* 499 */ 0, 0, 408, 0, 0, 473, 343, 349, 343, 351, 343, 343, 343, 704, 704, 0, 0, 0, 0, 0, 0, 832, 0, 832, 0, 832, 832,
/* 526 */ 832
};

const int csparser::EXPECTED[] =
{
/*  0 */ 33288, 33296, 33312, 33344, 33536, 35328, 1081856, 33424, 559616, 1085952, 33428, 1348224, 568832, 1348256,
/* 14 */ 1348480, 1413760, 699904, 716296, 512, 8, 32768, 8, 16, 32, 64, 20, 20, 20, 20, 20, 20, 20, 20, 4, 4
};

const wchar_t *csparser::TOKEN[] =
{
  L"(0)",
  L"END",
  L"scheme",
  L"Operator",
  L"nstring",
  L"string",
  L"astring",
  L"'/'",
  L"digit",
  L"S",
  L"EOF",
  L"'\"'",
  L"'#'",
  L"'$'",
  L"','",
  L"'//'",
  L"':'",
  L"';'",
  L"'?'",
  L"'['",
  L"']'"
};

#endif

// End
