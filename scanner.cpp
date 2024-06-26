#include <list>
#include <string>
#include <unordered_map>

#include "interp.hpp"
#include "ltype.hpp"
#include "token.hpp"

#include "scanner.hpp"

using enum Token::Type;

const std::unordered_map<std::string, const Token::Type> Scanner::keywords = {
    {"true", TRUE},         {"false", FALSE},
    {"and", AND},           {"class", CLASS},
    {"else", ELSE},         {"fun", FUN},
    {"for", FOR},           {"if", IF},
    {"nil", NIL},           {"or", OR},
    {"print", PRINT},       {"return", RETURN},
    {"super", SUPER},       {"this", THIS},
    {"var", VAR},           {"while", WHILE},
    {"continue", CONTINUE}, {"break", BREAK}};

Scanner::Scanner(std::string inputStr)
    : input(inputStr), start(0), current(0), lineNum(1), tokenList{} {}

std::list<Token> Scanner::scanTokens() {
  for (; current < input.size(); start = current)
    scanToken();

  tokenList.push_back(Token(EOFF, "", "", lineNum));
  return tokenList;
}

bool Scanner::compareAndNext(char expected) {
  if (current == input.size())
    return 0;
  if (input.at(current) != expected)
    return 0;
  current++;
  return 1;
}

void Scanner::scanToken() {
  char c;
  bool completeComment;

  double val;
  Token::Type type;

  if (current == input.size())
    return;
  switch (c = input.at(current++)) {
    case '(':
      addToken(LEFT_PAREN);
      break;
    case ')':
      addToken(RIGHT_PAREN);
      break;
    case '{':
      addToken(LEFT_BRACE);
      break;
    case '}':
      addToken(RIGHT_BRACE);
      break;
    case ',':
      addToken(COMMA);
      break;
    case '.':
      addToken(DOT);
      break;
    case '-':
      addToken(MINUS);
      break;
    case '+':
      addToken(PLUS);
      break;
    case ';':
      addToken(SEMICOLON);
      break;
    case '*':
      addToken(STAR);
      break;
    case '!':
      addToken(compareAndNext('=') ? BANG_EQUAL : BANG);
      break;
    case '<':
      addToken(compareAndNext('=') ? LESS_EQUAL : LESS);
      break;
    case '>':
      addToken(compareAndNext('=') ? GREATER_EQUAL : GREATER);
      break;
    case '=':
      addToken(compareAndNext('=') ? EQUAL_EQUAL : EQUAL);
      break;
    case '?':
      addToken(QUESTIONMARK);
      break;
    case ':':
      addToken(COLON);
      break;
    case '%':
      addToken(PERCENT);
      break;
    case '/':
      if (compareAndNext('/')) {
        while (current < input.size() && input.at(current) != '\n')
          current++;
        // /**/
      } else if (compareAndNext('*')) {
        completeComment = 0;
        for (; current < input.size(); current++) {
          if (compareAndNext('*') && compareAndNext('/')) {
            completeComment = 1;
            break;
          } else if (compareAndNext('/') && compareAndNext('*')) {
            Interp::error(lineNum, "nested multiline comments");
          }
        }
        if (!completeComment)
          Interp::error(lineNum, "unterminated comment");
      } else {
        addToken(SLASH);
      }
      break;
    case '"':
      while (current < input.size() && input.at(current) != '"') {
        if (input.at(current) == '\n')
          lineNum++;
        current++;
      }
      if (current == input.size())
        Interp::error(lineNum, "unterminated string");
      current++;
      addToken(STRING, input.substr(start + 1, current - start - 2));
      break;
    case ' ':
    case '\r':
    case '\t':  // FALLTHROUGH
      break;
    case '\n':
      lineNum++;
      break;
    default:
      // NUMBER
      if (std::isdigit(c)) {
        // std::stod allows this
        if (c == '0' && current < input.size() &&
            std::isdigit(input.at(current)))
          Interp::error(lineNum,
                        "multidigit number with"
                        " leading zero");
        while (current < input.size() && std::isdigit(input.at(current)))
          current++;

        if (current < input.size() && input.at(current) == '.' &&
            current + 1 < input.size() && std::isdigit(input.at(current + 1)))
          current++;

        while (current < input.size() && std::isdigit(input.at(current)))
          current++;

        val = std::stod(input.substr(start, current - start));
        addToken(NUMBER, val);
        // keyword, IDENTIFIER
      } else if (std::isalpha(c)) {
        while (current < input.size() &&
               (std::isalnum(input.at(current)) || input.at(current) == '_'))
          current++;
        auto ret = keywords.find(input.substr(start, current - start));
        if (ret != keywords.end())
          type = ret->second;
        else
          type = IDENTIFIER;
        addToken(type);
      } else {
        Interp::error(lineNum,
                      "unexpected character: '" + std::string(1, c) + "'");
      }
      break;
  }
}

void Scanner::addToken(Token::Type tt) {
  addToken(tt, "");
}

void Scanner::addToken(Token::Type tt, Literal literal) {
  tokenList.push_back(
      Token(tt, input.substr(start, current - start), literal, lineNum));
}
