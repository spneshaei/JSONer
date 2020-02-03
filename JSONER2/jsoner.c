//
//  jsoner.c
//  JSONER2
//
//  Created by Seyyed Parsa Neshaei on 2/2/20.
//  Copyright © 2020 Seyyed Parsa Neshaei. All rights reserved.
//

#include "jsoner.h"

JSONPreferences jsonPreferences;

void startBuildingTheJSON(char *str, unsigned int str_len, NodeType type);
void addStringWithoutQuotesToJSONBeingBuilt(char *str);
void addStringWithQuotesToJSONBeingBuilt(char *str);
void addCharacterToJSONBeingBuilt(char c);
void addNodeType(NodeType nodeType);
NodeType getNodeType(void);
void addNeccessaryEntersAndSpaces(void);

void insertKey(char *key);


static void strreverse(char* begin, char* end) { // copied function
    char aux;
    while (end > begin)
        (void)(aux = *end), (void)(*end-- = *begin), *begin++ = aux;
}

void integerToString(int32_t value, char *str) { // copied function
    char* wstr=str;
    unsigned int uvalue = (value < 0) ? -value : value;
    do *wstr++ = (char)(48 + (uvalue % 10)); while(uvalue /= 10);
    if (value < 0) *wstr++ = '-';
    *wstr='\0';
    strreverse(str,wstr-1);
}

static const double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
    10000000, 100000000, 1000000000};

void doubleToString(double value, char* str, int prec) { // copied function. also prec = precision (number of digist to the right of the decimal point)
    /* if input is larger than thres_max, revert to exponential */
    const double thres_max = (double)(0x7FFFFFFF);
    int count;
    double diff = 0.0;
    char* wstr = str;
    int neg= 0;
    int whole;
    double tmp;
    uint32_t frac;
    
    /* Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (! (value == value)) {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n'; str[3] = '\0';
        return;
    }
    
    if (prec < 0) {
        prec = 0;
    } else if (prec > 9) {
        /* precision of >= 10 can lead to overflow errors */
        prec = 9;
    }
    
    /* we'll work in positive values and deal with the
     negative sign issue later */
    if (value < 0) {
        neg = 1;
        value = -value;
    }
    
    
    whole = (int) value;
    tmp = (value - whole) * pow10[prec];
    frac = (uint32_t)(tmp);
    diff = tmp - frac;
    
    if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
         if last digit is 0.  That last part is strange */
        ++frac;
    }
    
    /* for very large numbers switch back to native sprintf for exponentials.
     anyone want to write code to replace this? */
    /*
     normal printf behavior is to print EVERY whole number digit
     which can be 100s of characters overflowing your buffers == bad
     */
    if (value > thres_max) {
        sprintf(str, "%e", neg ? -value : value);
        return;
    }
    
    if (prec == 0) {
        diff = value - whole;
        if (diff > 0.5) {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }
        
        //vvvvvvvvvvvvvvvvvvv  Diff from modp_dto2
    } else if (frac) {
        count = prec;
        // now do fractional part, as an unsigned number
        // we know it is not 0 but we can have leading zeros, these
        // should be removed
        while (!(frac % 10)) {
            --count;
            frac /= 10;
        }
        //^^^^^^^^^^^^^^^^^^^  Diff from modp_dto2
        
        // now do fractional part, as an unsigned number
        do {
            --count;
            *wstr++ = (char)(48 + (frac % 10));
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
    }
    
    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);
    if (neg) {
        *wstr++ = '-';
    }
    *wstr='\0';
    strreverse(str, wstr-1);
}

void startBuildingTheJSON(char *str, unsigned int str_len, NodeType type) {
    memset(str, 0, str_len);
    jsonPreferences.str = str;
    jsonPreferences.str_len = str_len;
    jsonPreferences.str_pos = str;
    jsonPreferences.nodes[0].nodeType = type;
    jsonPreferences.nodes[0].index = 0;
    jsonPreferences.nodesCount = 0;
    if (type == NODETYPE_ARRAY) {
        addCharacterToJSONBeingBuilt('[');
    } else { // obj
        addCharacterToJSONBeingBuilt('{');
    }
}

int finishBuildingTheJSON() { // returns -1 if error
    if (jsonPreferences.nodesCount == 0) {
        addCharacterToJSONBeingBuilt('\n');
        if (jsonPreferences.nodes[0].nodeType == NODETYPE_ARRAY) {
            addCharacterToJSONBeingBuilt(']');
        } else { // obj
            addCharacterToJSONBeingBuilt('}');
        }
        return 0;
    } else {
        addCharacterToJSONBeingBuilt('}');
        return -1;
    }
}

void endArray() {
    if (jsonPreferences.nodes[jsonPreferences.nodesCount].index > 0) {
        addNeccessaryEntersAndSpaces();
    }
    addCharacterToJSONBeingBuilt(']');
}

void endNestedObjectBlock() {
    if (jsonPreferences.nodes[jsonPreferences.nodesCount].index > 0) {
        addNeccessaryEntersAndSpaces();
    }
    addCharacterToJSONBeingBuilt('}');
}

void endArrayAndNestedObjectBlock() {
    if (jsonPreferences.nodes[jsonPreferences.nodesCount].index > 0) {
        addNeccessaryEntersAndSpaces();
    }
    if (getNodeType() == NODETYPE_ARRAY) {
        addCharacterToJSONBeingBuilt(']');
    } else { // obj
        addCharacterToJSONBeingBuilt('}');
    }
}

void insertKeyWithStringWithoutQuotes(char *key, char *string) {
    insertKey(key);
    addStringWithoutQuotesToJSONBeingBuilt(string);
}

void insertKeyWithStringWithQuotes(char *key, char *string) {
    insertKey(key);
    addStringWithQuotesToJSONBeingBuilt(string);
}

void insertKeyWithIntNumber(char *key, int number) {
    integerToString(number, jsonPreferences.temporary);
    insertKeyWithStringWithoutQuotes(key, jsonPreferences.temporary);
}

void insertKeyWithDoubleNumber(char *key, double number) {
    doubleToString(number, jsonPreferences.temporary, 6); // number 6 copied!
    insertKeyWithStringWithoutQuotes(key, jsonPreferences.temporary);
}

void insertKeyWithNull(char *key) {
    insertKeyWithStringWithoutQuotes(key, "null");
}

void insertKeyWithBoolean(char *key, int boolean) {
    if (boolean) {
        insertKeyWithStringWithoutQuotes(key, "true");
    } else {
        insertKeyWithStringWithoutQuotes(key, "false");
    }
}

void insertKeyWithArray(char *key) {
    insertKey(key);
    addCharacterToJSONBeingBuilt('[');
    addNodeType(NODETYPE_ARRAY);
}

void insertKeyWithObject(char *key) {
    insertKey(key);
    addCharacterToJSONBeingBuilt('{');
    addNodeType(NODETYPE_OBJECT);
}

void insertMember(void);

void insertMemberWithStringWithoutQuotes(char *string) {
    insertMember();
    addStringWithoutQuotesToJSONBeingBuilt(string);
}

void insertMemberWithStringWithQuotes(char *string) {
    insertMember();
    addStringWithQuotesToJSONBeingBuilt(string);
}

void insertMemberWithIntNumber(int value) {
    integerToString(value, jsonPreferences.temporary);
    insertMemberWithStringWithoutQuotes(jsonPreferences.temporary);
    // TODO: how to fix this??
    addCharacterToJSONBeingBuilt(',');
}

void insertMemberWithDoubleNumber(double value) {
    doubleToString(value, jsonPreferences.temporary, 6);
    insertMemberWithStringWithoutQuotes(jsonPreferences.temporary);
}

void insertMemberWithNull() {
    insertMemberWithStringWithoutQuotes("true");
}

void insertMemberWithBoolean(int boolean) {
    if (boolean) {
        insertMemberWithStringWithoutQuotes("true");
    } else {
        insertMemberWithStringWithoutQuotes("false");
    }
}

void insertMemberWithObject() {
    insertMember();
    addCharacterToJSONBeingBuilt('{');
    addNodeType(NODETYPE_OBJECT);
}

void insertMemberWithArray() {
    insertMember();
    addCharacterToJSONBeingBuilt('[');
    addNodeType(NODETYPE_ARRAY);
}

void addNeccessaryEntersAndSpaces() {
    addCharacterToJSONBeingBuilt('\n');
    for (int i = 0; i < jsonPreferences.nodesCount + 1; i++) {
        addStringWithoutQuotesToJSONBeingBuilt("    "); // or any other (this amount was borrowed from a lib)
    }
}

void addNodeType(NodeType nodeType) {
    if (jsonPreferences.nodesCount + 1 < NESTING_LEVELS) {
        jsonPreferences.nodesCount += 1;
        jsonPreferences.nodes[jsonPreferences.nodesCount].index = 0;
        jsonPreferences.nodes[jsonPreferences.nodesCount].nodeType = nodeType;
    } else {
        printf("\nFATAL JSONer ERROR: NESTING_LEVELS REACHED\n");
    }
}

NodeType getNodeType() {
    if (jsonPreferences.nodesCount != 0) {
        jsonPreferences.nodesCount -= 1;
    }
    return jsonPreferences.nodes[jsonPreferences.nodesCount].nodeType;
}

void addCharacterToJSONBeingBuilt(char c) {
    if ((unsigned int) (jsonPreferences.str_pos - jsonPreferences.str) < jsonPreferences.str_len) {
        *jsonPreferences.str_pos++ = c;
    }
}

void addStringWithQuotesToJSONBeingBuilt(char *str) {
    addCharacterToJSONBeingBuilt('\"');
    while(*str != '\0') {
        addCharacterToJSONBeingBuilt(*str++);
    }
    addCharacterToJSONBeingBuilt('\"');
}

void addStringWithoutQuotesToJSONBeingBuilt(char *str) {
    while(*str != '\0') {
        addCharacterToJSONBeingBuilt(*str++);
    }
}

void insertKey(char *key) {
    if (jsonPreferences.nodes[jsonPreferences.nodesCount].nodeType == NODETYPE_OBJECT && jsonPreferences.nodes[jsonPreferences.nodesCount].index++ > 0) {
        addCharacterToJSONBeingBuilt(',');
    }
    addNeccessaryEntersAndSpaces();
    addStringWithQuotesToJSONBeingBuilt(key);
    addCharacterToJSONBeingBuilt(':');
    addCharacterToJSONBeingBuilt(' ');
}

void insertMember() {
     if (jsonPreferences.nodes[jsonPreferences.nodesCount].nodeType == NODETYPE_OBJECT && jsonPreferences.nodes[jsonPreferences.nodesCount].index++ > 0) {
         addCharacterToJSONBeingBuilt(',');
     }
    addNeccessaryEntersAndSpaces();
}
