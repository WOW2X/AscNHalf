#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "TileAssembler.h"

//=======================================================
// remove last return or LF and tailing SPACE
// remove all char after a #

void chompAndTrim(std::string& str)
{
    for(unsigned int i=0;i<str.length(); ++i) {
        char lc = str[i];
        if(lc == '#') {
            str = str.substr(0,i);
            break;
        }
    }

    while(str.length() >0) {
        char lc = str[str.length()-1];
        if(lc == '\r' || lc == '\n' || lc == ' ') {
            str = str.substr(0,str.length()-1);
        } else {
            break;
        }
    }
}

//=======================================================
/**
This callback method is called for each model found in the dir file.
return true if it should be included in the vmap
*/
bool modelNameFilter(char *pName)
{
#if 0
    bool result;
    result = !Wildcard::wildcardfit("*bush[0-9]*", pName);
    if(result) result = !Wildcard::wildcardfit("*shrub[0-9]*", pName);
    if(result) result = !Wildcard::wildcardfit("*_Bushes_*", pName);
    if(result) result = !Wildcard::wildcardfit("*_Bush_*", pName);
    if(!result) {
        printf("%s",pName);
    }
#endif
    return true;
}

//=======================================================
int main(int argc, char* argv[])
{
    bool ok = true;
    char *src = "buildings";
	char *dest = "vmaps";
    char *conffile = NULL;
    VMAP::TileAssembler* ta = new VMAP::TileAssembler(std::string(src), std::string(dest));
    ta->setModelNameFilterMethod(modelNameFilter);

	static uint32 maps[999] = { 0, 1, 13, 25, 29, 30, 33, 34, 35, 36, 37, 42, 43, 44, 47, 48, 70, 90, 109, 129, 169, 189, 209, 229, 230, 249, 269, 289, 309, 329, 369, 389, 409, 429, 449, 450, 451, 469, 489, 509, 529, 530, 531, 532, 533, 534, 540, 542, 543, 544, 545, 546, 547, 548, 550, 552, 553, 554, 555, 556, 557, 558, 559, 560, 562, 564, 565, 568, 571, 572, 573, 574, 575, 576, 578, 580, 582, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 628, 641, 642, 647, 649, 650 };
	for(uint32 i = 0; maps[i] != 0; ++i)
		ta->addWorldAreaMapId( maps[i] );

    if(ok) { ok = ta->convertWorld(); }
    if(ok) {
        printf("Ok, all done\n");
    } else {
        printf("exit with errors\n");
        return 1;
    }
    delete ta;
    return 0;
}
