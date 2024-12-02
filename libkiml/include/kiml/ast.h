#ifndef __AST_H__
#define __AST_H__

#include "kiml.h"
#include "utils.h"
#include <vector>

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
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

#include "object.h"

class AstExpression
{
protected:
	EXPRESSIONTYPES exprtype;
	YYLTYPE location;
	KIMLTYPES datatype;
	Object *constval;

public:
	AstExpression(EXPRESSIONTYPES exprtype, const YYLTYPE &location)
		: exprtype(exprtype), location(location), datatype(KIML_NOTYPE), constval(nullptr) { }

	~AstExpression(void)
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
	AstExpressionList(void) { }
	AstExpressionList(const YYLTYPE &location)
		: location(location) { }

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
	std::vector<KIMLTYPES> types;

public:
	AstParamList(void) { }
	AstParamList(const YYLTYPE &location)
		: location(location) { }

	void add(KIMLTYPES paramType)
	{
		types.push_back(paramType);
	}

	const std::vector<KIMLTYPES> & getTypes(void) const { return types; }
};

class AstIdentList
{
protected:
	YYLTYPE location;
	std::vector<KIMLCSTRING> idents;

public:
	AstIdentList(void) { }
	AstIdentList(const YYLTYPE &location)
		: location(location) { }

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
	BINOP_POW,
	BINOP_CAT,
	BINOP_AND,
	BINOP_OR,
	BINOP_XOR,
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

public:
	AstIdent(const YYLTYPE &location, KIMLCSTRING ident)
		: AstExpression(EXPR_IDENT, location), ident(ident) { }

	~AstIdent(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getIdent(void) const { return ident; }
};

class AstConstTrue : public AstExpression
{
public:
	AstConstTrue(const YYLTYPE &location)
		: AstExpression(EXPR_TRUE, location) { AstExpression::datatype = KIML_BOOL; }
};

class AstConstFalse : public AstExpression
{
public:
	AstConstFalse(const YYLTYPE &location)
		: AstExpression(EXPR_FALSE, location) { AstExpression::datatype = KIML_BOOL; }
};

class AstConstNull : public AstExpression
{
public:
	AstConstNull(const YYLTYPE &location)
		: AstExpression(EXPR_NULL, location) { AstExpression::datatype = KIML_OBJECT; }
};

class AstConstInt : public AstExpression
{
protected:
	KIMLINT value;

public:
	AstConstInt(const YYLTYPE &location, KIMLINT value)
		: AstExpression(EXPR_INT, location), value(value) { AstExpression::datatype = KIML_INT; }

	inline KIMLINT getValue(void) const { return value; }
};

class AstConstReal : public AstExpression
{
protected:
	KIMLFLOAT value;

public:
	AstConstReal(const YYLTYPE &location, KIMLFLOAT value)
		: AstExpression(EXPR_REAL, location), value(value) { AstExpression::datatype = KIML_REAL; }

	inline KIMLFLOAT getValue(void) const { return value; }
};

class AstConstString : public AstExpression
{
protected:
	KIMLCSTRING value;

public:
	AstConstString(const YYLTYPE &location, KIMLCSTRING value)
		: AstExpression(EXPR_STRING, location), value(value) { AstExpression::datatype = KIML_STRING; }

	~AstConstString(void)
	{
		delete []value;
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
		: AstExpression(EXPR_FUNC, location), ident(ident), exprlist(exprlist) { }

	~AstExprFunc(void)
	{
		delete []ident;
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
		: AstExpression(EXPR_CAST, location), casttype(casttype), expr(expr) { AstExpression::datatype = casttype; }

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
		: AstExpression(EXPR_BIN, location), op(op), lhs(lhs), rhs(rhs) { }

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
		: AstExpression(EXPR_UN, location), op(op), expr(expr) { }

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
	EMPTYLINE,
	IOIN,
	IOOUT,
	VARDECL,
	VARSET,
	VARDEL,
	STACKPUSH,
	STACKPOP,
	STACKPEEK,
	STACKSWAP,
	STACKCLEAR,
	STACKREAD,
	STACKWRITE,
	TAPEREAD,
	TAPEWRITE,
	TAPENEXT,
	TAPEPREV,
	TAPEMOVE,
	CTRLGOTO,
	CTRLCALL,
	CTRLRETURN,
	CTRLEND,
	FUNCDECL,
	FUNCCALL,
	EVENTHANDLE,
	EVENTUNHANDLE,
	EVENTRAISE,
	LABELDECL,
	CONSTDEF,
	ENUMDEF,
};

//=========================================================

class AstStatement;
class AstStatementList;
class AstExpression;
class AstExpressionList;
class AstParamList;

//=========================================================

class AstProgram
{
protected:
	AstStatementList *statements;

public:
	void setStatements(AstStatementList *statements) { this->statements = statements; }
	AstStatementList * getStatements(void) const { return statements; }
};

class AstStatement
{
protected:
	STATEMENTTYPES stmttype;
	YYLTYPE location;

public:
	AstStatement(STATEMENTTYPES stmttype, const YYLTYPE &location)
		: stmttype(stmttype), location(location) { }

	inline STATEMENTTYPES getType(void) const { return stmttype; }
	inline const YYLTYPE & getLocation(void) const { return location; }
};

class AstStatementList
{
protected:
	YYLTYPE location;
	std::vector<AstStatement*> statements;

public:
	AstStatementList(void) { }
	AstStatementList(const YYLTYPE &location)
		: location(location) { }

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

//=========================================================

class AstIoIn : public AstStatement
{
protected:
	KIMLCSTRING ident;
	KIMLTYPES type;

public:
	AstIoIn(const YYLTYPE &location, KIMLCSTRING ident)
		: AstStatement(IOIN, location), ident(ident), type(KIML_NOTYPE) { }
	AstIoIn(const YYLTYPE &location, KIMLTYPES type)
		: AstStatement(IOIN, location), ident(nullptr), type(type) { }

	~AstIoIn(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getInputIdent(void) const { return ident; }
	inline KIMLTYPES getInputType(void) const { return type; }
};

class AstIoOut : public AstStatement
{
protected:
	AstExpression *expr;

public:
	AstIoOut(const YYLTYPE &location, AstExpression *expr)
		: AstStatement(IOOUT, location), expr(expr) { }

	~AstIoOut(void)
	{
		delete expr;
	}

	inline AstExpression * getValue(void) const { return expr; }
};

class AstVarDecl : public AstStatement
{
protected:
	KIMLTYPES type;
	KIMLCSTRING ident;
	AstExpression *expr;

public:
	AstVarDecl(const YYLTYPE &location, KIMLTYPES type, KIMLCSTRING ident, AstExpression *expr = nullptr)
		: AstStatement(VARDECL, location), type(type), ident(ident), expr(expr) { }

	~AstVarDecl(void)
	{
		delete []ident;
		delete expr;
	}

	inline KIMLTYPES getVarType(void) const { return type; }
	inline KIMLCSTRING getVarName(void) const { return ident; }
	inline AstExpression * getDefaultValue(void) const { return expr; }
};

class AstVarSet : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpression *expr;
	
public:
	AstVarSet(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *expr)
		: AstStatement(VARSET, location), ident(ident), expr(expr) { }

	~AstVarSet(void)
	{
		delete []ident;
		delete expr;
	}

	inline KIMLCSTRING getVarName(void) const { return ident; }
	inline AstExpression * getValue(void) const { return expr; }
};

class AstVarDel : public AstStatement
{
protected:
	KIMLCSTRING ident;
	
public:
	AstVarDel(const YYLTYPE &location, KIMLCSTRING ident)
		: AstStatement(VARDEL, location), ident(ident) { }

	~AstVarDel(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getVarName(void) const { return ident; }
};

class AstStackPush : public AstStatement
{
protected:
	AstExpression *expr;

public:
	AstStackPush(const YYLTYPE &location, AstExpression *expr)
		: AstStatement(STACKPUSH, location), expr(expr) { }

	~AstStackPush(void)
	{
		delete expr;
	}

	inline AstExpression * getValue(void) const { return expr; }
};

class AstStackPeek : public AstStatement
{
protected:
	KIMLCSTRING ident;

public:
	AstStackPeek(const YYLTYPE &location, KIMLCSTRING ident)
		: AstStatement(STACKPEEK, location), ident(ident) { }

	~AstStackPeek(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getVarName(void) const { return ident; }
};

class AstStackPop : public AstStatement
{
protected:
	KIMLCSTRING ident;

public:
	AstStackPop(const YYLTYPE &location, KIMLCSTRING ident = nullptr)
		: AstStatement(STACKPOP, location), ident(ident) { }

	~AstStackPop(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getVarName(void) const { return ident; }
};

class AstStackSwap : public AstStatement
{
public:
	AstStackSwap(const YYLTYPE &location)
		: AstStatement(STACKSWAP, location) { }
};

class AstStackClear : public AstStatement
{
protected:
	KIMLINT count;

public:
	AstStackClear(const YYLTYPE &location, KIMLINT count = -1)
		: AstStatement(STACKCLEAR, location), count(count) { }

	inline KIMLINT getCount(void) const { return count; }
};

class AstStackRead : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpression *offset;

public:
	AstStackRead(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *offset)
		: AstStatement(STACKREAD, location), ident(ident), offset(offset) { }

	~AstStackRead(void)
	{
		delete []ident;
		delete offset;
	}

	inline KIMLCSTRING getVarName(void) const { return ident; }

	inline AstExpression * getOffset(void) const { return offset; }
};

class AstStackWrite : public AstStatement
{
protected:
	AstExpression *expr;
	AstExpression *offset;

public:
	AstStackWrite(const YYLTYPE &location, AstExpression *expr, AstExpression *offset)
		: AstStatement(STACKWRITE, location), expr(expr), offset(offset) { }

	~AstStackWrite(void)
	{
		delete expr;
		delete offset;
	}

	inline AstExpression * getValue(void) const { return expr; }

	inline AstExpression * getOffset(void) const { return offset; }
};

class AstTapeRead : public AstStatement
{
protected:
	KIMLCSTRING ident;
	KIMLTYPES type;
	AstExpression *expr;

public:
	AstTapeRead(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *expr = nullptr)
		: AstStatement(TAPEREAD, location), ident(ident), type(KIML_NOTYPE), expr(expr) { }
	AstTapeRead(const YYLTYPE &location, KIMLTYPES type, AstExpression *expr = nullptr)
		: AstStatement(TAPEREAD, location), ident(nullptr), type(type), expr(expr) { }

	~AstTapeRead(void)
	{
		delete []ident;
		delete expr;
	}

	inline KIMLCSTRING getDestVar(void) const { return ident; }
	inline KIMLTYPES getDestType(void) const { return type; }
	inline AstExpression * getIndex(void) const { return expr; }
};

class AstTapeWrite : public AstStatement
{
protected:
	AstExpression *expr;
	AstExpression *idxexpr;
	//KIMLINT index;

public:
	AstTapeWrite(const YYLTYPE &location, AstExpression *expr, AstExpression *idxexpr = nullptr)
		: AstStatement(TAPEWRITE, location), expr(expr), idxexpr(idxexpr) { }

	~AstTapeWrite(void)
	{
		delete expr;
		delete idxexpr;
	}

	inline AstExpression * getValue(void) const { return expr; }
	inline AstExpression * getIndex(void) const { return idxexpr; }
};

class AstTapeNext : public AstStatement
{
public:
	AstTapeNext(const YYLTYPE &location)
		: AstStatement(TAPENEXT, location) { }
};

class AstTapePrev : public AstStatement
{
public:
	AstTapePrev(const YYLTYPE &location)
		: AstStatement(TAPEPREV, location) { }
};

class AstTapeMove : public AstStatement
{
protected:
	AstExpression *expr;

public:
	AstTapeMove(const YYLTYPE &location, AstExpression *expr)
		: AstStatement(TAPEWRITE, location), expr(expr) { }

	~AstTapeMove(void)
	{
		delete expr;
	}

	inline AstExpression * getValue(void) const { return expr; }
};

class AstCtrlGoto : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpression *expr;

public:
	AstCtrlGoto(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *expr = nullptr)
		: AstStatement(CTRLGOTO, location), ident(ident), expr(expr) { }

	~AstCtrlGoto(void)
	{
		delete []ident;
		delete expr;
	}

	inline KIMLCSTRING getLabel(void) const { return ident; }
	inline AstExpression * getCondition(void) const { return expr; }
};

class AstCtrlCall : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpression *expr;

public:
	AstCtrlCall(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *expr = nullptr)
		: AstStatement(CTRLCALL, location), ident(ident), expr(expr) { }

	~AstCtrlCall(void)
	{
		delete []ident;
		delete expr;
	}

	inline KIMLCSTRING getLabel(void) const { return ident; }
	inline AstExpression * getCondition(void) const { return expr; }
};

class AstCtrlReturn : public AstStatement
{
public:
	AstCtrlReturn(const YYLTYPE &location)
		: AstStatement(CTRLRETURN, location) { }
};

class AstCtrlEnd : public AstStatement
{
public:
	AstCtrlEnd(const YYLTYPE &location)
		: AstStatement(CTRLEND, location) { }
};

class AstFuncDecl : public AstStatement
{
protected:
	KIMLCSTRING ident;
	KIMLTYPES rettype;
	AstParamList *params;
	KIMLCSTRING label;

public:
	AstFuncDecl(const YYLTYPE &location, KIMLCSTRING ident, KIMLTYPES rettype, AstParamList *params, KIMLCSTRING label = nullptr)
		: AstStatement(FUNCDECL, location), ident(ident), rettype(rettype), params(params), label(label) { }

	~AstFuncDecl(void)
	{
		delete []ident;
		delete params;
		if (label)
			delete []label;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
	inline KIMLTYPES getReturnType(void) const { return rettype; }
	inline AstParamList * getParams(void) const { return params; }
	inline KIMLCSTRING getLabel(void) const { return label; }
};

class AstFuncCall : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpressionList *exprlist;

public:
	AstFuncCall(const YYLTYPE &location, KIMLCSTRING ident, AstExpressionList *exprlist)
		: AstStatement(FUNCCALL, location), ident(ident), exprlist(exprlist) { }

	~AstFuncCall(void)
	{
		delete []ident;
		delete exprlist;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
	inline AstExpressionList * getArgs(void) const { return exprlist; }
};

class AstEventHandle : public AstStatement
{
protected:
	KIMLCSTRING ident;
	KIMLCSTRING label;

public:
	AstEventHandle(const YYLTYPE &location, KIMLCSTRING ident, KIMLCSTRING label)
		: AstStatement(EVENTHANDLE, location), ident(ident), label(label) { }

	~AstEventHandle(void)
	{
		delete []ident;
		delete []label;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
	inline KIMLCSTRING getLabel(void) const { return label; }
};

class AstEventUnhandle : public AstStatement
{
protected:
	KIMLCSTRING ident;
	KIMLCSTRING label;

public:
	AstEventUnhandle(const YYLTYPE &location, KIMLCSTRING ident, KIMLCSTRING label)
		: AstStatement(EVENTUNHANDLE, location), ident(ident), label(label) { }

	~AstEventUnhandle(void)
	{
		delete []ident;
		delete []label;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
	inline KIMLCSTRING getLabel(void) const { return label; }
};

class AstEventRaise : public AstStatement
{
protected:
	KIMLCSTRING ident;

public:
	AstEventRaise(const YYLTYPE &location, KIMLCSTRING ident)
		: AstStatement(EVENTRAISE, location), ident(ident) { }

	~AstEventRaise(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
};

class AstLabel : public AstStatement
{
protected:
	KIMLCSTRING ident;

public:
	AstLabel(const YYLTYPE &location, KIMLCSTRING ident)
		: AstStatement(LABELDECL, location), ident(ident) { }

	~AstLabel(void)
	{
		delete []ident;
	}

	inline KIMLCSTRING getName(void) const { return ident; }
};

class AstConstDef : public AstStatement
{
protected:
	KIMLCSTRING ident;
	AstExpression *expr;

public:
	AstConstDef(const YYLTYPE &location, KIMLCSTRING ident, AstExpression *expr)
		: AstStatement(CONSTDEF, location), ident(ident), expr(expr) { }

	~AstConstDef(void)
	{
		delete []ident;
		delete expr;
	}

	KIMLCSTRING getName(void) const { return ident; }
	AstExpression * getValue(void) { return expr; }
};

class AstEnumDef : public AstStatement
{
protected:
	std::vector<KIMLCSTRING> idents;

public:
	AstEnumDef(const YYLTYPE &location, AstIdentList *list)
		: AstStatement(ENUMDEF, location)
	{
		idents.swap(list->getIdents());
		delete list;
	}

	~AstEnumDef(void)
	{
		for (auto it = idents.begin(); it != idents.end(); ++it)
			delete []*it;
	}

	const std::vector<KIMLCSTRING> & getIdents(void) const { return idents; }
};

#endif//__AST_H__
