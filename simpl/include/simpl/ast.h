#ifndef __SIMPL_AST_H__
#define __SIMPL_AST_H__

#include <kiml/kiml.h>
#include <kiml/utils.h>
#include <vector>

#if !defined YYLTYPE && !defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
} YYLTYPE;
#define yyltype YYLTYPE
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 1
#endif

//=========================================================

enum EXPRESSIONTYPES
{
	EXPR_NONE,
	EXPR_IDENT,
	EXPR_TRUE,
	EXPR_FALSE,
	EXPR_NULL,
	EXPR_INT,
	EXPR_REAL,
	EXPR_STRING,
	EXPR_BIN,
	EXPR_UN,
	EXPR_FUNC,
	EXPR_CAST
};

//=========================================================

#include <kiml/object.h>

class AstExpression
{
protected:
	EXPRESSIONTYPES exprtype;
	YYLTYPE location;
	KIMLTYPES datatype;
	Object *constval;

public:
	AstExpression(EXPRESSIONTYPES exprtype, const YYLTYPE &location)
		: exprtype(exprtype), location(location), datatype(KIML_NOTYPE), constval(nullptr)
	{
	}

	virtual ~AstExpression(void)
	{
		if (this->constval)
			delete this->constval;
	}

	EXPRESSIONTYPES getType(void) const { return this->exprtype; }
	const YYLTYPE & getLocation(void) const { return this->location; }

	KIMLTYPES getDataType(void) const { return this->datatype; }
	void setDataType(KIMLTYPES t) { this->datatype = t; }

	Object * getConstValue(void) { return this->constval; }
	void setConstValue(Object *value)
	{
		if (this->constval)
			delete this->constval;

		this->constval = value;
	}
};

class AstExpressionList
{
protected:
	YYLTYPE location;
	std::vector<AstExpression *> list;

public:
	AstExpressionList(void) {}
	AstExpressionList(const YYLTYPE &location)
		: location(location)
	{
	}

	~AstExpressionList(void)
	{
		for (auto it = list.begin(); it != list.end(); ++it)
			delete *it;
	}

	void add(AstExpression *expression)
	{
		list.push_back(expression);
	}

	const std::vector<AstExpression *> & getExpressions(void) const { return list; }
};

class AstParamList
{
protected:
	YYLTYPE location;
	std::vector<std::pair<KIMLTYPES, KIMLCSTRING>> params;

public:
	AstParamList(void) {}
	AstParamList(const YYLTYPE &location)
		: location(location)
	{
	}

	~AstParamList(void)
	{
		for (auto it = params.begin(), itEnd = params.end(); it != itEnd; ++it)
			if (it->second)delete[]it->second;
		params.clear();
	}

	void add(KIMLTYPES paramType, KIMLCSTRING paramName)
	{
		params.push_back(std::make_pair(paramType, paramName));
	}

	const std::vector<std::pair<KIMLTYPES, KIMLCSTRING>> & getParams(void) const { return params; }
};

class AstIdentList
{
protected:
	YYLTYPE location;
	std::vector<KIMLCSTRING> idents;

public:
	AstIdentList(void) {}
	AstIdentList(const YYLTYPE &location)
		: location(location)
	{
	}

	void add(KIMLCSTRING ident)
	{
		idents.push_back(ident);
	}

	std::vector<KIMLCSTRING> & getIdents(void) { return idents; }
};

//=========================================================

enum BINOPS
{
	BINOP_EQU,
	BINOP_NEQ,
	BINOP_LES,
	BINOP_LEQ,
	BINOP_GRT,
	BINOP_GEQ,
	BINOP_ADD,
	BINOP_SUB,
	BINOP_MUL,
	BINOP_DIV,
	BINOP_MOD,
	BINOP_AND,
	BINOP_OR,
	BINOP_XOR,
	BINOP_ASN,
};

enum UNOPS
{
	UNOP_NOT,
	UNOP_NEG,
};

//=========================================================

class AstIdent : public AstExpression
{
protected:
	KIMLCSTRING ident;
	int scopeDepth;

public:
	AstIdent(const YYLTYPE &location, KIMLCSTRING ident)
		: AstExpression(EXPR_IDENT, location), ident(ident)
	{
	}

	~AstIdent(void)
	{
		delete[]ident;
	}

	inline KIMLCSTRING getIdent(void) const { return ident; }

	inline int getScopeDepth(void)const { return scopeDepth; }
	inline void setScopeDepth(int depth) { scopeDepth = depth; }
};

class AstConstTrue : public AstExpression
{
public:
	AstConstTrue(const YYLTYPE &location)
		: AstExpression(EXPR_TRUE, location)
	{
		AstExpression::datatype = KIML_BOOL;
	}
};

class AstConstFalse : public AstExpression
{
public:
	AstConstFalse(const YYLTYPE &location)
		: AstExpression(EXPR_FALSE, location)
	{
		AstExpression::datatype = KIML_BOOL;
	}
};

class AstConstNull : public AstExpression
{
public:
	AstConstNull(const YYLTYPE &location)
		: AstExpression(EXPR_NULL, location)
	{
		AstExpression::datatype = KIML_OBJECT;
	}
};

class AstConstInt : public AstExpression
{
protected:
	KIMLINT value;

public:
	AstConstInt(const YYLTYPE &location, KIMLINT value)
		: AstExpression(EXPR_INT, location), value(value)
	{
		AstExpression::datatype = KIML_INT;
	}

	inline KIMLINT getValue(void) const { return value; }
};

class AstConstReal : public AstExpression
{
protected:
	KIMLFLOAT value;

public:
	AstConstReal(const YYLTYPE &location, KIMLFLOAT value)
		: AstExpression(EXPR_REAL, location), value(value)
	{
		AstExpression::datatype = KIML_REAL;
	}

	inline KIMLFLOAT getValue(void) const { return value; }
};

class AstConstString : public AstExpression
{
protected:
	KIMLCSTRING value;

public:
	AstConstString(const YYLTYPE &location, KIMLCSTRING value)
		: AstExpression(EXPR_STRING, location), value(value)
	{
		AstExpression::datatype = KIML_STRING;
	}

	~AstConstString(void)
	{
		delete[]value;
	}

	inline KIMLCSTRING getValue(void) const { return value; }
};

class AstExprFunc : public AstExpression
{
protected:
	KIMLCSTRING ident;
	AstExpressionList *exprlist;

public:
	AstExprFunc(const YYLTYPE &location, KIMLCSTRING ident, AstExpressionList *exprlist)
		: AstExpression(EXPR_FUNC, location), ident(ident), exprlist(exprlist)
	{
	}

	~AstExprFunc(void)
	{
		delete[]ident;
		delete exprlist;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
	inline AstExpressionList * getArgs(void) const { return exprlist; }
};

class AstCast : public AstExpression
{
protected:
	KIMLTYPES casttype;
	AstExpression *expr;

public:
	AstCast(const YYLTYPE &location, KIMLTYPES casttype, AstExpression *expr)
		: AstExpression(EXPR_CAST, location), casttype(casttype), expr(expr)
	{
		AstExpression::datatype = casttype;
	}

	~AstCast(void)
	{
		delete expr;
	}

	inline KIMLTYPES getCastType(void) const { return casttype; }
	inline AstExpression * getValue(void) const { return expr; }
};

class AstBinExpr : public AstExpression
{
protected:
	BINOPS op;
	AstExpression *lhs;
	AstExpression *rhs;

public:
	AstBinExpr(const YYLTYPE &location, BINOPS op, AstExpression *lhs, AstExpression *rhs)
		: AstExpression(EXPR_BIN, location), op(op), lhs(lhs), rhs(rhs)
	{
	}

	~AstBinExpr(void)
	{
		delete lhs;
		delete rhs;
	}

	inline BINOPS getOperator(void) const { return op; }
	inline AstExpression * getLeft(void) const { return lhs; }
	inline AstExpression * getRight(void) const { return rhs; }
};

class AstUnExpr : public AstExpression
{
protected:
	UNOPS op;
	AstExpression *expr;

public:
	AstUnExpr(const YYLTYPE &location, UNOPS op, AstExpression *expr)
		: AstExpression(EXPR_UN, location), op(op), expr(expr)
	{
	}

	~AstUnExpr(void)
	{
		delete expr;
	}

	inline UNOPS getOperator(void) const { return op; }
	inline AstExpression * getExpression(void) const { return expr; }
};

//=========================================================

enum STATEMENTTYPES
{
	STMT_EMPTY,
	STMT_LIST,
	STMT_FUNC,
	STMT_IF,
	STMT_WHILE,
	STMT_DO,
	STMT_BREAK,
	STMT_CONTINUE,
	STMT_RETURN,
	STMT_EXPR,
	STMT_VAR,
	STMT_CONST,
	STMT_ENUM,
};

//=========================================================

class AstStatement;
class AstStatementList;
class AstExpression;
class AstExpressionList;
class AstParamList;

//=========================================================

class AstStatement
{
protected:
	STATEMENTTYPES stmttype;
	YYLTYPE location;

public:
	AstStatement(STATEMENTTYPES stmttype, const YYLTYPE &location)
		: stmttype(stmttype), location(location)
	{
	}

	inline STATEMENTTYPES getType(void) const { return stmttype; }
	inline const YYLTYPE & getLocation(void) const { return location; }
};

class AstStatementList : public AstStatement
{
protected:
	std::vector<AstStatement*> statements;

public:
	AstStatementList(const YYLTYPE &location)
		: AstStatement(STMT_LIST, location)
	{
	}

	~AstStatementList(void)
	{
		for (auto it = statements.begin(); it != statements.end(); ++it)
			delete *it;
	}

	void add(AstStatement *statement)
	{
		statements.push_back(statement);
	}

	inline std::vector<AstStatement *> & getStatements(void) { return statements; }
};

class AstProgram
{
protected:
	AstStatementList *statements;

public:
	~AstProgram(void)
	{
		if (statements)
			delete statements;
	}

public:
	void setStatements(AstStatementList *statements) { this->statements = statements; }
	AstStatementList * getStatements(void) const { return statements; }
};

//=========================================================

class AstFunc : public AstStatement
{
protected:
	KIMLTYPES returnType;
	KIMLCSTRING ident;
	AstParamList *params;
	AstStatementList *statements;

public:
	AstFunc(const YYLTYPE &location, KIMLTYPES returnType, KIMLCSTRING ident, AstParamList *params, AstStatementList *statements)
		: AstStatement(STMT_FUNC, location), returnType(returnType), ident(ident), params(params), statements(statements)
	{
	}

	~AstFunc(void)
	{
		delete[]ident;
		delete params;
		if (statements)
			delete statements;
	}

	inline const YYLTYPE & getLocation(void) const { return location; }
	inline KIMLCSTRING getName(void) const { return ident; }
	inline KIMLTYPES getReturnType(void) const { return returnType; }
	inline AstParamList * getParams(void) const { return params; }
	inline AstStatementList * getStatements(void) const { return statements; }
};

class AstIf : public AstStatement
{
protected:
	AstExpression *condition;
	AstStatement *truePart;
	AstStatement *falsePart;

public:
	AstIf(const YYLTYPE &location, AstExpression *condition, AstStatement *truePart, AstStatement *falsePart)
		: AstStatement(STMT_IF, location), condition(condition), truePart(truePart), falsePart(falsePart)
	{
	}

	~AstIf(void)
	{
		delete condition;
		delete truePart;
		if (falsePart)
			delete falsePart;
	}

	inline AstExpression * getCondition(void) const { return condition; }
	inline AstStatement * getTruePart(void) const { return truePart; }
	inline AstStatement * getFalsePart(void) const { return falsePart; }
};

class AstWhile : public AstStatement
{
protected:
	AstExpression *condition;
	AstStatement *loopPart;

public:
	AstWhile(const YYLTYPE &location, AstExpression *condition, AstStatement *loopPart)
		: AstStatement(STMT_WHILE, location), condition(condition), loopPart(loopPart)
	{
	}

	~AstWhile(void)
	{
		delete condition;
		delete loopPart;
	}

	inline AstExpression * getCondition(void) const { return condition; }
	inline AstStatement * getLoopPart(void) const { return loopPart; }
};

class AstDo : public AstStatement
{
protected:
	AstExpression *condition;
	AstStatement *loopPart;

public:
	AstDo(const YYLTYPE &location, AstExpression *condition, AstStatement *loopPart)
		: AstStatement(STMT_DO, location), condition(condition), loopPart(loopPart)
	{
	}

	~AstDo(void)
	{
		delete condition;
		delete loopPart;
	}

	inline AstExpression * getCondition(void) const { return condition; }
	inline AstStatement * getLoopPart(void) const { return loopPart; }
};

class AstBreak : public AstStatement
{
public:
	AstBreak(const YYLTYPE &location)
		: AstStatement(STMT_BREAK, location)
	{
	}
};

class AstContinue : public AstStatement
{
public:
	AstContinue(const YYLTYPE &location)
		: AstStatement(STMT_CONTINUE, location)
	{
	}
};

class AstReturn : public AstStatement
{
protected:
	AstExpression *expr;

public:
	AstReturn(const YYLTYPE &location, AstExpression *expr)
		: AstStatement(STMT_RETURN, location), expr(expr)
	{
	}

	~AstReturn(void)
	{
		if (expr)
			delete expr;
	}

	inline AstExpression * getExpression(void) const { return expr; }
};

class AstExprStatement : public AstStatement
{
protected:
	AstExpression *expr;

public:
	AstExprStatement(const YYLTYPE &location, AstExpression *expr)
		: AstStatement(STMT_EXPR, location), expr(expr)
	{
	}

	~AstExprStatement(void)
	{
		delete expr;
	}

	inline AstExpression * getExpression(void) const { return expr; }
};

class AstVar : public AstStatement
{
protected:
	KIMLTYPES type;
	KIMLCSTRING ident;
	AstExpression *value;

public:
	AstVar(const YYLTYPE &location, KIMLTYPES type, KIMLCSTRING ident, AstExpression *value)
		: AstStatement(STMT_VAR, location), type(type), ident(ident), value(value)
	{
	}

	~AstVar(void)
	{
		delete[]ident;
		if (value)
			delete value;
	}

	inline KIMLTYPES getType(void) const { return type; }
	inline KIMLCSTRING getName(void) const { return ident; }
	inline AstExpression * getValue(void) const { return value; }
};

class AstConst : public AstStatement
{
protected:
	KIMLTYPES type;
	KIMLCSTRING ident;
	AstExpression *value;

public:
	AstConst(const YYLTYPE &location, KIMLTYPES type, KIMLCSTRING ident, AstExpression *value)
		: AstStatement(STMT_CONST, location), type(type), ident(ident), value(value)
	{
	}

	~AstConst(void)
	{
		delete[]ident;
		if (value)
			delete value;
	}

	inline KIMLTYPES getType(void) const { return type; }
	inline KIMLCSTRING getName(void) const { return ident; }
	inline AstExpression * getValue(void) const { return value; }
};

class AstEnum : public AstStatement
{
protected:
	std::vector<KIMLCSTRING> idents;

public:
	AstEnum(const YYLTYPE &location, AstIdentList *list)
		: AstStatement(STMT_ENUM, location)
	{
		idents.swap(list->getIdents());
		delete list;
	}

	~AstEnum(void)
	{
		for (auto it = idents.begin(); it != idents.end(); ++it)
			delete[] * it;
	}

	const std::vector<KIMLCSTRING> & getIdents(void) const { return idents; }
};

#endif//__SIMPL_AST_H__
