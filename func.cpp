#include "func.h"
#include <cmath>

int modulusz(int a){
	if (a > 0) return a; else return -a;
}

int distance(int x1,int x2,int y1,int y2){
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
