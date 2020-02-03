//
//  jsoner.h
//  JSONER2
//
//  Created by Seyyed Parsa Neshaei on 2/2/20.
//  Copyright © 2020 Seyyed Parsa Neshaei. All rights reserved.
//

#ifndef jsoner_h
#define jsoner_h

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NESTING_LEVELS 32

#define jsonerTrue 1
#define jsonerFalse 0

typedef enum NodeType { NODETYPE_OBJECT = 1, NODETYPE_ARRAY = 2 } NodeType;

typedef struct Node {
    int index;
    NodeType nodeType;
} Node;

typedef struct JSONPreferences {
    char *str;
    unsigned int str_len;
    char *str_pos; // where are we writing in *str
    Node nodes[NESTING_LEVELS];
    int nodesCount; // position in the above array
    char temporary[NESTING_LEVELS]; // local buffer for int/double convertions (got the idea from a lib)
} JSONPreferences;

void startBuildingTheJSON(char *str, unsigned int str_len, NodeType type);
void insertKeyWithStringWithQuotes(char *key, char *string);
void insertKeyWithIntNumber(char *key, int number);
void insertKeyWithDoubleNumber(char *key, double number);
void insertKeyWithNull(char *key);
void insertKeyWithBoolean(char *key, int boolean);
void insertMemberWithStringWithQuotes(char *string);
void insertMemberWithIntNumber(int value);
void insertMemberWithDoubleNumber(double value);
void insertMemberWithNull(void);
void insertMemberWithBoolean(int boolean);
void insertKeyWithArray(char *key);
void insertKeyWithObject(char *key);
void endArrayAndNestedObjectBlock(void);
void endArray(void);
void endNestedObjectBlock(void);
void insertMemberWithObject(void);
void insertMemberWithArray(void);
int finishBuildingTheJSON(void);

#endif /* jsoner_h */
