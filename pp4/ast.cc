/* File: ast.cc
 * ------------
 */

#include "ast.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h> // strdup
#include <stdio.h>  // printf

Node::Node(yyltype loc) {
    location = new yyltype(loc);
    parent = nullptr;
    scope = nullptr;

}

Node::Node() {
    location = nullptr;
    parent = nullptr;
    scope = nullptr;
}

Identifier::Identifier(yyltype loc, const char *n) : Node(loc) {
    name = strdup(n);
}

bool Identifier::operator==(const Identifier &rhs) {
    return strcmp(name, rhs.name) == 0 ? true : false;
}