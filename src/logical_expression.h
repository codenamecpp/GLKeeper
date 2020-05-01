#pragma once

#include "common_utils.h"

namespace cxx
{
    // evaluates logica expression
    // grammar:
    // !, ||, &&, (, ), name_Id
    // example
    // !(aaa && bbb || ccc) && (ddd) || (((eee)))

    // BNF notation:
    /*
        <Identifier> ::= ...
        <Factor> ::=
            <Variable> |
            '(' <Expression> ')'
        <NotExpression> ::=
            <Factor> |
            '!' <Factor>
        <Expression> ::=
            <NotExpression> |
            <NotExpression> '&&' <NotExpression> |
            <NotExpression> '||' <NotExpression>
    */

    class logical_expression
    {
    public:
        logical_expression() = default;
        logical_expression(logical_expression&& expr) = delete;
        logical_expression(const logical_expression& expr)
            : mExpressionSource(expr.mExpressionSource)
        {
            parse_root_expression();
        }
        logical_expression& operator = (logical_expression&& expr) = delete;
        // assign logical expression
        inline logical_expression& operator = (const logical_expression& expr)
        {
            if (&expr == this)
            {
                debug_assert(false);
            }
            else
            {
                clear();

                mExpressionSource = expr.mExpressionSource;
                parse_root_expression();
            }
            return *this;
        }

        // parse expression string
        inline bool parse_expression(const std::string& expression_string)
        {
            clear();

            mExpressionSource = expression_string;
            if (!mExpressionSource.empty())
            {
                mExpressionSource.erase(std::remove_if(
                    mExpressionSource.begin(), 
                    mExpressionSource.end(), ::isspace), mExpressionSource.end());

                return parse_root_expression();
            }
            return true;
        }

        // set null
        inline void clear()
        {
            mNodesBuffer.clear();
            mExpressionSource.clear();
            mExpressionRoot = nullptr;
        }

        // test whether expression non null
        inline bool non_null() const 
        { 
            return mExpressionRoot != nullptr; 
        }

        // get expression result
        // @param func: Boolean values provider
        template<typename TFunc>
        inline bool evaluate_expression(TFunc func) const
        {
            if (non_null())
            {
                return evaluate_node(mExpressionRoot, func);
            }

            debug_assert(false);
            return false;
        }

    private:

        // syntax tree node identifier
        enum SyntreeNodeID
        {
            SyntreeNode_NOT,
            SyntreeNode_OR,
            SyntreeNode_AND,
            SyntreeNode_Identifier,
        };

        // syntax tree node struct
        struct SyntreeNode
        {
            SyntreeNode(SyntreeNodeID nodeID): mNodeID(nodeID)
            {
            }
            SyntreeNodeID mNodeID;
            SyntreeNode* mExpressionL = nullptr;
            SyntreeNode* mExpressionR = nullptr;
            unique_string mIdentifier;
        };

        // input characters stream helper
        struct characters_stream
        {
        public:
            inline char peek_char() const { return *mCursor; }
            inline char get_char()
            {
                char nextChar = *mCursor;
                ++mCursor;
                return nextChar;
            }
            inline bool peek_string(const char* substring) const
            {
                int stringLength = strlen(substring);
                return strncmp(mCursor, substring, stringLength) == 0;   
            }
            inline bool match_string(const char* substring)
            {
                int stringLength = strlen(substring);
                if (strncmp(mCursor, substring, stringLength) == 0)
                {
                    mCursor += stringLength;
                    return true;
                }
                return false;
            }
            inline bool is_eos() const { return *mCursor == 0; }
        public:
            const char* mString;
            const char* mCursor;
        };

    private:

        bool parse_root_expression()
        {
            characters_stream charactersStream;
            charactersStream.mString = mExpressionSource.c_str();
            charactersStream.mCursor = mExpressionSource.c_str();

            if (charactersStream.is_eos())
                return true;

            mExpressionRoot = parse_expression(charactersStream);
            debug_assert(mExpressionRoot);
            if (mExpressionRoot == nullptr)
            {
                mNodesBuffer.clear();
            }
            return mExpressionRoot != nullptr;
        }

        SyntreeNode* parse_expression(characters_stream& charactersStream)
        {
            // left hand side expression
            SyntreeNode* lhs_expression = parse_not_expression(charactersStream);        
            if (lhs_expression == nullptr)
            {
                debug_assert(false);
                return nullptr;
            }

            // right hand side expression
            for (SyntreeNodeID expressionNodeID = SyntreeNode_Identifier;;)
            {
                if (charactersStream.match_string("&&"))
                {
                    expressionNodeID = SyntreeNode_AND;
                }
                else if (charactersStream.match_string("||"))
                {
                    expressionNodeID = SyntreeNode_OR;
                }
                else
                {
                    return lhs_expression;
                }

                SyntreeNode* rhs_expression = parse_not_expression(charactersStream);
                if (rhs_expression == nullptr)
                {
                    debug_assert(false);
                    return nullptr;
                }

                SyntreeNode* full_expression = new_syntree_node(expressionNodeID);
                full_expression->mExpressionL = lhs_expression;
                full_expression->mExpressionR = rhs_expression;
                lhs_expression = full_expression;
            }
            return lhs_expression;
        }

        SyntreeNode* parse_not_expression(characters_stream& charactersStream)
        {
            SyntreeNode* currNode = nullptr;
            if (charactersStream.match_string("!"))
            {
                currNode = new_syntree_node(SyntreeNode_NOT);
                if (SyntreeNode* lhs_expression = parse_factor_expression(charactersStream))
                {
                    currNode->mExpressionL = lhs_expression;
                }
                else
                {
                    debug_assert(false);
                    return nullptr;
                }
            }
            else
            {
                currNode = parse_factor_expression(charactersStream);
            }
            debug_assert(currNode);
            return currNode;
        }

        SyntreeNode* parse_factor_expression(characters_stream& charactersStream)
        {
            SyntreeNode* expression_node = nullptr;
            if (charactersStream.peek_string("("))
            {
                expression_node = parse_sub_expression(charactersStream);
            }
            else
            {
                expression_node = parse_identifier_expression(charactersStream);
            }
            debug_assert(expression_node);
            return expression_node;
        }

        SyntreeNode* parse_sub_expression(characters_stream& charactersStream)
        {
            if (!charactersStream.match_string("("))
            {
                debug_assert(false);
                return nullptr;
            }
            SyntreeNode* currNode = parse_expression(charactersStream);
            if (!charactersStream.match_string(")"))
            {
                debug_assert(false);
                return nullptr;
            }
            debug_assert(currNode);
            return currNode;
        }

        SyntreeNode* parse_identifier_expression(characters_stream& charactersStream)
        {
            SyntreeNode* expression_node = nullptr;

            unique_string name;
            if (parse_identifier(charactersStream, name))
            {
                expression_node = new_syntree_node(SyntreeNode_Identifier);
                expression_node->mIdentifier = name;
            }
            debug_assert(expression_node);
            return expression_node;
        }

        bool parse_identifier(characters_stream& charactersStream, unique_string& name)
        {
            const char* nameStart = charactersStream.mCursor;
            const char* nameEnd = nameStart;
            for (; ; )
            {
                char currChar = *nameEnd;
                if ((currChar >= 'a' && currChar <= 'z') || (currChar >= 'A' && currChar <= 'Z') ||
                    (currChar >= '0' && currChar <= '9') || 
                    (currChar == '_') || (currChar == '#') || (currChar == '$'))
                {
                    ++nameEnd;
                    continue;
                }
                break;
            }
            if (nameStart == nameEnd)
                return false;

            charactersStream.mCursor = nameEnd;
            name.assign(nameStart, nameEnd);
            return true;
        }

        SyntreeNode* new_syntree_node(SyntreeNodeID nodeid)
        {
            mNodesBuffer.emplace_back(nodeid);
            return &mNodesBuffer.back();
        }

        // syntax tree traverse

        template<typename TFunc>
        bool evaluate_node(SyntreeNode* treeNode, TFunc func) const
        {
            debug_assert(treeNode);
            switch (treeNode->mNodeID)
            {
                case SyntreeNode_NOT:
                    return !evaluate_node(treeNode->mExpressionL, func);
                case SyntreeNode_OR:
                    return evaluate_node(treeNode->mExpressionL, func) || evaluate_node(treeNode->mExpressionR, func);
                case SyntreeNode_AND:
                    return evaluate_node(treeNode->mExpressionL, func) && evaluate_node(treeNode->mExpressionR, func);
                case SyntreeNode_Identifier:
                    return func(treeNode->mIdentifier);
            }
            debug_assert(false);
            return false;
        }

    private:
        std::list<SyntreeNode> mNodesBuffer;

        SyntreeNode* mExpressionRoot = nullptr;
        std::string mExpressionSource;
    };

} // namespace cxx