/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include  "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
Scope *Program::gScope = new Scope;
stack<const char*> *Program::gBreakLabels = new stack<const char*>;

int Scope::AddDecl(Decl *d) {
    Decl* lookup = table->Lookup(d->GetName());
    if(lookup!=NULL) {
           ReportError::DeclConflict(d,lookup);
           return 1;
    }
    table->Enter(d->GetName(),d);
    return 0;
}

ostream& operator<<(ostream& out,Scope* s) {
    out<<"============== Scope =============="<<std::endl;
    Iterator<Decl*> iter= s->table->GetIterator();
    Decl* d;
    while((d=iter.GetNextValue())!=NULL)
        out<<d<<std::endl;
    return out;
}

Program::Program(List<Decl*> *d):codeGenerator(new CodeGenerator) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
    scope = gScope;
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    BuildScope();
    for (int i=0,n=decls->NumElements();i<n;++i)
    {
        decls->Nth(i)->Check();
    }
}

void Program::BuildScope() {
    for(int i=0, n=decls->NumElements();i<n;++i) {
        gScope->AddDecl(decls->Nth(i));
    }
    for(int i=0, n=decls->NumElements();i<n;++i) {
        decls->Nth(i)->BuildScope();
    }
}

void Program::Emit() {
    int offset = CodeGenerator::OffsetToFirstGlobal;

    for(int i=0,n=decls->NumElements();i<n;++i) {
        VarDecl *d = dynamic_cast<VarDecl*>(decls->Nth(i));
        if(d== nullptr)
            continue;
        Location *loc = new Location(gpRelative,offset,d->GetName());
        d->SetMemLoc(loc);
        offset+=d->GetMemBytes();
    }
    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->PreEmit();

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->Emit(codeGenerator);

    codeGenerator->DoFinalCodeGen();
}

void Stmt::BuildScope() {

}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::BuildScope() {

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        scope->AddDecl(decls->Nth(i));

    for (int i=0,n=decls->NumElements();i<n;++i) {
        decls->Nth(i)->BuildScope();
    }
    for (int i=0,n=stmts->NumElements();i<n;++i) {
        stmts->Nth(i)->BuildScope();
    }
}

void StmtBlock::Check() {
    for (int i=0,n=decls->NumElements();i<n;++i) {
        decls->Nth(i)->Check();
    }
    for (int i=0,n=stmts->NumElements();i<n;++i) {
        stmts->Nth(i)->Check();
    }
}

Location *StmtBlock::Emit(CodeGenerator *cg) {
    for (int i = 0, n = decls->NumElements(); i < n; ++i) {
        VarDecl *d = dynamic_cast<VarDecl*>(decls->Nth(i));
        if (d == NULL)
            continue;
        Location *loc = cg->GenLocalVar(d->GetName(), d->GetMemBytes());
        d->SetMemLoc(loc);
    }

    for (int i = 0, n = stmts->NumElements(); i < n; ++i)
        stmts->Nth(i)->Emit(cg);

    return NULL;
}

int StmtBlock::GetMemBytes() {
    int memBytes = 0;
    for (int i=0,n=decls->NumElements();i<n;++i)
        memBytes +=decls->Nth(i)->GetMemBytes();
    for (int i=0,n=stmts->NumElements();i<n;++i)
        memBytes +=stmts->Nth(i)->GetMemBytes();
    return memBytes;
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

void ConditionalStmt::BuildScope() {


    test->BuildScope();
    body->BuildScope();
}

void ConditionalStmt::Check() {
    test->Check();
    body->Check();
    if(!test->GetType()->IsEquivalentTo(Type::boolType)) {
        ReportError::TestNotBoolean(test);
    }
}

void LoopStmt::BuildScope() {

    ConditionalStmt::BuildScope();
    scope->SetLoopStmt((this));
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::BuildScope() {

    LoopStmt::BuildScope();

    init->BuildScope();
    step->BuildScope();
}

Location *ForStmt::Emit(CodeGenerator *cg) {
    const char* top = cg->NewLabel();
    const char* bot = cg->NewLabel();

    Program::gBreakLabels->push(bot);
    init->Emit(cg);
    cg->GenLabel(top);
    Location *t = test->Emit(cg);
    cg->GenIfZ(t,bot);
    body->Emit(cg);
    step->Emit(cg);
    cg->GenGoto(top);
    cg->GenLabel(bot);
    Program::gBreakLabels->pop();
    return NULL;
}

int ForStmt::GetMemBytes() {
    return init->GetMemBytes()+test->GetMemBytes()
    +body->GetMemBytes()+step->GetMemBytes();
}

void WhileStmt::BuildScope() {
    LoopStmt::BuildScope();
}

Location *WhileStmt::Emit(CodeGenerator *cg) {
    const char* top = cg->NewLabel();
    const char* bot = cg->NewLabel();

    Program::gBreakLabels->push(bot);

    cg->GenLabel(top);
    Location *t = test->Emit(cg);
    cg->GenIfZ(t, bot);
    body->Emit(cg);
    cg->GenGoto(top);
    cg->GenLabel(bot);

    Program::gBreakLabels->pop();

    return nullptr;
}

int WhileStmt::GetMemBytes() {
    return test->GetMemBytes() + body->GetMemBytes();
}


void IfStmt::BuildScope() {


    test->BuildScope();
    body->BuildScope();

    if(elseBody!=NULL)
        elseBody->BuildScope();
}

void IfStmt::Check() {
    test->Check();
    body->Check();
    if(!test->GetType()->IsEquivalentTo(Type::boolType))
        ReportError::TestNotBoolean(test);
    if(elseBody!=NULL)
        elseBody->Check();
}


IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

Location *IfStmt::Emit(CodeGenerator *cg) {
    const char* els = cg->NewLabel();
    const char* bot = cg->NewLabel();

    Location *t = test->Emit(cg);
    cg->GenIfZ(t,els);
    body->Emit(cg);
    cg->GenGoto(bot);
    cg->GenLabel(els);
    if (elseBody) elseBody->Emit(cg);
    cg->GenLabel(bot);
    return nullptr;

}

int IfStmt::GetMemBytes() {
    int memBytes = test->GetMemBytes()+body->GetMemBytes();
    if(elseBody) memBytes+=elseBody->GetMemBytes();
    return memBytes;
}

void BreakStmt::Check() {
    Node* n = this;
    while(n!=NULL) {
        if(n->GetScope()->GetLoopStmt()!=NULL) {
            return;
        }
        n = n->GetParent();
    }
    ReportError::BreakOutsideLoop(this);
}



Location *BreakStmt::Emit(CodeGenerator *cg) {
    cg->GenGoto(Program::gBreakLabels->top());
    return nullptr;
}

int BreakStmt::GetMemBytes() {
    return 0;
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

void ReturnStmt::BuildScope() {
    expr->BuildScope();
}

void ReturnStmt::Check() {
    expr->Check();

    FnDecl *d = NULL;
    Node* n = this;
    while(n !=NULL) {
        if((d=n->GetScope()->GetFnDecl())!=NULL) {
            break;
        }
        n=n->GetParent();
    }
    if(d==NULL) {
        ReportError::Formatted(location,"return is only allowed inside a function");
        return;
    }
    Type *expected = d->GetReturnType();
    Type *given = expr->GetType();
    if(given== nullptr && strcmp(expected->GetName(),"void")==0) return;
    if(!given->IsEquivalentTo(expected)) {
        ReportError::ReturnMismatch(this,given,expected);
    }
}


Location *ReturnStmt::Emit(CodeGenerator *cg) {
    if (expr== nullptr)
        cg->GenReturn();
    else
        cg->GenReturn(expr->Emit(cg));
    return nullptr;
}

int ReturnStmt::GetMemBytes() {
    if (expr== nullptr)
        return 0;
    else
        return expr->GetMemBytes();
}

PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}


void PrintStmt::BuildScope() {

    for(int i=0,n=args->NumElements();i<n;++i) {
        args->Nth(i)->BuildScope();
    }
}

void PrintStmt::Check() {
    for(int i=0,n=args->NumElements();i<n;++i) {
        Type *given = args->Nth(i)->GetType();
        if(!(given->IsEquivalentTo(Type::intType)||
        given->IsEquivalentTo(Type::boolType)||
        given->IsEquivalentTo(Type::stringType))) {
            ReportError::PrintArgMismatch(args->Nth(i),i+1,given);
        }
    }

    for(int i=0,n=args->NumElements();i<n;++i) {
        args->Nth(i)->Check();
    }
}

int PrintStmt::GetMemBytes() {
    int memBytes = 0;
    for (int i = 0, n = args->NumElements(); i < n; ++i)
        memBytes += args->Nth(i)->GetMemBytes();
    return memBytes;
}

Location *PrintStmt::Emit(CodeGenerator *cg) {
    for (int i=0,n=args->NumElements();i<n;++i) {
        Expr *e = args->Nth(i);
        BuiltIn b = e->GetType()->GetPrint();

        Assert(b != NumBuiltIns);
        cg->GenBuiltInCall(b, e->Emit(cg));
    }
    return nullptr;
}


