#include "compiler.h"
#include "simpl.parser.h"

#define safeDelete(p) do { if (p) { delete p; p = nullptr; } } while(false)
#define safeDeleteArray(a) do { if (a) { delete []a; a = nullptr; } } while(false)

#define raiseCompileError(_err, _ln) \
	do { \
		SIMPLCOMPILEERRORINFO e = { _err, currentCompileUnitIndex, _ln }; \
		this->errors.push_back(e); \
		return false; \
	} while (0)

#define raiseCompileErrorNoReturn(_err, _ln) \
	do { \
		SIMPLCOMPILEERRORINFO e = { _err, currentCompileUnitIndex, _ln }; \
		this->errors.push_back(e); \
	} while (0)

// ========== parser & lexer =============
extern FILE *yyin;
extern AstProgram *root;
extern KIMLUINT nSyntaxErrors;
extern KIMLCSTRING szFileName;
extern int yylineno; //simpl.lexer.cpp

struct yy_buffer_state;
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
// =======================================

SimpLCompiler::SimpLCompiler(void)
{
	compiler = kimlCreateCompiler();
	constTable = new ConstTable();
	functionTable = new SimpLFunctionTable();
	currentFunction = nullptr;

	currentScope = nullptr;
	EnterScope();
	rootScope = currentScope;
}

SimpLCompiler::~SimpLCompiler(void)
{
	CleanUpAll();

	kimlFreeCompiler(&compiler);
	safeDelete(constTable);
	safeDelete(functionTable);
}

bool SimpLCompiler::Parse(const char *fileName, const char *fileContent)
{
	auto buffer = yy_scan_string(fileContent);
	int parseResult = yyparse();
	yy_delete_buffer(buffer);

	if (parseResult || nSyntaxErrors)
		return false;

	CompileUnit unit = { fileName, root };
	compileUnits.push_back(unit);

	return true;
}

bool SimpLCompiler::Parse(const char *fileName, FILE *fp)
{
	yyin = fp;
	int parseResult = yyparse();

	if (parseResult || nSyntaxErrors)
		return false;

	CompileUnit unit = { fileName, root };
	compileUnits.push_back(unit);

	return true;
}

bool SimpLCompiler::Compile(bool hasDebugInfo)
{
	this->hasDebugInfo = hasDebugInfo;

	for (unsigned int i = 0, c = compileUnits.size(); i < c; ++i)
	{
		currentCompileUnitIndex = i;
		auto &unit = compileUnits[i];
		if (!ScanDeclarations(unit.root))
			return false;
	}

	Emit(op_fcall, kimlHashName("main"));
	Emit(op_hlt);

	for (unsigned int i = 0, c = compileUnits.size(); i < c; ++i)
	{
		currentCompileUnitIndex = i;
		auto &unit = compileUnits[i];
		if (!Emit(unit.root))
			return false;
	}

	return true;
}

bool SimpLCompiler::Finish(void)
{
	if (kimlBake(compiler) == FALSE)
		return false;
	else
		return true;
}

bool SimpLCompiler::GetByteCode(KIMLBYTE **byteCode, KIMLUINT *byteCodeSize)
{
	kimlGetByteCode(compiler, byteCode, byteCodeSize);
	return true;
}

void SimpLCompiler::CleanUpAll(void)
{
	constTable->CleanUp();
	functionTable->CleanUp();

	for (unsigned int i = 0, c = compileUnits.size(); i < c; ++i)
		delete compileUnits[i].root;
	compileUnits.clear();

	for (auto s = currentScope; s; )
	{
		auto p = s->GetParent();
		delete s;
		s = p;
	}
	currentScope = nullptr;
	rootScope = nullptr;
}

bool SimpLCompiler::ScanDeclarations(AstProgram *root)
{
	SIMPLCOMPILEERRORCODES err;

	auto &stmts = root->getStatements()->getStatements();
	for (auto it = stmts.begin(), itEnd = stmts.end(); it != itEnd; ++it)
	{
		auto stmt = *it;
		KIMLUINT line = stmt->getLocation().first_line;

		switch (stmt->getType())
		{
			case STMT_CONST:
			{
				auto stmtConst = (AstConst *)stmt;
				if (!PerformTypeCheck(stmtConst->getValue()))
					return false;
				if (!FoldExpression(stmtConst->getValue()))
					raiseCompileError(SCE_CONST_NOT_CONST, line);

				err = this->constTable->AddConst(stmtConst->getName(), *stmtConst->getValue()->getConstValue());
				if (err)
					raiseCompileError(err, line);
				break;
			}
			case STMT_FUNC:
			{
				auto stmtFunc = static_cast<AstFunc *>(stmt);
				SimpLFunctionInfo fi = SimpLFunctionInfo(stmtFunc);
				err = this->functionTable->AddFunction(fi.name, fi);
				if (err)
					raiseCompileError(err, line);

				if (stmtFunc->getStatements())
				{
					if (hasDebugInfo)
						Emit(op_trace, stmt->getLocation().first_line);

					AddLabel(fi.name);
					kimlEmitFuncDecl(compiler, fi.name);
				}

				break;
			}
			case STMT_ENUM:
			{
				auto enumdef = static_cast<AstEnum *>(stmt);

				const std::vector<KIMLCSTRING> &idents = enumdef->getIdents();
				KIMLINT idx = 0;
				for (auto it = idents.begin(); it != idents.end(); ++it)
				{
					if (this->currentScope->HasSymbol(*it))
					{
						raiseCompileErrorNoReturn(SCE_DUPLICATE_SYMBOL, line);
						continue;
					}

					Object obj(idx);
					err = this->constTable->AddConst(*it, obj);
					if (err)
					{
						raiseCompileErrorNoReturn(err, line);
						continue;
					}

					++idx;
				}
				break;
			}
			default:
			{
				raiseCompileError(SCE_DECLARE_ONLY, (KIMLUINT)stmt->getLocation().first_line);
			}
		}
	}

	return true;
}

bool SimpLCompiler::Emit(AstProgram *root)
{
	auto &stmts = root->getStatements()->getStatements();
	for (auto it = stmts.begin(), itEnd = stmts.end(); it != itEnd; ++it)
	{
		auto stmt = *it;
		if (!Emit(stmt))
			return false;
	}

	return true;
}

bool SimpLCompiler::Emit(AstStatement *statement)
{
	if (hasDebugInfo)
		Emit(op_trace, statement->getLocation().first_line);

	switch (statement->getType())
	{
		case STMT_FUNC:
			return Emit(static_cast<AstFunc *>(statement));
		case STMT_LIST:
			return Emit(static_cast<AstStatementList *>(statement));
		case STMT_VAR:
			return Emit(static_cast<AstVar *>(statement));
		case STMT_IF:
			return Emit(static_cast<AstIf *>(statement));
		case STMT_WHILE:
			return Emit(static_cast<AstWhile *>(statement));
		case STMT_DO:
			return Emit(static_cast<AstDo *>(statement));
		case STMT_BREAK:
			return Emit(static_cast<AstBreak *>(statement));
		case STMT_CONTINUE:
			return Emit(static_cast<AstContinue *>(statement));
		case STMT_RETURN:
			return Emit(static_cast<AstReturn *>(statement));
		case STMT_EXPR:
			return Emit(static_cast<AstExprStatement *>(statement));
		case STMT_ENUM:
		case STMT_CONST:
			return true;
		default:
			return false;
	}
}

bool SimpLCompiler::Emit(AstFunc *statement)
{
	if (!statement->getStatements())
		return true;

	MarkLabel(statement->getName());

	functionTable->GetFunction(statement->getName(), currentFunction);
	EnterScope();

	auto &params = statement->getParams()->getParams();
	for (auto it = params.begin(), itEnd = params.end(); it != itEnd; ++it)
	{
		auto err = currentScope->AddSymbol(it->second, it->first);
		if (err)
			raiseCompileError(err, (KIMLUINT)statement->getLocation().first_line);
	}

	for (int i = 0, c = (int)params.size(); i < c; ++i)
	{
		int offset = c - i;
		if (offset <= 4)
		{
			OpCodes opcode = op_srd1;
			switch (offset)
			{
				case 1: opcode = op_srd1; break;
				case 2: opcode = op_srd2; break;
				case 3: opcode = op_srd3; break;
				case 4: opcode = op_srd4; break;
			}
			Emit(opcode);
		}
		else
		{
			Emit(op_srd, offset);
		}
		Emit(op_new, kimlHashName(GetScopedName(params[i].second).c_str()));
	}

	auto &stmts = statement->getStatements()->getStatements();
	for (auto it = stmts.begin(), itEnd = stmts.end(); it != itEnd; ++it)
		if (!Emit(*it))
			return false;

	LeaveScope();

	switch (currentFunction->returnType)
	{
		case KIML_BOOL:
			Emit(op_ldfalse);
			break;
		case KIML_ANYTYPE:
		case KIML_INT:
			Emit(op_ldi, 0);
			break;
		case KIML_REAL:
			Emit(op_ldr, 0);
			break;
		case KIML_STRING:
			Emit(op_ldes);
			break;
		case KIML_OBJECT:
			Emit(op_ldnull);
			break;
	}

	Emit(op_twr0);
	Emit(op_ret);

	currentFunction = nullptr;

	return true;
}

bool SimpLCompiler::Emit(AstStatementList *statement)
{
	EnterScope();
	
	auto stmts = statement->getStatements();
	for (auto it = stmts.begin(), itEnd = stmts.end(); it != itEnd; ++it)
		if (!Emit(*it))
			return false;
	
	LeaveScope();

	return true;
}

bool SimpLCompiler::Emit(AstVar *statement)
{
	auto err = currentScope->AddSymbol(statement->getName(), statement->getType());
	if (err)
		raiseCompileError(err, (KIMLUINT)statement->getLocation().first_line);

	if (statement->getValue() != nullptr)
	{
		if (!PerformTypeCheck(statement->getValue()))
			return false;

		if (!Emit(statement->getValue()))
			return false;

		int typeMatchResult = TypeMatchTable[statement->getType()][statement->getValue()->getDataType()];
		if (typeMatchResult == K_TYPE_MISMATCHED)
			raiseCompileError(SCE_TYPE_MISMATCHED, (KIMLUINT)statement->getValue()->getLocation().first_line);

		EmitCast(typeMatchResult, statement->getType());
		Emit(op_new, kimlHashName(GetScopedName(statement->getName()).c_str()));
	}
	else
	{
		switch (statement->getType())
		{
			case KIML_BOOL:
				Emit(op_ldfalse);
				break;
			case KIML_ANYTYPE:
			case KIML_INT:
				Emit(op_ldi, 0);
				break;
			case KIML_REAL:
				Emit(op_ldr, 0);
				break;
			case KIML_STRING:
				Emit(op_ldes);
				break;
			case KIML_OBJECT:
				Emit(op_ldnull);
				break;
		}
		Emit(op_new, kimlHashName(GetScopedName(statement->getName()).c_str()));
	}

	return true;
}

bool SimpLCompiler::Emit(AstIf *statement)
{
	if (!PerformTypeCheck(statement->getCondition()))
		return false;

	std::string Lendif = AllocLabel();
	AddLabel(Lendif);

	Emit(statement->getCondition());
	if (statement->getCondition()->getDataType() != KIML_BOOL)
		Emit(op_cvb);

	if (statement->getFalsePart() != nullptr)
	{
		std::string Lelse = AllocLabel();
		AddLabel(Lelse);

		Emit(op_not);
		EmitJump(op_cjmp, Lelse);

		if (!Emit(statement->getTruePart()))
			return false;

		EmitJump(op_jmp, Lendif);

		MarkLabel(Lelse);
		if (!Emit(statement->getFalsePart()))
			return false;
	}
	else
	{
		Emit(op_not);
		EmitJump(op_cjmp, Lendif);

		if (!Emit(statement->getTruePart()))
			return false;
	}

	MarkLabel(Lendif);

	return true;
}

bool SimpLCompiler::Emit(AstWhile *statement)
{
	if (!PerformTypeCheck(statement->getCondition()))
		return false;

	std::string Lbeginwhile = AllocLabel();
	std::string Lendwhile = AllocLabel();

	AddLabel(Lbeginwhile);
	AddLabel(Lendwhile);
	loopBeginLabels.push(Lbeginwhile);
	loopEndLabels.push(Lendwhile);

	MarkLabel(Lbeginwhile);
	
	Emit(statement->getCondition());
	if (statement->getCondition()->getDataType() != KIML_BOOL)
		Emit(op_cvb);

	Emit(op_not);
	EmitJump(op_cjmp, Lendwhile);

	if (!Emit(statement->getLoopPart()))
		return false;

	EmitJump(op_jmp, Lbeginwhile);
	MarkLabel(Lendwhile);

	loopBeginLabels.pop();
	loopEndLabels.pop();

	return true;
}

bool SimpLCompiler::Emit(AstDo *statement)
{
	if (!PerformTypeCheck(statement->getCondition()))
		return false;

	std::string Lbegindo = AllocLabel();
	std::string Lcheckdo = AllocLabel();
	std::string Lenddo = AllocLabel();

	AddLabel(Lbegindo);
	AddLabel(Lcheckdo);
	AddLabel(Lenddo);

	loopBeginLabels.push(Lcheckdo);
	loopEndLabels.push(Lenddo);

	MarkLabel(Lbegindo);

	if (!Emit(statement->getLoopPart()))
		return false;

	MarkLabel(Lcheckdo);

	Emit(statement->getCondition());
	if (statement->getCondition()->getDataType() != KIML_BOOL)
		Emit(op_cvb);

	EmitJump(op_cjmp, Lbegindo);
	MarkLabel(Lenddo);

	loopBeginLabels.pop();
	loopEndLabels.pop();

	return true;
}

bool SimpLCompiler::Emit(AstBreak *statement)
{
	if (loopEndLabels.empty())
		return false;

	EmitJump(op_jmp, loopEndLabels.top());
	return true;
}

bool SimpLCompiler::Emit(AstContinue *statement)
{
	if (loopBeginLabels.empty())
		return false;

	EmitJump(op_jmp, loopBeginLabels.top());
	return true;
}

bool SimpLCompiler::Emit(AstReturn *statement)
{
	if (statement->getExpression() != nullptr)
	{
		if (!PerformTypeCheck(statement->getExpression()))
			return false;

		auto typeMatchResult = TypeMatchTable[currentFunction->returnType][statement->getExpression()->getDataType()];

		if (typeMatchResult == K_TYPE_MISMATCHED)
			raiseCompileError(SCE_TYPE_MISMATCHED, (KIMLUINT)statement->getLocation().first_line);
		
		Emit(statement->getExpression());
		EmitCast(typeMatchResult, currentFunction->returnType);
	}
	else
	{
		switch (currentFunction->returnType)
		{
			case KIML_BOOL:
				Emit(op_ldfalse);
				break;
			case KIML_ANYTYPE:
			case KIML_INT:
				Emit(op_ldi, 0);
				break;
			case KIML_REAL:
				Emit(op_ldr, 0);
				break;
			case KIML_STRING:
				Emit(op_ldes);
				break;
			case KIML_OBJECT:
				Emit(op_ldnull);
				break;
		}
	}

	Emit(op_twr0);
	Emit(op_ret);
	
	return true;
}

bool SimpLCompiler::Emit(AstExprStatement *statement)
{
	switch (statement->getExpression()->getType())
	{
		case EXPR_BIN:
		{
			if (static_cast<AstBinExpr*>(statement->getExpression())->getOperator() != BINOP_ASN)
				raiseCompileError(SCE_ASSIGN_OR_FUNC_CALL_ONLY, (KIMLUINT)statement->getLocation().first_line);
			break;
		}
		case EXPR_FUNC:
			break;
		default:
			raiseCompileError(SCE_ASSIGN_OR_FUNC_CALL_ONLY, (KIMLUINT)statement->getLocation().first_line);
	}

	if (!PerformTypeCheck(statement->getExpression()))
		return false;

	if (!Emit(statement->getExpression()))
		return false;
	
	Emit(op_twr0);
	return true;
}

bool SimpLCompiler::Emit(AstExpression *expression)
{
	if (this->FoldExpression(expression))
	{
		Object *constval = expression->getConstValue();
		switch (constval->getDataType())
		{
			case KIML_BOOL:
				Emit((constval->getBool() ? op_ldtrue : op_ldfalse));
				break;
			case KIML_INT:
				Emit(op_ldi, constval->getInt());
				break;
			case KIML_REAL:
				Emit(op_ldr, constval->getFloat());
				break;
			case KIML_STRING:
			{
				KIMLCSTRING s = constval->getString();
				if (s[0] == 0)
				{
					Emit(op_ldes);
				}
				else
				{
					Emit(op_lds, s);
				}
				delete[]s;
				break;
			}
			case KIML_OBJECT:
				Emit(op_ldnull); // as the only constval for object type is null
				break;
		}

		return true; //no more things to do
	}

	switch (expression->getType())
	{
		case EXPR_IDENT:
		{
			Object obj;
			if (this->constTable->GetConst(static_cast<AstIdent *>(expression)->getIdent(), obj))
			{
				switch (obj.getDataType())
				{
					case KIML_BOOL:
						Emit(obj.getBool() ? op_ldtrue : op_ldfalse);
						break;
					case KIML_INT:
						Emit(op_ldi, obj.getInt());
						break;
					case KIML_REAL:
						Emit(op_ldr, obj.getFloat());
						break;
					case KIML_STRING:
					{
						KIMLCSTRING s = obj.getString();
						if (s[0] == 0)
						{
							Emit(op_ldes);
						}
						else
						{
							Emit(op_lds, s);
						}
						delete[]s;
						break;
					}
				}
			}
			else
			{
				auto exprIdent = static_cast<AstIdent *>(expression);
				auto name = GetScopedName(exprIdent->getIdent(), exprIdent->getScopeDepth());
				Emit(op_ldvar, kimlHashName(name.c_str()));
			}
		}
		break;

		case EXPR_TRUE:
			Emit(op_ldtrue);
			break;

		case EXPR_FALSE:
			Emit(op_ldfalse);
			break;

		case EXPR_NULL:
			Emit(op_ldnull);
			break;

		case EXPR_INT:
			Emit(op_ldi, static_cast<AstConstInt *>(expression)->getValue());
			break;

		case EXPR_REAL:
			Emit(op_ldr, static_cast<AstConstReal *>(expression)->getValue());
			break;

		case EXPR_STRING:
		{
			auto litstring = static_cast<AstConstString *>(expression);
			if (litstring->getValue()[0] == 0)
			{
				Emit(op_ldes);
			}
			else
			{
				Emit(op_lds, litstring->getValue());
			}
		}
		break;

		case EXPR_BIN:
		{
			auto binexpr = static_cast<AstBinExpr *>(expression);
			if (binexpr->getOperator() == BINOP_ASN)
			{
				int typeMatchResult = TypeMatchTable[binexpr->getLeft()->getDataType()][binexpr->getRight()->getDataType()];
				
				Emit(binexpr->getRight());
				EmitCast(typeMatchResult, binexpr->getDataType());

				auto ident = static_cast<AstIdent *>(binexpr->getLeft());
				auto name = GetScopedName(ident->getIdent(), ident->getScopeDepth());
				Emit(op_stvar, kimlHashName(name.c_str()));
				Emit(op_ldvar, kimlHashName(name.c_str()));
			}
			else
			{
				this->Emit(binexpr->getLeft());
				this->Emit(binexpr->getRight());

				switch (binexpr->getOperator())
				{
					case BINOP_EQU:
						Emit(op_equ);
						break;
					case BINOP_NEQ:
						Emit(op_neq);
						break;
					case BINOP_LES:
						Emit(op_lt);
						break;
					case BINOP_LEQ:
						Emit(op_lte);
						break;
					case BINOP_GRT:
						Emit(op_gt);
						break;
					case BINOP_GEQ:
						Emit(op_gte);
						break;
					case BINOP_ADD:
						Emit(op_add);
						break;
					case BINOP_SUB:
						Emit(op_sub);
						break;
					case BINOP_MUL:
						Emit(op_mul);
						break;
					case BINOP_DIV:
						Emit(op_div);
						break;
					case BINOP_MOD:
						Emit(op_mod);
						break;
					case BINOP_AND:
						Emit(op_and);
						break;
					case BINOP_OR:
						Emit(op_or);
						break;
					case BINOP_XOR:
						Emit(op_xor);
						break;
				}
			}
		}
		break;

		case EXPR_UN:
		{
			auto unexpr = static_cast<AstUnExpr *>(expression);
			this->Emit(unexpr->getExpression());
			switch (unexpr->getOperator())
			{
				case UNOP_NEG:
					Emit(op_neg);
					break;
				case UNOP_NOT:
					Emit(op_not);
					break;
			}
		}
		break;

		case EXPR_FUNC:
		{
			auto exprfunc = static_cast<AstExprFunc *>(expression);
			const SimpLFunctionInfo *fi = nullptr;

			KIMLUINT fidx;
			const SimpLFunctionInfo *builtinFI = SimpLFunctionTable::GetBuiltinFunctionInfo(exprfunc->getName(), fidx);
			if (builtinFI)
				fi = builtinFI;
			else
				this->functionTable->GetFunction(exprfunc->getName(), fi);

			auto args = exprfunc->getArgs()->getExpressions();
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				Emit(arg);
				EmitCast(TypeMatchTable[fi->params[idx].first][arg->getDataType()], fi->params[idx].first);

				if (!builtinFI)
					Emit(op_push);
			}

			if (builtinFI)
			{
				Emit(op_func, fidx);
			}
			else
			{
				Emit(op_fcall, kimlHashName(fi->name));
				Emit(op_trd0);
			}
		}
		break;

		case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			this->Emit(exprcast->getValue());
			switch (exprcast->getCastType())
			{
				case KIML_BOOL:
					Emit(op_cvb);
					break;
				case KIML_INT:
					Emit(op_cvin);
					break;
				case KIML_REAL:
					Emit(op_cvrn);
					break;
				case KIML_STRING:
					Emit(op_cvs);
					break;
				case KIML_OBJECT:
					Emit(op_cvo);
					break;
			}
		}
		break;
	}
	return true;
}

void SimpLCompiler::EmitCast(int typeMatchResult, KIMLTYPES toType)
{
	if (typeMatchResult != K_TYPE_CAST)
		return;

	switch (toType)
	{
		case KIML_BOOL:
			Emit(op_cvb);
			break;
		case KIML_INT:
			Emit(op_cvi);
			break;
		case KIML_REAL:
			Emit(op_cvr);
			break;
		case KIML_STRING:
			Emit(op_cvs);
			break;
		case KIML_OBJECT:
			Emit(op_cvo);
			break;
	}
}

void SimpLCompiler::Emit(OpCodes opcode)
{
	kimlEmit(compiler, opcode);
}

void SimpLCompiler::Emit(OpCodes opcode, KIMLINT value)
{
	kimlEmitInt(compiler, opcode, value);
}

void SimpLCompiler::Emit(OpCodes opcode, KIMLUINT value)
{
	kimlEmitUInt(compiler, opcode, value);
}

void SimpLCompiler::Emit(OpCodes opcode, KIMLFLOAT value)
{
	kimlEmitFloat(compiler, opcode, value);
}

void SimpLCompiler::Emit(OpCodes opcode, KIMLCSTRING value)
{
	kimlEmitString(compiler, opcode, value);
}

void SimpLCompiler::AddLabel(const std::string &label)
{
	kimlAddLabel(compiler, label.c_str());
}

void SimpLCompiler::MarkLabel(const std::string &label)
{
	kimlMarkLabel(compiler, label.c_str());
}

void SimpLCompiler::EmitJump(OpCodes opcode, const std::string &label)
{
	kimlEmitJump(compiler, opcode, label.c_str());
}

bool SimpLCompiler::PerformTypeCheck(AstExpression *expression)
{
	KIMLUINT line = (KIMLUINT)expression->getLocation().first_line;
	SIMPLCOMPILEERRORCODES err;

	switch (expression->getType())
	{
		case EXPR_NONE:
			return true;

		case EXPR_IDENT:
		{
			auto exprident = static_cast<AstIdent *>(expression);
			KIMLTYPES vartype;

			Object obj;
			if (this->constTable->GetConst(exprident->getIdent(), obj))
			{
				vartype = obj.getDataType();
			}
			else
			{
				int depth;
				err = this->currentScope->GetSymbol(exprident->getIdent(), vartype, depth);
				if (err)
					raiseCompileError(err, line);

				exprident->setScopeDepth(depth);
			}

			expression->setDataType(vartype);
		}
		return true;

		case EXPR_TRUE:
		case EXPR_FALSE:
		case EXPR_NULL:
		case EXPR_INT:
		case EXPR_REAL:
		case EXPR_STRING:
			return true;

		case EXPR_BIN:
		{
			auto exprbin = static_cast<AstBinExpr *>(expression);
			if (!this->PerformTypeCheck(exprbin->getLeft()))
				return false;
			if (!this->PerformTypeCheck(exprbin->getRight()))
				return false;

			switch (exprbin->getOperator())
			{
				case BINOP_EQU:
				case BINOP_NEQ:
				{
					expression->setDataType(KIML_BOOL);
					return true;
				}
				case BINOP_LES:
				case BINOP_LEQ:
				case BINOP_GRT:
				case BINOP_GEQ:
				{
					KIMLTYPES t = HigherType(exprbin->getLeft()->getDataType(), exprbin->getRight()->getDataType());
					if (!IsValidArithmetic(t))
						raiseCompileError(SCE_TYPE_MISMATCHED, line);

					expression->setDataType(KIML_BOOL);
					return true;
				}
				case BINOP_ADD:
				{
					KIMLTYPES t = HigherType(exprbin->getLeft()->getDataType(), exprbin->getRight()->getDataType());
					if (t == KIML_STRING)
					{
						expression->setDataType(KIML_STRING);
					}
					else
					{
						if (!IsValidArithmetic(t))
							raiseCompileError(SCE_TYPE_MISMATCHED, line);

						expression->setDataType(t);
					}
					return true;
				}
				case BINOP_SUB:
				case BINOP_MUL:
				case BINOP_DIV:
				case BINOP_MOD:
				{
					KIMLTYPES t = HigherType(exprbin->getLeft()->getDataType(), exprbin->getRight()->getDataType());
					if (!IsValidArithmetic(t))
						raiseCompileError(SCE_TYPE_MISMATCHED, line);

					expression->setDataType(t);
					return true;
				}
				case BINOP_AND:
				case BINOP_OR:
				case BINOP_XOR:
				{
					expression->setDataType(KIML_BOOL);
					return true;
				}
				case BINOP_ASN:
				{
					if (exprbin->getLeft()->getType() != EXPR_IDENT)
						raiseCompileError(SCE_ASSIGN_WRONG_LHS, line);

					int typeMatchResult = TypeMatchTable[exprbin->getLeft()->getDataType()][exprbin->getRight()->getDataType()];
					if (typeMatchResult == K_TYPE_MISMATCHED)
						raiseCompileError(SCE_TYPE_MISMATCHED, line);

					expression->setDataType(exprbin->getLeft()->getDataType());
					return true;
				}
			}
		}
		return true;

		case EXPR_UN:
		{
			auto exprun = static_cast<AstUnExpr *>(expression);
			if (!this->PerformTypeCheck(exprun->getExpression()))
				return false;

			switch (exprun->getOperator())
			{
				case UNOP_NEG:
					if (!IsValidArithmetic(exprun->getExpression()->getDataType()))
						raiseCompileError(SCE_TYPE_MISMATCHED, line);

					expression->setDataType(exprun->getExpression()->getDataType());
					return true;

				case UNOP_NOT:
					expression->setDataType(KIML_BOOL);
					return true;
			}
		}
		return true;

		case EXPR_FUNC:
		{
			auto exprfunc = static_cast<AstExprFunc *>(expression);
			auto args = exprfunc->getArgs()->getExpressions();
			const SimpLFunctionInfo *fi = nullptr;

			KIMLUINT fidx;
			const SimpLFunctionInfo *builtinFI = SimpLFunctionTable::GetBuiltinFunctionInfo(exprfunc->getName(), fidx);
			if (builtinFI)
			{
				fi = builtinFI;
			}
			else
			{
				err = this->functionTable->GetFunction(exprfunc->getName(), fi);
				if (err)
					raiseCompileError(err, line);
			}

			if (fi->params.size() != args.size())
				raiseCompileError(SCE_INCONSISTENT_ARG_COUNT, line);

			bool result = true;
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				if (!this->PerformTypeCheck(arg))
				{
					result = false;
					continue;
				}

				if (TypeMatchTable[fi->params[idx].first][arg->getDataType()] == K_TYPE_MISMATCHED)
				{
					result = false;
					raiseCompileErrorNoReturn(SCE_TYPE_MISMATCHED, line);
				}
			}

			expression->setDataType(fi->returnType);
			return result;
		}

		case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			if (!this->PerformTypeCheck(exprcast->getValue()))
				return false;
		}
		return true;

		default:
			return true;
	}
}

bool SimpLCompiler::FoldExpression(AstExpression *expression)
{
	if (expression->getConstValue())
		return true;

	switch (expression->getType())
	{
		case EXPR_TRUE:
			expression->setConstValue(new Object(true));
			return true;

		case EXPR_FALSE:
			expression->setConstValue(new Object(false));
			return true;

		case EXPR_NULL:
			expression->setConstValue(new Object((KIMLOBJECT)nullptr));
			return true;

		case EXPR_INT:
			expression->setConstValue(new Object(static_cast<AstConstInt *>(expression)->getValue()));
			return true;

		case EXPR_REAL:
			expression->setConstValue(new Object(static_cast<AstConstReal *>(expression)->getValue()));
			return true;

		case EXPR_STRING:
			expression->setConstValue(new Object(static_cast<AstConstString *>(expression)->getValue()));
			return true;

		case EXPR_IDENT:
		{
			auto exprident = static_cast<AstIdent *>(expression);
			Object obj;
			if (this->constTable->GetConst(exprident->getIdent(), obj))
			{
				expression->setConstValue(new Object(std::move(obj)));
				return true;
			}
			else
			{
				return false;
			}
		}

		case EXPR_BIN:
		{
			auto binexpr = static_cast<AstBinExpr *>(expression);
			auto left = binexpr->getLeft();
			auto right = binexpr->getRight();

			if (!this->FoldExpression(left) || !this->FoldExpression(right))
				return false;

			switch (binexpr->getOperator())
			{
				case BINOP_EQU:
					expression->setConstValue(new Object(*left->getConstValue() == *right->getConstValue()));
					return true;
				case BINOP_NEQ:
					expression->setConstValue(new Object(*left->getConstValue() != *right->getConstValue()));
					return true;
				case BINOP_LES:
					expression->setConstValue(new Object(*left->getConstValue() < *right->getConstValue()));
					return true;
				case BINOP_LEQ:
					expression->setConstValue(new Object(*left->getConstValue() <= *right->getConstValue()));
					return true;
				case BINOP_GRT:
					expression->setConstValue(new Object(*left->getConstValue() > *right->getConstValue()));
					return true;
				case BINOP_GEQ:
					expression->setConstValue(new Object(*left->getConstValue() >= *right->getConstValue()));
					return true;
				case BINOP_ADD:
					expression->setConstValue(new Object(*left->getConstValue() + *right->getConstValue()));
					return true;
				case BINOP_SUB:
					expression->setConstValue(new Object(*left->getConstValue() - *right->getConstValue()));
					return true;
				case BINOP_MUL:
					expression->setConstValue(new Object(*left->getConstValue() * *right->getConstValue()));
					return true;
				case BINOP_DIV:
					expression->setConstValue(new Object(*left->getConstValue() / *right->getConstValue()));
					return true;
				case BINOP_MOD:
					expression->setConstValue(new Object(*left->getConstValue() % *right->getConstValue()));
					return true;
				case BINOP_AND:
					expression->setConstValue(new Object(*left->getConstValue() && *right->getConstValue()));
					return true;
				case BINOP_OR:
					expression->setConstValue(new Object(*left->getConstValue() || *right->getConstValue()));
					return true;
				case BINOP_XOR:
					expression->setConstValue(new Object(*left->getConstValue() ^ *right->getConstValue()));
					return true;
				default:
					return false;
			}
		}

		case EXPR_UN:
		{
			auto unexpr = static_cast<AstUnExpr *>(expression);
			auto expr = unexpr->getExpression();

			if (!this->FoldExpression(expr))
				return false;

			switch (unexpr->getOperator())
			{
				case UNOP_NOT:
					expression->setConstValue(new Object(!*expr->getConstValue()));
					return true;
				case UNOP_NEG:
					expression->setConstValue(new Object(-*expr->getConstValue()));
					return true;
				default:
					return false;
			}
		}

		case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			auto expr = exprcast->getValue();

			if (!this->FoldExpression(expr))
				return false;

			switch (exprcast->getDataType())
			{
				case KIML_BOOL:
					expression->setConstValue(new Object(expr->getConstValue()->getBool() == TRUE));
					return true;
				case KIML_INT:
					expression->setConstValue(new Object(expr->getConstValue()->getInt()));
					return true;
				case KIML_REAL:
					expression->setConstValue(new Object(expr->getConstValue()->getFloat()));
					return true;
				case KIML_STRING:
				{
					KIMLCSTRING s = expr->getConstValue()->getString();
					expression->setConstValue(new Object(s));
					delete[]s;
					return true;
				}
			}
		}

		default:
			return false;
	}
}

void SimpLCompiler::EnterScope(void)
{
	auto scope = new Scope(currentScope);
	currentScope = scope;
}

void SimpLCompiler::LeaveScope(void)
{
	auto scope = currentScope;
	currentScope = scope->GetParent();

	auto symbols = scope->GetAllSymbols();
	for (auto it = symbols.begin(), itEnd = symbols.end(); it != itEnd; ++it)
	{
		Emit(op_free, kimlHashName(GetScopedName(it->first.c_str()).c_str()));
	}

	delete scope;
}

std::string SimpLCompiler::GetScopedName(const char *name)
{
	std::ostringstream oss;
	oss << "%" << currentScope->GetDepth() << "%" << name;
	return oss.str();
}

std::string SimpLCompiler::GetScopedName(const char *name, int depth)
{
	std::ostringstream oss;
	oss << "%" << depth << "%" << name;
	return oss.str();
}

std::string SimpLCompiler::AllocLabel(void)
{
	int labelOrdinal = currentScope->IncrementLabelCounter();

	std::ostringstream oss;
	oss << ":" << currentScope->GetDepth() << "^" << currentScope->GetOrdinal() << ":L" << labelOrdinal;
	return oss.str();
}
