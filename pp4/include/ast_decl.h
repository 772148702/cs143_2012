/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "ast_stmt.h"
#include "ast_type.h"
class Type;
class Identifier;
class Stmt;
class InterfaceDecl;

class Decl : public Node 
{
  protected:
    Identifier *id;

  public:
    Decl(Identifier *name);
    friend ostream& operator<<(ostream& out, Decl *d) { return out << d->id; }

    virtual bool IsEquivalentTo(Decl *other);

    char* GetName() {return id->GetName();}


    virtual void BuildScope();
    virtual void Check()=0;
    virtual void PreEmit() = 0;
    virtual Location* Emit(CodeGenerator *cg) = 0;
    virtual int GetMemBytes()=0;
    virtual int GetVTblBytes()=0;
    virtual  void AddLabelPrefix(const char *prefix)=0;

};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    Location *memLoc;
    int memOffset;
  public:
    VarDecl(Identifier *name, Type *type);
    Type* GetType() {return type;}

    void PreEmit() override {};
    Location *Emit(CodeGenerator *cg) override {return nullptr;};
    int GetMemBytes() override;
    int GetVTblBytes() override {return 0;};
    void AddLabelPrefix(const char *prefix) override {};
    bool IsEquivalentTo(Decl* other) override ;

    Location* GetMemLoc() {return memLoc;}
    void SetMemLoc(Location* m) {memLoc = m;}
    int GetMemOffset() {return memOffset;}
    void SetMemOffset(int m) {memOffset=m;}

    void Check() override ;

  private:
    void CheckType();
};

class ClassDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;

  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);

    void BuildScope() override ;
    void Check() override ;

    NamedType* GetType() {return new NamedType(id);}
    NamedType* GetExtends() {return extends;}
    List<NamedType*>* GetImplements() {return implements;}

    void PreEmit() override ;
    Location* Emit(CodeGenerator *cg) override ;
    int GetMemBytes() override ;
    int GetVTblBytes() override;
    void AddLabelPrefix(const char* prefix) override {}



  private:
    void CheckExtends();
    void CheckImplements();

    void CheckExtendedMembers(NamedType *extType);
    void CheckImplementedMembers(NamedType *impType);
    void CheckAgainstScope(Scope *other);
    void CheckImplementsInterfaces();
    List<FnDecl*>* GetMethodDecls();
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);

    void BuildScope() override ;
    void Check() override ;
    void PreEmit() override {}
    Location* Emit(CodeGenerator *cg) override {return nullptr;}
    int GetMemBytes() override {return 0;}
    int GetVTblBytes() override {return 0;}
    void AddLabelPrefix(const char *prefix) override {}
    Type* GetType() {return new NamedType(id);}
    List<Decl*>* GetMembers() {return members;}
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    std::string *label;
    int vtlOffset;
    bool isMethod;
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    const char* GetLabel();
    void SetFunctionBody(Stmt *b);
    bool IsEquivalentTo(Decl *other) override ;
    Type* GetReturnType() {return returnType;}
    List<VarDecl*>* GetFormals() {return formals;}
    void BuildScope() override;
    void Check() override;
    void PreEmit() override;
    Location* Emit(CodeGenerator *cg) override;
    int GetMemBytes()  override {return 0;}
    int GetVTblBytes() override;
    void AddLabelPrefix(const char *prefix) override ;
    int  GetVTblOffset() {return vtlOffset;}
    void SetVTblOffset(int v) {vtlOffset = v;}
    void SetIsMethod(bool b) {isMethod = b;}
    Type* GetType() { return returnType; }
    bool HasReturnVal();

};

#endif
