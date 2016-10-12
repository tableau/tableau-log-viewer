#include "tokenizer.h"
#include <QDebug>
#include <QMessageBox>

Tokenizer::Tokenizer()
{
}

QList<Tokenizer::Token> Tokenizer::Tokenize(QString script)
{
    int i = 0;
    int length = script.length();

    QRegularExpressionMatch match;

    QList<Token> results;

    while (i < length)
    {
       for (LexicalRule rule : m_rules)
       {
           match = rule.regExp.match(script, i);
           if (match.hasMatch())
           {
               if (match.capturedLength() == 0)
               {
                   qDebug() << "Regex match length is zero";
               }

               //Add to results
               results.append(Token(i, match.capturedLength(), rule.type));
               i += match.capturedLength();
               break;
           }
       }
    }
    return results;
}

void Tokenizer::SetSQL()
{
    m_rules = {
        LexicalRule(TokenType::String, QRegularExpression("\\G'([^']*)'")),
        LexicalRule(TokenType::Tag, QRegularExpression("\\G([a-zA-Z\\d_\\$-]+:)")),
        LexicalRule(TokenType::Keyword, QRegularExpression("\\G(ON|COMMIT|PRESERVE|INTEGER|ROWS|LOCAL|TEMPORARY|ELSE|CASE|END|WHEN|THEN|CREATE|INSERT|DROP|DELETE|INDEX|TABLE|SELECT|WHERE|INTO|VALUES|ON|AS|FROM|AND|NOT|LEFT|RIGHT|INNER|OUTER|JOIN|ORDER|GROUP|BY|IN|OR|SUM)\\b", QRegularExpression::CaseInsensitiveOption)),
        LexicalRule(TokenType::Whitespace, QRegularExpression("\\G([\\s]+)")),
        LexicalRule(TokenType::Float, QRegularExpression("\\G[\\d]+\\.[\\d]+")),
        LexicalRule(TokenType::Integer, QRegularExpression("\\G[\\d]+")),
        LexicalRule(TokenType::Identifier, QRegularExpression("\\G([a-zA-Z_\\$][a-zA-Z_\\d\\$]*|(`([^`]*)`)|(\\[([^\\]]*)\\])|\\\"([^\\\"]*)\\\")")),
        LexicalRule(TokenType::Symbol, QRegularExpression("\\G[\\.,\\}\\{]+")),
        LexicalRule(TokenType::OpenParan, QRegularExpression("\\G\\(")),
        LexicalRule(TokenType::CloseParan, QRegularExpression("\\G\\)")),
        LexicalRule(TokenType::Operator, QRegularExpression("\\G((!=)|(==)|(<=)|(>=)|(<>)|(\\*\\*)|=|<|>|\\+|-)")),
        LexicalRule(TokenType::NoMatch, QRegularExpression("\\G."))
    };
}

