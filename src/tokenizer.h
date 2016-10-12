#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QList>
#include <QRegularExpression>

class Tokenizer
{
public:
    enum TokenType
    {
        String,
        Keyword,
        Tag,
        Whitespace,
        Identifier,
        Symbol,
        OpenParan,
        CloseParan,
        Operator,
        Integer,
        Float,
        NoMatch
    };

    struct LexicalRule {
        TokenType type;
        QRegularExpression regExp;

        LexicalRule(TokenType type, QRegularExpression regExp) :
            type(type),
            regExp(regExp)
        {}
    };

    struct Token {
        int start;
        int length;
        TokenType type;

        Token(int start, int length, TokenType type) :
            start(start),
            length(length),
            type(type)
        {}
    };

    Tokenizer();
    QList<Token> Tokenize(QString in);
    void SetSQL();

    QList<LexicalRule> m_rules;
};

#endif // TOKENIZER_H
