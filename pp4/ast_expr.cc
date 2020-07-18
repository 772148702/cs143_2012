/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "include/ast_expr.h"
#include "include/ast_type.h"
#include "include/ast_decl.h"



Decl* Expr::GetFieldDecl(Identifier *field, Expr *b) {

    if (b != NULL)
        return GetFieldDecl(field, b->GetType());

    Decl *d = GetFieldDecl(field, static_cast<Node*>(this));

    if (d == NULL) {
        ClassDecl *classDecl = GetClassDecl();
        if (classDecl != NULL)
       d = GetFieldDecl(field, classDecl->GetType());
    }

    return d;
}




Decl* Expr::GetFieldDecl(Identifier *f, Type *b) {
    NamedType *t = dynamic_cast<NamedType*>(b);

    while (t != NULL) {
        Decl *d = Program::gScope->table->Lookup(t->Name());
        ClassDecl *c = dynamic_cast<ClassDecl*>(d);
        InterfaceDecl *i = dynamic_cast<InterfaceDecl*>(d);

        Decl *fieldDecl;
        if (c != NULL) {
            if ((fieldDecl = GetFieldDecl(f, c->GetNode())) != NULL)
                return fieldDecl;
            else
                t = c->GetExtends();
        } else if (i != NULL) {
            if ((fieldDecl = GetFieldDecl(f, i->GetNode())) != NULL)
                return fieldDecl;
            else
                t = NULL;
        } else {
            t = NULL;
        }
    }

    return GetFieldDecl(f, this->GetNode());
}



Location *Expr::GetThisLoc() {
    return new Location(fpRelative, CodeGenerator::OffsetToFirstParam, "this");
}

Decl *Expr::GetFieldDecl(Identifier *field, Node *n) {
    while (n != NULL) {
        Decl *d = n->GetScope()->table->Lookup(field->GetName());
        if (d != NULL)
            return d;
        n = n->GetParent();
    }
    return NULL;
}

ClassDecl *Expr::GetClassDecl() {
    Node *n = this;
    while (n != nullptr) {
        if (dynamic_cast<ClassDecl*>(n))
            return static_cast<ClassDecl*>(n);
        n = n->GetParent();
    }
    return nullptr;
}


IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
Type* IntConstant::GetType() {
    return Type::intType;
}

Location *IntConstant::Emit(CodeGenerator *cg) {
    return cg->GenLoadConstant(value);
}

int IntConstant::GetMemBytes() {
    return 4;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

Type* DoubleConstant::GetType() {
    return Type::doubleType;
}

Location *DoubleConstant::Emit(CodeGenerator *cg) {
    Assert(0);
    return nullptr;
}

int DoubleConstant::GetMemBytes() {
    return 4;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

Type* BoolConstant::GetType() {
    return Type::boolType;
}

Location *BoolConstant::Emit(CodeGenerator *cg) {
    return cg->GenLoadConstant(value?1:0);
}

int BoolConstant::GetMemBytes() {
    return 4;
}


StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Type* StringConstant::GetType() {
    return Type::stringType;
}

Location *StringConstant::Emit(CodeGenerator *cg) {
    return cg->GenLoadConstant(value);
}

int StringConstant::GetMemBytes() {
    return 4;
}

Type* NullConstant::GetType() {
    return Type::nullType;
}

Location *NullConstant::Emit(CodeGenerator *cg) {
    return cg->GenLoadConstant(0);
}

int NullConstant::GetMemBytes() {
    return 4;
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}



void CompoundExpr::BuildScope() {


    if(left!=NULL)
        left->BuildScope();
    right->BuildScope();
}

void CompoundExpr::Check() {
    if(left!=NULL)
        left->Check();
    right->Check();
}

Type* ArithmeticExpr::GetType() {
    Type* rtype = right->GetType();
    if(left==NULL) {
            if(rtype->IsEquivalentTo(Type::intType)||
                rtype->IsEquivalentTo(Type::doubleType))
                return rtype;
            else
                return Type::errorType;
    }

    Type *ltype = left->GetType();

    if(ltype->IsEquivalentTo(Type::intType)&&
        rtype->IsEquivalentTo(Type::intType))
        return ltype;
    if(ltype->IsEquivalentTo(Type::doubleType)&&
       rtype->IsEquivalentTo(Type::doubleType))
        return ltype;
    return Type::errorType;

}

void ArithmeticExpr::Check() {
    if(left!=NULL)
        left->Check();
    right->Check();
    Type* rtype = right->GetType();
    if(left==NULL) {
        if(rtype->IsEquivalentTo(Type::intType)||
           rtype->IsEquivalentTo(Type::doubleType))
            return;
        else
            ReportError::IncompatibleOperand(op,rtype);
        return;
    }
    Type *ltype= left->GetType();
    if(ltype->IsEquivalentTo(Type::intType)&&
        rtype->IsEquivalentTo(Type::intType))
        return;

    if(ltype->IsEquivalentTo(Type::doubleType)&&
       rtype->IsEquivalentTo(Type::doubleType))
        return ;

    ReportError::IncompatibleOperands(op,ltype,rtype);
}

Location *ArithmeticExpr::Emit(CodeGenerator *cg) {
    if(left==nullptr)
        return EmitUnary(cg);
    else
        return EmitBinary(cg);
}

int ArithmeticExpr::GetMemBytes() {
    if(left== nullptr)
        return GetMemBytesUnary();
    else
        return GetMemBytesBinary();
}

Location *ArithmeticExpr::EmitUnary(CodeGenerator *cg) {
    Location *rtemp = right->Emit(cg);

    Location *zero = cg->GenLoadConstant(0);
    return cg->GenBinaryOp(op->GetTokenString(),zero,rtemp);
}

int ArithmeticExpr::GetMemBytesUnary() {
    return right->GetMemBytes()+2*CodeGenerator::VarSize;
}

Location *ArithmeticExpr::EmitBinary(CodeGenerator *cg) {
    Location *ltemp = left->Emit(cg);
    Location *rtemp = right->Emit(cg);

    return cg->GenBinaryOp(op->GetTokenString(),ltemp,rtemp);
}

int ArithmeticExpr::GetMemBytesBinary() {
   return right->GetMemBytes()+left->GetMemBytes()+CodeGenerator::VarSize;
}

Type* RelationalExpr::GetType() {
    Type *rtype= right->GetType();
    Type *ltype = left->GetType();

    if(ltype->IsEquivalentTo(Type::intType)&&
    rtype->IsEquivalentTo(Type::intType))
        return Type::errorType;
    if(ltype->IsEquivalentTo(Type::doubleType)&&
    rtype->IsEquivalentTo(Type::doubleType))
        return Type::errorType;
    return Type::boolType;
}

void RelationalExpr::Check() {
    Type *rtype = right->GetType();
    Type *ltype = left->GetType();
    if(ltype->IsEquivalentTo(Type::intType)&&
    rtype->IsEquivalentTo(Type::intType))
        return ;
    if(ltype->IsEquivalentTo(Type::doubleType)&&
    rtype->IsEquivalentTo(Type::doubleType))
        return ;

    ReportError::IncompatibleOperands(op,ltype,rtype);
}

Location *RelationalExpr::Emit(CodeGenerator *cg) {
    const char *tok = op->GetTokenString();

    if(strcmp("<",tok)==0)
        return EmitLess(cg,left,right);
    else if(strcmp("<=",tok)==0)
        return EmitLessEqual(cg,left,right);
    else if(strcmp(">",tok)==0)
        return EmitLess(cg,right,left);
    else if(strcmp(">=",tok)==0)
        return EmitLessEqual(cg,right,left);
    else
        Assert(0);
    return nullptr;
}

int RelationalExpr::GetMemBytes() {
    const char *tok = op->GetTokenString();

    if(strcmp("<",tok)==0)
        return GetMemBytesLess(left,right);
    else if(strcmp("<=",tok)==0)
        return GetMemBytesLessEqual(left,right);
    else if(strcmp(">",tok)==0)
        return GetMemBytesLess(right,left);
    else if(strcmp(">=",tok)==0)
        return GetMemBytesLessEqual(right,left);
    else
        Assert(0);
    return 0;
}

Location *RelationalExpr::EmitLess(CodeGenerator *cg, Expr *l, Expr *r) {
    Location *ltmp = l->Emit(cg);
    Location *rtmp = r->Emit(cg);

    return cg->GenBinaryOp("<",ltmp,rtmp);
}

int RelationalExpr::GetMemBytesLess(Expr *l, Expr *r) {
    return l->GetMemBytes() + r->GetMemBytes() + CodeGenerator::VarSize;
}

Location *RelationalExpr::EmitLessEqual(CodeGenerator *cg, Expr *l, Expr *r) {
    Location *ltmp = l->Emit(cg);
    Location *rtmp = r->Emit(cg);

    Location *less = cg->GenBinaryOp("<", ltmp, rtmp);
    Location *equal = cg->GenBinaryOp("==", ltmp, rtmp);

    return cg->GenBinaryOp("||", less, equal);
}

int RelationalExpr::GetMemBytesLessEqual(Expr *l, Expr *r) {
    return l->GetMemBytes() + r->GetMemBytes() + 3 * CodeGenerator::VarSize;
}

Type* EqualityExpr::GetType() {
    Type *rtype = right->GetType();
    Type *ltype = left->GetType();

    if(!rtype->IsEquivalentTo(ltype)&&
    !ltype->IsEquivalentTo(rtype))
        return Type::errorType;

    return Type::boolType;
}

void EqualityExpr::Check() {
    left->Check();
    right->Check();

    Type *rtype = right->GetType();
    Type *ltype = left->GetType();

    if(!rtype->IsEquivalentTo(ltype)&&
    !ltype->IsEquivalentTo(rtype))
        ReportError::IncompatibleOperands(op,ltype,rtype);

}

Location *EqualityExpr::Emit(CodeGenerator *cg) {
    const char *tok = op->GetTokenString();
    if(strcmp("==",tok)==0)
        return EmitEqual(cg);
    else if (strcmp("!=",tok)==0)
        return EmitNotEqual(cg);
    else
        Assert(0);
    return nullptr;
}

int EqualityExpr::GetMemBytes() {
    const char *tok = op->GetTokenString();

    if(strcmp("==",tok)==0)
        return GetMemBytesEqual();
    else if(strcmp("!=",tok)==0)
        return GetMemBytesNotEqual();
    else
        Assert(0);
    return 0;
}

Location *EqualityExpr::EmitEqual(CodeGenerator *cg) {
    Location *ltmp = left->Emit(cg);
    Location *rtmp = right->Emit(cg);

    if(left->GetType()->IsEquivalentTo(Type::stringType))
        return cg->GenBuiltInCall(StringEqual,ltmp,rtmp);
    else
        return cg->GenBinaryOp("==",ltmp,rtmp);
}

int EqualityExpr::GetMemBytesEqual() {
    return left->GetMemBytes()+right->GetMemBytes()+CodeGenerator::VarSize;
}

Location *EqualityExpr::EmitNotEqual(CodeGenerator *cg) {
    const char* ret_zro = cg->NewLabel();
    const char* ret_one = cg->NewLabel();
    Location *ret = cg->GenTempVar();

    Location *ltmp = left->Emit(cg);
    Location *rtmp = right->Emit(cg);

    Location *equal;
    if(left->GetType()->IsEquivalentTo(Type::stringType))
        equal = cg->GenBuiltInCall(StringEqual,ltmp,rtmp);
    else
        equal = cg->GenBinaryOp("==",ltmp,rtmp);
    cg->GenIfZ(equal, ret_one);
    cg->GenAssign(ret, cg->GenLoadConstant(0));
    cg->GenGoto(ret_zro);
    cg->GenLabel(ret_one);
    cg->GenAssign(ret, cg->GenLoadConstant(1));
    cg->GenLabel(ret_zro);

    return ret;
}

int EqualityExpr::GetMemBytesNotEqual() {
   return left->GetMemBytes()+right->GetMemBytes()+4*CodeGenerator::VarSize;
}

Type* LogicalExpr::GetType() {
    Type* rtype= right->GetType();
    if(left==NULL) {
        if(rtype->IsEquivalentTo(Type::boolType))
            return Type::boolType;
        else
            return Type::errorType;
    }
    Type * ltype = left->GetType();

    if(ltype->IsEquivalentTo(Type::boolType)&&
    rtype->IsEquivalentTo(Type::boolType))
        return Type::boolType;

    return Type::errorType;
}

void LogicalExpr::Check() {
    if(left!=NULL)
        left->Check();
    right->Check();
    Type *rtype = right->GetType();
    if(left==NULL) {
        if(rtype->IsEquivalentTo(Type::boolType))
            return ;
        else
            ReportError::IncompatibleOperand(op,rtype);
        return;
    }
    Type *ltype = left->GetType();

    if(ltype->IsEquivalentTo(Type::boolType)&&
    rtype->IsEquivalentTo(Type::boolType))
        return ;
    ReportError::IncompatibleOperands(op,ltype,rtype);
}

Location *LogicalExpr::Emit(CodeGenerator *cg) {
    const char *tok = op->GetTokenString();

    if (strcmp("&&", tok) == 0)
        return EmitAnd(cg);
    else if (strcmp("||", tok) == 0)
        return EmitOr(cg);
    else if (strcmp("!", tok) == 0)
        return EmitNot(cg);
    else
        Assert(0); // Should never reach this point!

    return 0;
}

int LogicalExpr::GetMemBytes() {
    const char *tok = op->GetTokenString();

    if (strcmp("&&", tok) == 0)
        return GetMemBytesAnd();
    else if (strcmp("||", tok) == 0)
        return GetMemBytesOr();
    else if (strcmp("!", tok) == 0)
        return GetMemBytesNot();
    else
        Assert(0); // Should never reach this point!

    return 0;
}

Location *LogicalExpr::EmitAnd(CodeGenerator *cg) {
    Location *ltmp = left->Emit(cg);
    Location *rtmp = right->Emit(cg);

    return cg->GenBinaryOp("&&", ltmp, rtmp);
}

int LogicalExpr::GetMemBytesAnd() {
    return left->GetMemBytes() + right->GetMemBytes() + CodeGenerator::VarSize;
}

Location *LogicalExpr::EmitOr(CodeGenerator *cg) {
    Location *ltmp = left->Emit(cg);
    Location *rtmp = right->Emit(cg);

    return cg->GenBinaryOp("||", ltmp, rtmp);
}

int LogicalExpr::GetMemBytesOr() {
    return left->GetMemBytes() + right->GetMemBytes() + CodeGenerator::VarSize;
}

Location *LogicalExpr::EmitNot(CodeGenerator *cg) {
    const char* ret_zro = cg->NewLabel();
    const char* ret_one = cg->NewLabel();
    Location *ret = cg->GenTempVar();

    Location *rtmp = right->Emit(cg);

    cg->GenIfZ(rtmp, ret_one);
    cg->GenAssign(ret, cg->GenLoadConstant(0));
    cg->GenGoto(ret_zro);
    cg->GenLabel(ret_one);
    cg->GenAssign(ret, cg->GenLoadConstant(1));
    cg->GenLabel(ret_zro);

    return ret;
}

int LogicalExpr::GetMemBytesNot() {
    return right->GetMemBytes() + 3 * CodeGenerator::VarSize;
}

Type* AssignExpr::GetType() {
    Type *ltype = left->GetType();
    Type *rtype = right->GetType();
    if(!rtype->IsEquivalentTo(ltype))
        return Type::errorType;

    return ltype;
}

void AssignExpr::Check() {
    left->Check();
    right->Check();

    Type *ltype = left->GetType();
    Type *rtype= right->GetType();

    if(!rtype->IsEquivalentTo(ltype)&&!ltype->IsEquivalentTo(Type::errorType))
        ReportError::IncompatibleOperands(op,ltype,rtype);

}

Location *AssignExpr::Emit(CodeGenerator *cg) {
    Location *rtemp = right->Emit(cg);
    LValue *lval = dynamic_cast<LValue*>(left);

    if (lval != NULL)
        return lval->EmitStore(cg, rtemp);

    Location *ltemp = left->Emit(cg);
    cg->GenAssign(ltemp, rtemp);
    return ltemp;

}

int AssignExpr::GetMemBytes() {
    LValue *lval = dynamic_cast<LValue*>(left);
    if (lval != NULL)
        return right->GetMemBytes() + lval->GetMemBytesStore();

    return right->GetMemBytes() + left->GetMemBytes();
}

Type* This::GetType() {
    ClassDecl *d = GetClassDecl();
    Assert(d != NULL);
    return d->GetType();
}

void This::Check() {
    if(GetClassDecl()==NULL)
        ReportError::ThisOutsideClassScope(this);
}

Location *This::Emit(CodeGenerator *cg) {
    return GetThisLoc();
}

int This::GetMemBytes() {
    return 0;
}

void This::BuildScope() {

}


ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

Type* ArrayAccess::GetType() {
    ArrayType *t = dynamic_cast<ArrayType*>(base->GetType());
    if(t==NULL)
        return Type::errorType;
    return t->GetElemType();
}

void ArrayAccess::BuildScope() {
    base->BuildScope();
    subscript->BuildScope();
}

void ArrayAccess::Check() {
    base->Check();
    subscript->Check();

    ArrayType *t = dynamic_cast<ArrayType*>(base->GetType());
    if(t==NULL)
        ReportError::BracketsOnNonArray(base);
    if(!subscript->GetType()->IsEqualTo(Type::intType))
        ReportError::SubscriptNotInteger(subscript);
}
Location* ArrayAccess::Emit(CodeGenerator *cg) {
    return cg->GenLoad(EmitAddr(cg),CodeGenerator::VarSize);
}

int ArrayAccess::GetMemBytes() {
    return GetMemBytesAddr() + CodeGenerator::VarSize;
}

Location* ArrayAccess::EmitStore(CodeGenerator *cg, Location *val) {
    Location *addr = EmitAddr(cg);
    cg->GenStore(addr, val, CodeGenerator::VarSize);
    return cg->GenLoad(addr, CodeGenerator::VarSize);
}

int ArrayAccess::GetMemBytesStore() {
    return GetMemBytesAddr() + CodeGenerator::VarSize;
}

Location* ArrayAccess::EmitAddr(CodeGenerator *cg) {
    Location *b = base->Emit(cg);
    Location *s = subscript->Emit(cg);

    EmitRuntimeSubscriptCheck(cg, b, s);

    Location *con = cg->GenLoadConstant(CodeGenerator::VarSize);

    // Offset in bytes without skipping the array header info
    Location *num = cg->GenBinaryOp("*", s, con);

    // Offset in bytes taking the array header info into account
    Location *off = cg->GenBinaryOp("+", num, con);

    return cg->GenBinaryOp("+", b, off);
}

int ArrayAccess::GetMemBytesAddr() {
    return base->GetMemBytes() + subscript->GetMemBytes() +
           4 * CodeGenerator::VarSize + GetMemBytesRuntimeSubscriptCheck();
}

Location* ArrayAccess::EmitRuntimeSubscriptCheck(CodeGenerator *cg,
                                                 Location *arr,
                                                 Location *sub) {
    Location *zro = cg->GenLoadConstant(0);
    Location *siz = cg->GenLoad(arr);

    Location *lessZro = cg->GenBinaryOp("<", sub, zro);
    Location *gratSiz = cg->GenBinaryOp("<", siz, sub);
    Location *equlSiz = cg->GenBinaryOp("==", siz, sub);
    Location *gratEqulSiz = cg->GenBinaryOp("||", gratSiz, equlSiz);
    Location *gratEqulSizLessZro = cg->GenBinaryOp("||", gratEqulSiz, lessZro);

    const char *passCheck = cg->NewLabel();
    cg->GenIfZ(gratEqulSizLessZro, passCheck);
    cg->GenBuiltInCall(PrintString, cg->GenLoadConstant(err_arr_out_of_bounds));
    cg->GenBuiltInCall(Halt);
    cg->GenLabel(passCheck);

    return NULL;

    return NULL;
}

int ArrayAccess::GetMemBytesRuntimeSubscriptCheck() {
    return 8 * CodeGenerator::VarSize;
}
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}

Type* FieldAccess::GetType() {
    VarDecl *d = GetDecl();
    Assert(d != NULL);
    return d->GetType();
}

void FieldAccess::BuildScope() {

    if(base!=NULL)
        base->BuildScope();
}

void FieldAccess::Check() {
    if(base!=NULL)
        base->Check();

    Decl* d;
    Type* t;
    if(base==NULL) {
        /*
        ClassDecl *c = GetClassDecl();
        if(c==NULL) {
            if((d=GetFieldDecl(field,scope))==NULL) {
                ReportError::IdentifierNotDeclared(field,LookingForVariable);
                return;
            }
        } else {
            t = c->GetType();
            if((d=GetFieldDecl(field,t))==NULL) {
                ReportError::FieldNotFoundInBase(field,t);
                return ;
            }
        }*/

        d = GetDecl();
        if(d== nullptr) {
            ReportError::FieldNotFoundInBase(field,t);
        }
    } else {
        t= base->GetType();
        if((d=GetFieldDecl(field,t))==NULL) {
            ReportError::FieldNotFoundInBase(field,t);
            return;
        }
        else if(GetClassDecl()==NULL) {
            ReportError::InaccessibleField(field,t);
            return;
        }
    }

    if(dynamic_cast<VarDecl*>(d)==NULL) {
        ReportError::IdentifierNotDeclared(field,LookingForVariable);
    }
}

Location *FieldAccess::Emit(CodeGenerator *cg) {
    FieldAccess *baseAccess = dynamic_cast<FieldAccess*>(base);
    VarDecl *fieldDecl = GetDecl();
    Assert(fieldDecl != NULL);

    if (baseAccess == NULL)
        return EmitMemLoc(cg, fieldDecl);

    VarDecl *baseDecl = baseAccess->GetDecl();
    Assert(baseDecl != NULL);
    int fieldOffset = fieldDecl->GetMemOffset();
    return cg->GenLoad(baseDecl->GetMemLoc(), fieldOffset);
}

Location *FieldAccess::EmitStore(CodeGenerator *cg, Location *val) {
    FieldAccess *baseAccess = dynamic_cast<FieldAccess*>(base);
    VarDecl *fieldDecl = GetDecl();
    Assert(fieldDecl != NULL);

    if (baseAccess == NULL) {
        return EmitMemLocStore(cg, val, fieldDecl);
    }

    VarDecl *baseDecl = baseAccess->GetDecl();
    Assert(baseDecl != NULL);
    int fieldOffset = fieldDecl->GetMemOffset();
    Location *ltemp = baseDecl->GetMemLoc();
    cg->GenStore(ltemp, val, fieldOffset);
    return ltemp;
}

int FieldAccess::GetMemBytes() {
    FieldAccess *baseAccess = dynamic_cast<FieldAccess*>(base);
    VarDecl *fieldDecl = GetDecl();
    Assert(fieldDecl != NULL);

    if (baseAccess == NULL)
        return GetMemBytesMemLoc(fieldDecl);

    return CodeGenerator::VarSize;
}

int FieldAccess::GetMemBytesStore() {
    FieldAccess *baseAccess = dynamic_cast<FieldAccess*>(base);
    VarDecl *fieldDecl = GetDecl();
    Assert(fieldDecl != NULL);

    if (baseAccess == NULL)
        return GetMemBytesMemLocStore(fieldDecl);

    return 0;
}

VarDecl *FieldAccess::GetDecl() {
    Decl *d = GetFieldDecl(field, base);
    return dynamic_cast<VarDecl*>(d);
}

int FieldAccess::GetMemBytesMemLoc(VarDecl *fieldDecl) {
    Location *loc = fieldDecl->GetMemLoc();
    if (loc != NULL)
        return 0;
    return CodeGenerator::VarSize;
}

Location *FieldAccess::EmitMemLocStore(CodeGenerator *cg, Location *val, VarDecl *fieldDecl) {
    Location *loc = fieldDecl->GetMemLoc();
    if (loc != NULL) {
        cg->GenAssign(loc, val);
        return loc;
    }

    /* If loc == NULL, it is assumed that the base is implicitly or explicitly
     * the 'this' pointer.
     */
    Location *This = GetThisLoc();
    cg->GenStore(This, val, fieldDecl->GetMemOffset());
    return This;
}

int FieldAccess::GetMemBytesMemLocStore(VarDecl *fieldDecl) {
    return 0;
}

Location *FieldAccess::EmitMemLoc(CodeGenerator *cg, VarDecl *fieldDecl) {
    Location *loc = fieldDecl->GetMemLoc();
    if (loc != nullptr)
        return loc;

    /* If loc == NULL, it is assumed that the base is implicitly or explicitly
     * the 'this' pointer.
     */
    Location *This = GetThisLoc();

    return cg->GenLoad(This, fieldDecl->GetMemOffset());
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}
 
Type* Call::GetType() {
    if (IsArrayLengthCall())
        return Type::intType;

    FnDecl *d = GetDecl();
    Assert(d != NULL);
    return d->GetType();
}


void Call::Check() {
    if(base!=NULL)
        base->Check();

    Decl *d;
    Type *t;

    if(base==NULL) {
        ClassDecl *c = GetClassDecl();
        if(c==NULL) {
            if((d=GetFieldDecl(field,this))==NULL) {
                CheckActuals(d);
                ReportError::IdentifierNotDeclared(field,LookingForFunction);
                return;
            }
        } else {
            t = c->GetType();
            if((d=GetFieldDecl(field,t))==NULL) {
                CheckActuals(d);
                ReportError::IdentifierNotDeclared(field,LookingForFunction);
                return;
            }
        }
    } else {
        FieldAccess* fd;
        if((fd=dynamic_cast<FieldAccess*>(base))!= nullptr) {
            Decl* tempDecl = GetFieldDecl(fd->GetIdentifier(),this->GetNode());
            VarDecl* cd = dynamic_cast<VarDecl*>(tempDecl);
            if(tempDecl!= nullptr) return;
        }

        t = base->GetType();
        if((d=GetFieldDecl(field,t))==NULL) {
            CheckActuals(d);
            if(dynamic_cast<ArrayType*>(t)==NULL||
            strcmp("length",field->Name())!=0)
                ReportError::FieldNotFoundInBase(field,t);
            return;
        }
    }
    CheckActuals(d);
}

void Call::CheckActuals(Decl *d)
{
    for (int i=0,n=actuals->NumElements();i<n;++i)
            actuals->Nth(i)->Check();

    FnDecl* fnDecl = dynamic_cast<FnDecl*>(d);
    if(fnDecl==NULL) return;
    int expectNumVars = fnDecl->GetFormals()->NumElements();
    int acutalNumVars = actuals->NumElements();
    if(expectNumVars!=acutalNumVars) {
        ReportError::NumArgsMismatch(field,expectNumVars,acutalNumVars);
        return;
    }
    for(int i=0;i<expectNumVars;++i) {
        Type* d1 = fnDecl->GetFormals()->Nth(i)->GetType();
        Type* d2 = actuals->Nth(i)->GetType();
        if(!d1->IsEquivalentTo(d2)) {
            ReportError::ArgMismatch(actuals->Nth(i),i,d2,d1);
        }
    }
}

void Call::BuildScope() {


    if (base != NULL)
        base->BuildScope();

    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        actuals->Nth(i)->BuildScope();
}

Location* Call::Emit(CodeGenerator *cg) {
    if (IsArrayLengthCall())
        return EmitArrayLength(cg);

    return EmitLabel(cg);
}

int Call::GetMemBytes() {
    if (IsArrayLengthCall())
        return GetMemBytesArrayLength();

    return GetMemBytesLabel();
}

Location* Call::EmitLabel(CodeGenerator *cg) {
    List<Location*> *params = new List<Location*>;
    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        params->Append(actuals->Nth(i)->Emit(cg));

    int n = params->NumElements();
    for (int i = n-1; i >= 0; --i)
        cg->GenPushParam(params->Nth(i));

    Location *ret;
    if (!IsMethodCall()) {
        FnDecl *d = GetDecl();
        ret = cg->GenLCall(d->GetLabel(), d->HasReturnVal());

        cg->GenPopParams(n * CodeGenerator::VarSize);
    } else {
        Location *b;
        if (base != NULL)
            b = base->Emit(cg);
        else
            b = GetThisLoc();

        cg->GenPushParam(b);
        ret = EmitDynamicDispatch(cg, b);

        cg->GenPopParams((n+1) * CodeGenerator::VarSize);
    }

    return ret;
}

int Call::GetMemBytesLabel() {
    int memBytes = 0;
    for (int i = 0, n = actuals->NumElements(); i < n; ++i)
        memBytes += actuals->Nth(i)->GetMemBytes();

    if (IsMethodCall()) {
        if (base != NULL)
            memBytes += base->GetMemBytes();

        memBytes += GetMemBytesDynamicDispatch();
    }

    if (GetDecl()->HasReturnVal())
        memBytes += CodeGenerator::VarSize;

    return memBytes;
}

Location* Call::EmitArrayLength(CodeGenerator *cg) {
    return cg->GenLoad(base->Emit(cg));
}

int Call::GetMemBytesArrayLength() {
    return base->GetMemBytes() + CodeGenerator::VarSize;
}

Location* Call::EmitDynamicDispatch(CodeGenerator *cg, Location *b) {
    Location *vtable = cg->GenLoad(b);
    int methodOffset = GetDecl()->GetVTblOffset();
    Location *faddr = cg->GenLoad(vtable, methodOffset);
    return cg->GenACall(faddr, GetDecl()->HasReturnVal());
}

int Call::GetMemBytesDynamicDispatch() {
    return 2 * CodeGenerator::VarSize;
}

FnDecl* Call::GetDecl() {
    Decl *d = GetFieldDecl(field, base);
    return dynamic_cast<FnDecl*>(d);
}

bool Call::IsArrayLengthCall() {
    if (base == NULL)
        return false;

    if (dynamic_cast<ArrayType*>(base->GetType()) == NULL)
        return false;

    if (strcmp("length", field->GetName()) != 0)
        return false;

    return true;
}

bool Call::IsMethodCall() {
    if (base != NULL)
        return true;

    ClassDecl *c = GetClassDecl();
    if (c == NULL)
        return false;

    FnDecl *f = dynamic_cast<FnDecl*>(GetFieldDecl(field, c->GetType()));
    if (f == NULL)
        return false;

    return true;
}

int Call::GetMemBytesActuals() {
    int memBytes = 0;
    for(int i = 0, n = actuals->NumElements(); i < n; i++)
        memBytes += actuals->Nth(i)->GetMemBytes();
    return memBytes;
}


NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) {
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}

Type *NewExpr::GetType() {
    Decl *d = Program::gScope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);

    if (c == NULL)
        return Type::errorType;

    return c->GetType();
}

void NewExpr::Check() {
    //class definition must be in gscope
    Decl *d = Program::gScope->table->Lookup(cType->Name());
    ClassDecl *c = dynamic_cast<ClassDecl*>(d);

    if (c == NULL)
        ReportError::IdentifierNotDeclared(cType->GetId(), LookingForClass);
}


NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}

Type *NewArrayExpr::GetType() {
    return new ArrayType(elemType);
}

void NewArrayExpr::BuildScope() {


    size->BuildScope();
}

void NewArrayExpr::Check() {
    size->Check();

    if (!size->GetType()->IsEqualTo(Type::intType))
        ReportError::NewArraySizeNotInteger(size);

    if (elemType->IsPrimitive() && !elemType->IsEquivalentTo(Type::voidType))
        return;

    Decl *d = Program::gScope->table->Lookup(elemType->Name());
    if (dynamic_cast<ClassDecl*>(d) == NULL)
        elemType->ReportNotDeclaredIdentifier(LookingForType);
}

Location* NewExpr::Emit(CodeGenerator *cg) {
    const char *name = cType->GetName();

    Decl *d = Program::gScope->table->Lookup(name);
    Assert(d != NULL);

    Location *s = cg->GenLoadConstant(d->GetMemBytes());
    Location *c = cg->GenLoadConstant(CodeGenerator::VarSize);

    Location *mem = cg->GenBuiltInCall(Alloc, cg->GenBinaryOp("+", c, s));
    cg->GenStore(mem, cg->GenLoadLabel(name));

    return mem;
}

int NewExpr::GetMemBytes() {
    return 5 * CodeGenerator::VarSize;
}


Location* NewArrayExpr::Emit(CodeGenerator *cg) {
    Location *s = size->Emit(cg);
    Location *c = cg->GenLoadConstant(CodeGenerator::VarSize);

    EmitRuntimeSizeCheck(cg, s);

    Location *n = cg->GenBinaryOp("*", s, c);
    Location *mem = cg->GenBuiltInCall(Alloc, cg->GenBinaryOp("+", c, n));
    cg->GenStore(mem, s);

    return mem;
}

int NewArrayExpr::GetMemBytes() {
    return size->GetMemBytes() + 4 * CodeGenerator::VarSize +
           GetMemBytesRuntimeSizeCheck();
}

Location* NewArrayExpr::EmitRuntimeSizeCheck(CodeGenerator *cg, Location *siz) {
    Location *zro = cg->GenLoadConstant(0);

    Location *lessZro = cg->GenBinaryOp("<", siz, zro);
    Location *equlZro = cg->GenBinaryOp("==", siz, zro);
    Location *lessEqulZro = cg->GenBinaryOp("||", lessZro, equlZro);

    const char *passCheck = cg->NewLabel();
    cg->GenIfZ(lessEqulZro, passCheck);
    cg->GenBuiltInCall(PrintString, cg->GenLoadConstant(err_arr_bad_size));
    cg->GenBuiltInCall(Halt);
    cg->GenLabel(passCheck);

    return NULL;
}

int NewArrayExpr::GetMemBytesRuntimeSizeCheck() {
    return 5 * CodeGenerator::VarSize;
}


Type *ReadIntegerExpr::GetType() {
    return Type::intType;
}
Location* ReadIntegerExpr::Emit(CodeGenerator *cg) {
    return cg->GenBuiltInCall(ReadInteger);
}

int ReadIntegerExpr::GetMemBytes() {
    return CodeGenerator::VarSize;
}

Location* ReadLineExpr::Emit(CodeGenerator *cg) {
    return cg->GenBuiltInCall(ReadLine);
}

int ReadLineExpr::GetMemBytes() {
    return CodeGenerator::VarSize;
}



Type *ReadLineExpr::GetType() {
    return Type::stringType;
}
