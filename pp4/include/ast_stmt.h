/* File: ast_stmt.h
 * ----------------
 * The Stmt class and its subclasses are used to represent
 * statements in the parse tree.  For each statment in the
 * language (for, if, return, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Stmt classes to implement
 * semantic analysis for rules pertaining to statements.
 */


#ifndef _H_ast_stmt
#define _H_ast_stmt

#include "list.h"
#include "ast.h"
#include "hashtable.h"
#include "ast_type.h"
#include <stack>
#include "codegen.h"
class Decl;
class VarDecl;
class Expr;
class ClassDecl;
class LoopStmt;
class FnDecl;
class CodeGenerator;
class Location;

class Scope
{
    private:
        Scope * parent;
    public:
         Hashtable<Decl*> * table;
         ClassDecl *         classDecl;
         LoopStmt *          loopStmt;
         FnDecl *            fnDecl;
    public:
        Scope():table(new Hashtable<Decl*>),classDecl(NULL),loopStmt(NULL),
                fnDecl(NULL) {}
        void SetParent(Scope * p) {parent = p;}
        Scope* GetParent() {return parent;}

        void SetClassDecl(ClassDecl * d) {classDecl = d;}
        ClassDecl* GetClassDecl() {return classDecl;}

        void SetLoopStmt(LoopStmt * l) {loopStmt = l;}
        LoopStmt* GetLoopStmt() {return loopStmt;}

        void SetFnDecl(FnDecl * f) {fnDecl = f;}
        FnDecl* GetFnDecl() {return fnDecl;}

        int  AddDecl(Decl* decl);
        friend ostream& operator<<(ostream& out,Scope* s);
};

class Program : public Node
{
  public:
    static Scope *gScope;
    static stack<const char*> *gBreakLabels;
  protected:
     List<Decl*> *decls;
     CodeGenerator *codeGenerator;
     
  public:
     Program(List<Decl*> *declList);
     void Check();
     void Emit();
     Scope*  GetScope() override  {return gScope;}
  private:
    void BuildScope();
};

class Stmt : public Node
{
protected:

  public:
     Stmt() : Node() {scope = new Scope;}
     Stmt(yyltype loc) : Node(loc) {scope = new Scope;}

     virtual void BuildScope()=0;
     virtual void Check()=0;
     virtual Location* Emit(CodeGenerator *cg)=0;
     virtual int  GetMemBytes()=0;
};

class StmtBlock : public Stmt 
{
  protected:
    List<VarDecl*> *decls;
    List<Stmt*> *stmts;
    
  public:
    StmtBlock(List<VarDecl*> *variableDeclarations, List<Stmt*> *statements);

    void BuildScope() override ;
    Location* Emit(CodeGenerator *cg) override ;
    int   GetMemBytes() override ;
    void Check() override ;
};

  
class ConditionalStmt : public Stmt
{
  protected:
    Expr *test;
    Stmt *body;
  
  public:
    ConditionalStmt(Expr *testExpr, Stmt *body);

    void BuildScope() override;
    void Check() override;
};

class LoopStmt : public ConditionalStmt 
{
protected:
    const char* breakLabel;
  public:
    LoopStmt(Expr *testExpr, Stmt *body)
            : ConditionalStmt(testExpr, body) {}

    void BuildScope() override ;
    void SetBreak(const char* b) {breakLabel = b;}
    const char* GetBreak() {return breakLabel;}
};

class ForStmt : public LoopStmt 
{
  protected:
    Expr *init, *step;
  
  public:
    ForStmt(Expr *init, Expr *test, Expr *step, Stmt *body);

    void BuildScope() override;
    Location* Emit(CodeGenerator *cg) override;
    int   GetMemBytes() override ;
};

class WhileStmt : public LoopStmt {
public:
    WhileStmt(Expr *test, Stmt *body) : LoopStmt(test, body) {}

    void BuildScope() override;
    Location *Emit(CodeGenerator *cg) override ;
    int GetMemBytes() override ;
};

class IfStmt : public ConditionalStmt 
{
  protected:
    Stmt *elseBody;
  
  public:
    IfStmt(Expr *test, Stmt *thenBody, Stmt *elseBody);
    void Check() override;
    void BuildScope() override ;
    Location* Emit(CodeGenerator *cg) override;
    int GetMemBytes() override ;


};

class BreakStmt : public Stmt 
{
  public:
    BreakStmt(yyltype loc) : Stmt(loc) {}
    void BuildScope() override {};
    void Check() override ;
    Location* Emit(CodeGenerator *cg) override;
    int GetMemBytes() override ;
};

class ReturnStmt : public Stmt  
{
  protected:
    Expr *expr;
  
  public:
    ReturnStmt(yyltype loc, Expr *expr);

    void BuildScope() override ;
    void Check() override ;
    Location* Emit(CodeGenerator *cg) override;
    int GetMemBytes() override ;
};

class PrintStmt : public Stmt
{
  protected:
    List<Expr*> *args;
    
  public:
    PrintStmt(List<Expr*> *arguments);

    void BuildScope() override;
    void Check() override;
    Location* Emit(CodeGenerator *cg) override;
    int GetMemBytes() override;
};


#endif
