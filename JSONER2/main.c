//
//  main.c
//  JSONER2
//
//  Created by Seyyed Parsa Neshaei on 2/2/20.
//  Copyright © 2020 Seyyed Parsa Neshaei. All rights reserved.
//

#include <stdio.h>
#include "jsoner.h"

int main(int argc, const char * argv[]) {
    char wholeString[5000]; int wholeStringLen = 5000;
    startBuildingTheJSON(wholeString, wholeStringLen, NODETYPE_OBJECT);
    
    insertKeyWithStringWithQuotes("mykey1", "myvalue1");
    insertKeyWithIntNumber("mykey2", 2);
    insertKeyWithDoubleNumber("mykey3", 3.1);
    insertKeyWithNull("mykey4");
    insertKeyWithBoolean("mykey5", jsonerTrue);
    insertKeyWithObject("mykey6"); {
        insertKeyWithIntNumber("mykey7", 7);
        insertKeyWithIntNumber("mykey8", 8);
        insertKeyWithIntNumber("mykey9", 9);
        endNestedObjectBlock();
    }
    insertKeyWithArray("mykey10"); {
        insertMemberWithIntNumber(-11);
        insertMemberWithStringWithQuotes("mykey12");
        endArray();
    }
    
    finishBuildingTheJSON();
    printf("%s\n", wholeString);
    return 0;
}
