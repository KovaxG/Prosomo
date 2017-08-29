/**
  * Jatek : Prosomo
  * Datum : 2013.03.01
  */
  
#include <allegro.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <fstream>
using namespace std;

#include "func.h"

int ScreenWidth  = 640;
int ScreenHeight = 480;

#define BufferWidth 4098
#define BufferHeight 4098

#define MatrixWidth BufferWidth/64
#define MatrixHeight BufferHeight/64

#define InvWidth 356
#define InvHeight 292

#define Black makecol(0,0,0)
#define White makecol(255,255,255)

//bool FULLSCREEN = true;
short int mapMatrix[MatrixHeight][MatrixWidth]; 
enum DIRECTION{UP,DOWN,LEFT,RIGHT};
enum FOODTYPE{APPLE,BERRY};
bool areBitmapsAndSoundsLoaded = false;

struct FOOD{
	int Mx,My;
	int x,y;
	int foods;
	double time;
};

struct WOOD{
	int Mx,My;
	int x,y;
	int HP;
	bool cut;
	double time;
};

struct STONE{
	int Mx,My;
	int stones;
};

struct INVENTORY{
	int wood;
	bool woodTaken;
	int food;
	bool foodTaken;
	int stone;
	bool stoneTaken;
	bool axe;
	int bow;
	bool bowTaken;
	int arrow;
	bool arrowTaken;
	int gun;
	bool gunTaken;
	int kugel;
	bool kugelTaken;
	int meat;
	bool meatTaken;
	int ironOre;
	bool ironOreTaken;
	int iron;
	bool ironTaken;
	int stake;
	bool stakeTaken;
};

struct HUMAN_INVENTORY{
	int wood;
	int food;
	int gun;
	int kugel;
	int bow;
	int arrow;
};

struct SLOTS{
	int x,y;
	BITMAP *pic;
	int nr;
	std::string has;
};

struct HOUSE{
	int HP;
	int x,y;
};

int effood;
int appleTreeFoodNumber = 0;
FOOD appleTreeFood[200];
int bushFoodNumber = 0;
FOOD bushFood[200];
int woodNumber = 0;
WOOD holz[1000];
SLOTS slot[12];
int stoneNumber = 0;
int ironOreNumber = 0;
STONE stein[100];
STONE eisenErz[100];
bool playerIsDead = false;


BITMAP *grass;
BITMAP *buffer;
BITMAP *water;
BITMAP *def;
BITMAP *tree;
BITMAP *chr;
BITMAP *atree;
BITMAP *O;
BITMAP *dead;
BITMAP *invScreen;
BITMAP *foodPic;
BITMAP *woodPic;
BITMAP *cuttree;
BITMAP *rock;
BITMAP *bush;
BITMAP *bowPic;
BITMAP *gunPic;
BITMAP *house;
BITMAP *personPic1;
BITMAP *personPic2;
BITMAP *arrow; 
BITMAP *bullet;
BITMAP *meatPic;
BITMAP *karcsiPic;
BITMAP *kugelPic;
BITMAP *arrowPic;
BITMAP *ironBarPic;
BITMAP *ironOrePic;
BITMAP *stonePic;
BITMAP *ironOreStone;
BITMAP *smithy;
BITMAP *buildMenu;
BITMAP *craftMenu;
BITMAP *cookedMeatPic;
BITMAP *houseMenu;
BITMAP *apbuff; // All Purpose Buffer
BITMAP *menuScrn;
MIDI *musica;
SAMPLE *bite;
SAMPLE *pick;
SAMPLE *woodcut;
SAMPLE *hammer;
SAMPLE *gunFire;
SAMPLE *bulletHit;
SAMPLE *bowFire;
SAMPLE *hitO;
SAMPLE *mine1;
SAMPLE *mine2;
SAMPLE *backMusic;

//==========Initializations==============

int people1Number = 10;
int people2Number = 10;
int animalNumber = 50;

int charX = 64;
int charY = 64;
double HP = 100; // 100

//==============CLASSES================

class animal{
private:
	double HP;
	int x,y;
	int speed;
	DIRECTION dir;
	BITMAP *pic;
	double foodMeter;
	int xf,yf,index; // destination
	int CPC;
	bool move;
	bool hasDestination;
	bool idle;
public:
	void initialize(int,int,BITMAP*);
	void draw();
	void doStuff();
	void searchForFood(int &,int &,int &);
	void eat(int);
	void goTo(int,int);
	int getHP(){return (int)HP;}
	int getX(){return x;}
	int getY(){return y;}
	void inflictDamage(int a){
		HP -= a; 
		play_sample(hitO,128,128,1000,false);
		if (dir == RIGHT) masked_blit(pic,buffer,0,30,x,y,20,15);
		else if (dir == LEFT) masked_blit(pic,buffer,20,30,x,y,20,15);
	}
	int getPicHeight(){return 15;}
	int getPicWidth(){return 20;}
	//~animal(){destroy_bitmap(pic);}
};

void animal::initialize(int iksz,int ipsz,BITMAP *P){
	HP = 20;
	x = iksz;
	y = ipsz;
	speed = 1;
	dir = RIGHT;
	pic = P;
	foodMeter = 100.0;
	CPC = 0;
	move = false;
	hasDestination = false;
	idle = true;
}

void animal::draw(){
	if (HP > 0){
	if (!move) masked_blit(pic,buffer,0,0,x,y,20,15);
	else{
		if (dir == RIGHT){
			masked_blit(pic,buffer,CPC,0,x,y,20,15);
			if (CPC == 40) CPC = 0;
			else CPC += 20;
		}
		else if (dir == LEFT){
			masked_blit(pic,buffer,CPC,15,x,y,20,15);
			if (CPC == 40) CPC = 0;
			else CPC += 20;
		}
	}
	}
	else masked_blit(pic,buffer,0,30,x,y,20,15);
}

void animal::searchForFood(int &xf,int &yf,int &index){
	int MinD = 999999;
	for (int i=0;i<=bushFoodNumber;i++){
		if (distance(bushFood[i].x,x,bushFood[i].y,y) < MinD && bushFood[i].foods > 0){
			MinD = distance(bushFood[i].x,x,bushFood[i].y,y);
			xf = bushFood[i].x;
			yf = bushFood[i].y + 49;
			index = i;
		}
	}
}

void animal::eat(int index){
	if (distance(x,charX,y,charY) <= ScreenWidth/2) play_sample(bite,30,128,1000,false);
	bushFood[index].foods -= 1;
	foodMeter = 100;
	effood--;
}

void animal::doStuff(){
	if (HP > 0){
		if (foodMeter > 0){
			foodMeter -= 0.2;
			if (!idle && !hasDestination){
				hasDestination = true;
				idle = false;
				xf = rand()%BufferWidth;
				yf = rand()%BufferHeight;
			}
			if (hasDestination){
				if (xf > x){
				x += speed;
				move = true;
				dir = RIGHT;
			}
			else if (xf < x){
				x -= speed;
				move = true;
				dir = LEFT;
			}
			if (yf > y){
				y += speed;
				move = true;
			}
			else if (yf < y){
				y -= speed;
				move = true;
			}
			if (xf == x && yf == y){
				hasDestination = false;
				idle = true;
				move = false;
			}
		}
	}
	else{
		HP -= 0.005;
		searchForFood(xf,yf,index);
		if (xf > x){
			x += speed;
			move = true;
			dir = RIGHT;
		}
		else if (xf < x){
			x -= speed;
			move = true;
			dir = LEFT;
		}
		if (yf > y){
			y += speed;
			move = true;
		}
		else if (yf < y){
			y -= speed;
			move = true;
		}
		if (xf == x && yf == y){
			if (bushFood[index].foods > 0) eat(index);
			move = false;
			if (rand() %2) idle = false;
			else idle = true;
		}
	}
	}
	draw();
}
//================END=OF=ANIMAL===============

class human{
	private:
		double HP;
		int x,y;
		int speed;
		DIRECTION dir;
		BITMAP *pic;
		double foodMeter;
		int xf,yf,index; // destination
		int CPC;
		bool move;
		bool arrived;
		FOODTYPE foodType;
		bool hasHouse;
		HUMAN_INVENTORY I;
		bool suitablePlaceFound;
		int houseX;
		int houseY;
		bool sleep;
		short int relationToPlayer;
		bool doFire;
		long int lastFired;
		int bulletIndex;
		bool Karcsi;
	public:
		//~human(){destroy_bitmap(pic);}
		int getHP(){return (int)HP;}
		int getX(){return x;}
		int getY(){return y;}
		int getBulletIndex(){return bulletIndex;}
		void setRelationToPlayer(int r){relationToPlayer = r;}
		void inflictDamage(int a){
			HP -= a; 
			play_sample(hitO,128,128,1000,false);
			if (dir == RIGHT) masked_blit(pic,buffer,0,72,x,y,20,36);
			else if (dir == LEFT) masked_blit(pic,buffer,20,72,x,y,20,36);
		}
		int getPicHeight(){return 36;}
		int getPicWidth(){return 20;}
		
		void initialize(int iksz,int ipsz,BITMAP *P,int bulInd){
			HP = 100;
			x = iksz;
			y = ipsz;
			speed = 2;
			dir = RIGHT;
			pic = P;
			foodMeter = 100.0;
			CPC = 0;
			move = false;
			hasHouse = false;
			I.wood = 0;
			I.food = 0;
			suitablePlaceFound = false;
			sleep = false;
			relationToPlayer = 0;
			doFire = false;
			lastFired = time(0);
			bulletIndex = bulInd;
			I.gun = 0;
			I.kugel = 2;
			I.bow = 1;
			I.arrow = 100;
			Karcsi = false;
		}
		
		void setKarcsi(bool a){
			Karcsi = a;
			HP = 2000;
		}
		
		void draw(){
			if (!move) masked_blit(pic,buffer,0,0,x,y,20,36);
			else if (dir == RIGHT){
				masked_blit(pic,buffer,CPC,0,x,y,20,36);
				if (CPC == 40) CPC = 0;
				else CPC+=20;
			}
			else if (dir == LEFT){
				masked_blit(pic,buffer,CPC,36,x,y,20,36);
				if (CPC == 40) CPC = 0;
				else CPC+=20;
			}
		}
		
		void walk(int xf, int yf){
			if (xf >= x+speed){
				x += speed;
				move = true;
				dir = RIGHT;
			}
			else if (xf <= x-speed){
				x -= speed;
				move = true;
				dir = LEFT;
			}
			if (yf >= y+speed){
				y += speed;
				move = true;
			}
			else if (yf <= y-speed){
				y -= speed;
				move = true;
			}
			if (modulusz(xf-x) < speed && modulusz(yf-y) < speed){
				x = xf;
				y = yf;
				arrived = true;
				move = false;
			} else arrived = false;	
		}
		
		void findClosestFood(){
			int MinD = 999999;
			for (int i=0;i<=bushFoodNumber;i++){
				if (distance(bushFood[i].x,x,bushFood[i].y,y) < MinD && bushFood[i].foods > 0){
					MinD = distance(bushFood[i].x,x,bushFood[i].y,y);
					xf = bushFood[i].x;
					yf = bushFood[i].y + 28;
					index = i;
					foodType = BERRY;
				}	
			}
			for (int i=0;i<=appleTreeFoodNumber;i++){
				if (distance(appleTreeFood[i].x,x,appleTreeFood[i].y,y) < MinD && appleTreeFood[i].foods > 0){
					MinD = distance(appleTreeFood[i].x,x,appleTreeFood[i].y,y);
					xf = appleTreeFood[i].x;
					yf = appleTreeFood[i].y + 28;
					index = i;
					foodType = APPLE;
				}	
			}
		}
		
		void eatOrWhatever(){
			if (distance(x,charX,y,charY) <= ScreenWidth/2) play_sample(bite,30,128,1000,false);
			if (foodType == BERRY) bushFood[index].foods -= 1;
			else if (foodType == APPLE) appleTreeFood[index].foods -= 1;
			if (foodMeter < 70) foodMeter += 30;
			else I.food++;
			effood--;	
		}
		
		void searchForFood(){
			findClosestFood();
			walk(xf,yf);
			if (arrived){
				eatOrWhatever();
				arrived = false;
			}
		}
		
		void findClosestWood(){
			int MinD = 999999;
			for (int i=0;i<=woodNumber;i++){
				if (distance(holz[i].x,x,holz[i].y,y) < MinD && !holz[i].cut){
					MinD = distance(holz[i].x,x,holz[i].y,y);
					xf = holz[i].x;
					yf = holz[i].y + 28;
					index = i;
				}	
			}
		}
		
		void chop(){
			if (distance(x,charX,y,charY) <= ScreenWidth/2) play_sample(woodcut,30,128,1000,false);
			holz[index].cut = true;
			I.wood += 1;
		}
		
		void searchForWood(){
			findClosestWood();
			walk(xf,yf);
			if (arrived){
				chop();
				arrived = false;
			}
		}
		
		void findSuitablePlaceForHouse(){
			int possibleHouseLocationX = rand()%BufferWidth;
			int possibleHouseLocationY = rand()%BufferHeight;
			if (mapMatrix[possibleHouseLocationY/64][possibleHouseLocationX/64] == 0){
				xf = possibleHouseLocationX;
				yf = possibleHouseLocationY;
				houseX = possibleHouseLocationX;
				houseY = possibleHouseLocationY;
				suitablePlaceFound = true;
			}
		}
		
		void buildHouse(){
			if (!suitablePlaceFound) findSuitablePlaceForHouse();
			walk(houseX,houseY);
			if (arrived){
				if (distance(x,charX,y,charY) <= ScreenWidth/2) play_sample(hammer,30,128,1000,false);
				mapMatrix[y/64][x/64] = 30;
				hasHouse = true;
				houseX = x;
				houseY = y;
				arrived = false;
			}
		}
		
		bool fire(){
			return doFire;
		}
		
		DIRECTION getDir(){
			return dir;
		}
		
		bool hasAGun(){
			if (I.gun > 0) return true;
			else return false;
		}
		
		void attackPlayer(){
			doFire = false;
			
			if (x > charX) dir = LEFT;
			else dir = RIGHT;
			
			if (charY-36 >= y+speed) y += speed;
			else if (charY-36 <= y-speed) y -=speed;
			else if (time(0) - lastFired > 1) {
				doFire = true;
				lastFired = time(0);
			}
		}
		
		void doStuff(){
			if (HP > 0){
				if (mapMatrix[y/64][x/64] == 1) speed = 1;
				else speed = 4;
				foodMeter -= 0.05;
				if (foodMeter <= 10 && !sleep && I.food == 0){
					HP -= 0.05;
					searchForFood();
					suitablePlaceFound = false;
				}
				else if (foodMeter <=10 && I.food > 0){
					HP -= 0.05;
					I.food--;
					foodMeter += 30;
				}
				if (relationToPlayer == -1 && distance(charX,x,charY,y) < ScreenWidth/2 && !sleep && !playerIsDead){
					attackPlayer();
				}
				else if (!hasHouse){
					if (I.wood < 6){
						searchForWood();
					}else{
					buildHouse();
				}
				}else if (HP < 100){
					walk(houseX,houseY);
					if (arrived){
						sleep = true;
						arrived = false;
					}
				}
				else if (I.food <= 5){
					searchForFood();
				}
				else walk(houseX,houseY);
				if (!sleep) draw();
				else{
					HP += 0.05;
					textout_ex(buffer,font,"ZZzz..",x,y,White,-1);
					if (HP >= 100) sleep = false;
				}
			}else draw_sprite(buffer,dead,x,y);
		}
};
//================END=OF=HUMAN===================

//==============INIT=OF=OBJECTS==================
animal o[50];
human people1[10];
human people2[10];
human Karcsi;
//============================================

class projectile{
	private:
		int x,y;
		short int speed;
		int damage;
		DIRECTION dir;
		BITMAP *pic;
		bool destroyed;
		short int type;
		int index;
	public:
		//~projectile(){destroy_bitmap(pic);}
		
		void initialize(int iksz,int ipszilon,DIRECTION ir,short int szpid,int demedzs,BITMAP *pik,short int tajp,int bulInd){
			x = iksz;
			y = ipszilon;
			speed = szpid;
			damage = demedzs;
			pic = pik;
			dir = ir;
			destroyed = false;
			type = tajp;
			index = bulInd;
		}
		
		int getIndex(){
			return index;
		}
		
		void moveProjectile(){
			if (dir == UP) y -= speed;
			else if (dir == DOWN) y += speed;
			else if (dir == LEFT){
				
				for(int i=0;i<animalNumber;i++){
					if (o[i].getY() + o[i].getPicHeight() >= y && o[i].getY() - pic->h <= y && x - speed <= o[i].getX()  && x > o[i].getX() && o[i].getHP() > 0){
						o[i].inflictDamage(damage);
						destroyed = true;
					}
				}
				
				if (!destroyed) for(int i=0;i<people1Number;i++){
					if (people1[i].getY() + people1[i].getPicHeight() >= y && people1[i].getY() - pic->h <= y && x - speed <= people1[i].getX()  && x > people1[i].getX() && people1[i].getHP() > 0){
						people1[i].inflictDamage(damage);
						if (index == 0) people1[i].setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				if (!destroyed) for(int i=0;i<people2Number;i++){
					if (people2[i].getY() + people2[i].getPicHeight() >= y && people2[i].getY() - pic->h <= y && x - speed <= people2[i].getX()  && x > people2[i].getX() && people2[i].getHP() > 0){
						people2[i].inflictDamage(damage);
						if (index == 0) people2[i].setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				
				if (!destroyed){
					if (charY > y && charY - 36 < y && ((x > charX-10 && x < charX+10) || (x > charX+10 && x-speed <charX-10)) && HP > 0){
						HP -= damage;
						destroyed = true;
					}
				}
				
				if (!destroyed){
					if (Karcsi.getY() + Karcsi.getPicHeight() >= y && Karcsi.getY() - pic->h <= y && x - speed <= Karcsi.getX()  && x > Karcsi.getX() && Karcsi.getHP() > 0){
						Karcsi.inflictDamage(damage);
						if (index == 0) Karcsi.setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				
				if (!destroyed) x -= speed;
			}
			else if (dir == RIGHT){
				
				for (int i=0;i<animalNumber;i++){
					if (o[i].getY() + o[i].getPicHeight() >= y && o[i].getY() - pic->h <= y && x + speed >= o[i].getX() && x < o[i].getX() && o[i].getHP() > 0){
						o[i].inflictDamage(damage);
						destroyed = true;
					}
				}
				
				if (!destroyed) for (int i=0;i<people1Number;i++){
					if (people1[i].getY() + people1[i].getPicHeight() >= y && people1[i].getY() - pic->h <= y && x + speed >= people1[i].getX() && x < people1[i].getX() && people1[i].getHP() > 0){
						people1[i].inflictDamage(damage);
						if (index == 0) people1[i].setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				if (!destroyed) for (int i=0;i<people2Number;i++){
					if (people2[i].getY() + people2[i].getPicHeight() >= y && people2[i].getY() - pic->h <= y && x + speed >= people2[i].getX() && x <= people2[i].getX() && people2[i].getHP() > 0){
						people2[i].inflictDamage(damage);
						if (index == 0) people2[i].setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				
				if (!destroyed){
					if (charY > y && charY - 36 < y && ((x > charX-10 && x < charX+10) || (x < charX-10 && x+speed > charX+10)) && HP > 0){
						HP -= damage;
						destroyed = true;
					}
				}
				
				if (!destroyed){
					if (Karcsi.getY() + Karcsi.getPicHeight() >= y && Karcsi.getY() - pic->h <= y && x + speed >= Karcsi.getX() && x <= Karcsi.getX() && Karcsi.getHP() > 0){
						Karcsi.inflictDamage(damage);
						if (index == 0) Karcsi.setRelationToPlayer(-1);
						destroyed = true;
					}
				}
				if (!destroyed) x += speed;
			}
		}
		
		void draw(){
			if (type == 0){
				if (dir == RIGHT) masked_blit(pic,buffer,0,0,x,y,39,11);
				else if (dir == LEFT) masked_blit(pic,buffer,0,11,x,y,39,11);
			}else if (type == 1) draw_sprite(buffer,pic,x,y);
		}
		
		bool isDestroyed(){return destroyed;}
		
		void doStuff(){
			if (!destroyed){
				moveProjectile();
				if (mapMatrix[y/64][x/64] == 0 || mapMatrix[y/64][x/64] == 1 || mapMatrix[y/64][x/64] == 22 || (mapMatrix[y/64][x/64] >=10 && mapMatrix[y/64][x/64] <= 14 )) draw();
				else{
					//if (mapMatrix[y/64][x/64] >= 2 && mapMatrix[y/64][x/64] <= 8) play_sample(arrowHitWood,128,128,1000,false);
					play_sample(bulletHit,128,128,1000,false);
					destroyed = true;
				}
			}
		}
};
//==============END=OF=PROJECTILE=============

void loadBitmapsAndSound(){
	grass = load_bitmap("GFX/Textures/grass.bmp",NULL);
	water = load_bitmap("GFX/Textures/water.bmp",NULL);
	def   = load_bitmap("GFX/Textures/default.bmp",NULL);
	
	tree         = load_bitmap("GFX/Objects/Map/tree.bmp",NULL);
	atree        = load_bitmap("GFX/Objects/Map/apple_tree.bmp",NULL);
	cuttree      = load_bitmap("GFX/Objects/Map/cut_tree.bmp",NULL);
	rock         = load_bitmap("GFX/Objects/Map/rock1.bmp",NULL);
	bush         = load_bitmap("GFX/Objects/Map/bush.bmp",NULL);
	ironOreStone = load_bitmap("GFX/Objects/Map/ironOreStone.bmp",NULL);
	
	chr        = load_bitmap("GFX/Objects/Characters/char.bmp",NULL);
	O          = load_bitmap("GFX/Objects/Characters/o.bmp",NULL);
	personPic1 = load_bitmap("GFX/Objects/Characters/human.bmp",NULL);
	personPic2 = load_bitmap("GFX/Objects/Characters/human2.bmp",NULL);
	karcsiPic  = load_bitmap("GFX/Objects/Characters/karcsiPic.bmp",NULL);
	
	dead     = load_bitmap("GFX/Objects/Other/dead.bmp",NULL);
	arrow    = load_bitmap("GFX/Objects/Other/arrow.bmp",NULL);
	bullet   = load_bitmap("GFX/Objects/Other/bullet.bmp",NULL);
	
	foodPic       = load_bitmap("GFX/Objects/Resources/apple.bmp",NULL);
	woodPic       = load_bitmap("GFX/Objects/Resources/wood.bmp",NULL);
	stonePic      = load_bitmap("GFX/Objects/Resources/Stone.bmp",NULL);
	bowPic        = load_bitmap("GFX/Objects/Resources/bow.bmp",NULL);
	gunPic        = load_bitmap("GFX/Objects/Resources/gun.bmp",NULL);
	meatPic       = load_bitmap("GFX/Objects/Resources/hus.bmp",NULL);
	arrowPic      = load_bitmap("GFX/Objects/Resources/arrowPic.bmp",NULL);
	kugelPic      = load_bitmap("GFX/Objects/Resources/kugel.bmp",NULL);
	ironBarPic    = load_bitmap("GFX/Objects/Resources/ironBar.bmp",NULL); 
	ironOrePic    = load_bitmap("GFX/Objects/Resources/ironOre.bmp",NULL);
	cookedMeatPic = load_bitmap("GFX/Objects/Resources/sultHus.bmp",NULL);
	
	house  = load_bitmap("GFX/Objects/Buildings/house.bmp",NULL);
	smithy = load_bitmap("GFX/Objects/Buildings/smithy.bmp",NULL); 
	
	

	buffer = create_bitmap(BufferWidth,BufferHeight);
	invScreen = create_bitmap(InvWidth,InvHeight);
	buildMenu = create_bitmap(356,124);
	craftMenu = create_bitmap(440,230);
	houseMenu = create_bitmap(104,230);
	
	bite      = load_sample("SFX/Sounds/bite.wav");
	pick      = load_sample("SFX/Sounds/pick.wav");
	woodcut   = load_sample("SFX/Sounds/saw.wav");
	hammer    = load_sample("SFX/Sounds/hammer.wav");
	gunFire   = load_sample("SFX/Sounds/gunFire.wav");
	bulletHit = load_sample("SFX/Sounds/bulletHit.wav");
	bowFire   = load_sample("SFX/Sounds/bowFire.wav");
	hitO      = load_sample("SFX/Sounds/hitO.wav");
	mine1     = load_sample("SFX/Sounds/mine1.wav");
	mine2     = load_sample("SFX/Sounds/mine4.wav");
	
	musica = load_midi("SFX/Music/musica.mid");
	backMusic = load_sample("SFX/Music/musica.wav");
	
	// Slots setup
	int I,J;
	for (int i=0;i<12;i++){
		if (i < 4){I=i+1; J=1;}
		if (i > 3 && i < 8){I=i-3; J=2;}
		if (i > 7){I=i-7; J=3;}
		
		slot[i].x = I * 20 + (I-1) * 64;
		slot[i].y = (J+1) * 20 + (J-1) * 64;
		slot[i].pic = def;
		slot[i].nr = 0;
		slot[i].has = "EMPTY";
	}
	areBitmapsAndSoundsLoaded = true;
}END_OF_FUNCTION();



void initializeMatrix(){
	srand(time(0));
	for(int mHeight=0;mHeight<MatrixHeight;mHeight++){
		for(int mWidth=0;mWidth<MatrixWidth;mWidth++){
			// Fu elhelyezese
			mapMatrix[mHeight][mWidth] = 0; 
			
			// Viz elhelyezese
			if ((mapMatrix[mHeight-1][mWidth] == 1 || mapMatrix[mHeight+1][mWidth] == 1 || mapMatrix[mHeight][mWidth-1] == 1 || mapMatrix[mHeight][mWidth+1] == 1) && (rand()%3 == 0 || rand()%3 ==1)){
				 mapMatrix[mHeight][mWidth] = 1;
			}
			else if (rand()%100 == 0 || rand()%100==1) mapMatrix[mHeight][mWidth] = 1;
			
			// Fak elhelyezese
			if (mapMatrix[mHeight][mWidth] == 0){
				if (rand()%20 == 0){
					int numberThatRandomizesWoodProcentige = rand()%3;
					if (numberThatRandomizesWoodProcentige == 0 || numberThatRandomizesWoodProcentige == 1){
						mapMatrix[mHeight][mWidth] = 2;
						woodNumber++;
						holz[woodNumber-1].Mx = mWidth;
						holz[woodNumber-1].My = mHeight;
						holz[woodNumber-1].x = mWidth*def->w;
						holz[woodNumber-1].y = mHeight*def->h;
						holz[woodNumber-1].HP = 100;
						holz[woodNumber-1].cut = false;
					}
					else{
						mapMatrix[mHeight][mWidth] = 4;
						appleTreeFoodNumber++; effood++;
						appleTreeFood[appleTreeFoodNumber-1].x = mWidth*def->w;
						appleTreeFood[appleTreeFoodNumber-1].Mx = mWidth;
						appleTreeFood[appleTreeFoodNumber-1].y = mHeight*def->h;
						appleTreeFood[appleTreeFoodNumber-1].My = mHeight;
						appleTreeFood[appleTreeFoodNumber-1].foods = 1;
						appleTreeFood[appleTreeFoodNumber-1].time = 0.0;
					}
				}
				// Ko es Vaserc elhelyezese
				else if (rand()%40 == 1){
					if (rand()%2){
						mapMatrix[mHeight][mWidth] = 9;
						stoneNumber++;
						stein[stoneNumber-1].Mx = mWidth;
						stein[stoneNumber-1].My = mHeight;
						stein[stoneNumber-1].stones = 15;
					}
					else {
						mapMatrix[mHeight][mWidth] = 15;
						ironOreNumber++;
						eisenErz[ironOreNumber-1].Mx = mWidth;
						eisenErz[ironOreNumber-1].My = mHeight;
						eisenErz[ironOreNumber-1].stones = 5;
					}
				}
				// Bokor
				else if (rand()%30 == 2){
					mapMatrix[mHeight][mWidth] = 11;
					bushFoodNumber++; effood++;
					bushFood[bushFoodNumber-1].x = mWidth*def->w;
					bushFood[bushFoodNumber-1].Mx = mWidth;
					bushFood[bushFoodNumber-1].y = mHeight*def->h;
					bushFood[bushFoodNumber-1].My = mHeight;
					bushFood[bushFoodNumber-1].foods = 1;
					bushFood[bushFoodNumber-1].time = 0.0;
				}
			}
		}
	}
}END_OF_FUNCTION();

void initializeAllegro(){
	allegro_init();
	install_mouse();
	install_keyboard();
	set_window_title("Prosomo");
	set_color_depth(32);

	install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,"A");
	
}END_OF_FUNCTION();

void deinitializeAllegroAndStuff(){
	destroy_bitmap(buffer);
	destroy_bitmap(grass);
	destroy_bitmap(water);
	destroy_bitmap(def);
	destroy_bitmap(tree);
	destroy_bitmap(chr);
	destroy_bitmap(O);
	destroy_bitmap(dead);
	destroy_bitmap(invScreen);
	destroy_bitmap(foodPic);
	destroy_bitmap(woodPic);
	destroy_bitmap(cuttree);
	destroy_bitmap(rock);
	destroy_bitmap(bush);
	destroy_bitmap(stonePic);
	destroy_bitmap(bowPic);
	destroy_bitmap(gunPic);
	destroy_bitmap(house);
	destroy_bitmap(personPic1);
	destroy_bitmap(arrow);
	destroy_bitmap(personPic2);
	destroy_bitmap(bullet);
	destroy_bitmap(meatPic);
	destroy_bitmap(karcsiPic);
	destroy_bitmap(kugelPic);
	destroy_bitmap(arrowPic);
	destroy_bitmap(ironOreStone);
	destroy_bitmap(ironBarPic);
	destroy_bitmap(ironOrePic);
	destroy_bitmap(smithy);
	destroy_bitmap(buildMenu);
	destroy_bitmap(craftMenu);
	destroy_bitmap(cookedMeatPic);
	destroy_bitmap(houseMenu);
	destroy_midi(musica);
	destroy_sample(bite);
	destroy_sample(pick);
	destroy_sample(woodcut);
	destroy_sample(hammer);
	destroy_sample(gunFire);
	destroy_sample(bulletHit);
	destroy_sample(bowFire);
	destroy_sample(hitO);
	destroy_sample(mine1);
	destroy_sample(mine2);
	allegro_exit();
}END_OF_FUNCTION();

void drawMapGeoToBuffer(int currentX,int currentY){
	
	// Almafak:
	for (int i=0;i<appleTreeFoodNumber;i++){
		if (appleTreeFood[i].foods == 0) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 3;
		else if (appleTreeFood[i].foods == 1) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 4;
		else if (appleTreeFood[i].foods == 2) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 5;
		else if (appleTreeFood[i].foods == 3) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 6;
		else if (appleTreeFood[i].foods == 4) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 7;
		else if (appleTreeFood[i].foods == 5) mapMatrix[appleTreeFood[i].My][appleTreeFood[i].Mx] = 8;
		if (appleTreeFood[i].foods < 5) appleTreeFood[i].time += 0.5; // Az almak novekedesi sebessege
		if (appleTreeFood[i].time >= 1000){
			appleTreeFood[i].foods += 1;
			appleTreeFood[i].time = 0;
			effood++;
		}
	}
	
	//Bokrok
	for (int i=0;i<bushFoodNumber;i++){
		if (bushFood[i].foods == 0) mapMatrix[bushFood[i].My][bushFood[i].Mx] = 10;
		else if (bushFood[i].foods == 1) mapMatrix[bushFood[i].My][bushFood[i].Mx] = 11;
		else if (bushFood[i].foods == 2) mapMatrix[bushFood[i].My][bushFood[i].Mx] = 12;
		else if (bushFood[i].foods == 3) mapMatrix[bushFood[i].My][bushFood[i].Mx] = 13;
		else if (bushFood[i].foods == 4) mapMatrix[bushFood[i].My][bushFood[i].Mx] = 14;
		if (bushFood[i].foods < 4) bushFood[i].time += 0.8; // A bogyok novekedesi sebesdege
		if (bushFood[i].time >= 1000){
			bushFood[i].foods += 1;
			bushFood[i].time = 0;
			effood++;
		}
	}
	
	//Fak
	for (int i=0;i<woodNumber;i++){
		if (holz[i].cut){
			mapMatrix[holz[i].My][holz[i].Mx] = 22;
			holz[i].time += 0.5;
			if (holz[i].time >= 1000){
				holz[i].cut = false;
				holz[i].time = 0.0;
			}
		}else if (!holz[i].cut) mapMatrix[holz[i].My][holz[i].Mx] = 2;
	}
	
	for (int i=currentY/64;i<(currentY+ScreenHeight)/64+1;i++){
		for (int j=currentX/64;j<(currentX+ScreenWidth)/64+1;j++){
			if (mapMatrix[i][j] == 0) draw_sprite(buffer,grass,j*def->w,i*def->h);
			else if (mapMatrix[i][j] == 1) draw_sprite(buffer,water,j*def->w,i*def->h);
			else if (mapMatrix[i][j] == 2){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,tree,j*def->w,i*def->h);
			}
			else if (mapMatrix[i][j] == 22){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,cuttree,j*def->w,i*def->h);
			}
			else if (mapMatrix[i][j] >= 3 && mapMatrix[i][j] <= 8){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				if (mapMatrix[i][j] == 3) masked_blit(atree,buffer,0,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 4) masked_blit(atree,buffer,64,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 5) masked_blit(atree,buffer,64*2,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 6) masked_blit(atree,buffer,64*3,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 7) masked_blit(atree,buffer,64*4,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 8) masked_blit(atree,buffer,64*5,0,j*def->w,i*def->h,64,64);
			}
			else if (mapMatrix[i][j] == 9){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,rock,j*def->w,i*def->h);
			}
			else if (mapMatrix[i][j] == 15){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,ironOreStone,j*def->w,i*def->h);
			}
			else if (mapMatrix[i][j] >= 10 && mapMatrix[i][j] <= 14){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				if (mapMatrix[i][j] == 10) masked_blit(bush,buffer,0,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 11) masked_blit(bush,buffer,64,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 12) masked_blit(bush,buffer,64*2,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 13) masked_blit(bush,buffer,64*3,0,j*def->w,i*def->h,64,64);
				else if (mapMatrix[i][j] == 14) masked_blit(bush,buffer,64*4,0,j*def->w,i*def->h,64,64); 
			}
			else if (mapMatrix[i][j] == 30){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,house,j*def->w,i*def->h);
			}
			else if (mapMatrix[i][j] == 31){
				draw_sprite(buffer,grass,j*def->w,i*def->h);
				draw_sprite(buffer,smithy,j*def->w,i*def->h);
			}
		}
	}
}END_OF_FUNCTION();


bool keyrel(int k){
    static bool initialized = false;
    static bool keyp[KEY_MAX];
 
    if(!initialized){
        // Set the keyp (key pressed) flags to false
        for(int i = 0; i < KEY_MAX; i++) keyp[i] = false;
        initialized = true;
    }
    // Now for the checking
    // Check if the key was pressed
    if(key[k] && !keyp[k]){
        // Set the flag and return
        keyp[k] = true;
        return false;
    }
    else if(!key[k] && keyp[k]){
        // The key was released
        keyp[k] = false;
        return true;
    }
    // Nothing happened?
    return false;
}

void game(){
	
	int currentX = 0;
	int currentY = 0;
	bool exitGame = false;
	short int speed = 1;
	
	DIRECTION direction; // Milyen iranyba nez a karakter
	int charPicCol = 0; // Hanyadik kep az oszlopbol
	bool walk; 

	direction = RIGHT;
	double foodMeter = 100; // 100
	bool sleep = false;
	
	// Build Map
	initializeMatrix();
	
	// 	Inventory
	INVENTORY inv;
	inv.food    = 0;
	inv.wood    = 10;
	inv.stone   = 0;
	inv.bow     = 1;
	inv.gun     = 1;
	inv.meat    = 0;
	inv.kugel   = 100;
	inv.arrow   = 50;
	inv.iron    = 0;
	inv.ironOre = 0;
	inv.stake   = 1;
	inv.foodTaken = false;
	inv.woodTaken = false;
	inv.stoneTaken = false;
	inv.bowTaken = false;
	inv.gunTaken = false;
	inv.meatTaken = false;
	inv.kugelTaken = false;
	inv.arrowTaken = false;
	inv.ironTaken = false;
	inv.ironOreTaken = false;
	inv.stakeTaken = false;
	bool inventoryOn = false;
	char R[10];
	
	//Create Animals
	for (int i=0;i<animalNumber;i++) o[i].initialize(rand()%BufferWidth,rand()%BufferHeight,O);
	
	//Create Human
	int playerBulletIndex = 0;
	int justAnIndexVariable = 2;
	
	Karcsi.initialize(64,64,karcsiPic,1);
	Karcsi.setKarcsi(true);
	
	for (int i=0;i<people1Number;i++){
		people1[i].initialize(rand()%BufferWidth,rand()%BufferHeight,personPic1,justAnIndexVariable);
		justAnIndexVariable++;
	}
	for (int i=0;i<people2Number;i++){
		people2[i].initialize(rand()%BufferWidth,rand()%BufferHeight,personPic2,justAnIndexVariable);
		justAnIndexVariable++;
	}
	
	//play_midi(musica,false);
	//play_sample(backMusic,64,128,1000,true);
	
	//Other
	bool canPressSpace = true;
	bool canPressF = true;
	projectile projectile[100];
	int projectileNumber = 0;
	int fireWeapon = 0; // Milyen fegyverrel lo
	bool drawBuildMenu = false;
	bool showCraftMenu = false;
	bool canPlayerMove = true;
	bool canPressNumber1 = true;
	bool canPressNumber2 = true;
	bool canPressNumber3 = true;
	bool canPressNumber4 = true;
	bool canPressNumber5 = true;
	bool showHouseMenu = false;
	
	while (!exitGame){
		// A karakterrel kapcsolatos dolgok
		walk = false;
		inventoryOn = false;
		drawBuildMenu = false;
		if (keyrel(KEY_SPACE)) canPressSpace = true;
		if (keyrel(KEY_F)) canPressF = true;
		if (keyrel(KEY_1)) canPressNumber1 = true;
		if (keyrel(KEY_2)) canPressNumber2 = true;
		if (keyrel(KEY_3)) canPressNumber3 = true;
		if (keyrel(KEY_4)) canPressNumber4 = true;
		if (keyrel(KEY_5)) canPressNumber5 = true;
			
		if (foodMeter > 0) foodMeter -= 0.05; 
		else if (HP > 0){HP -= 0.1; foodMeter = 0.0;}
		if (foodMeter <= 30 && inv.stake > 0){
			//eatingSound
			inv.stake--;
			foodMeter += 100;
		}
		else if (foodMeter <= 30 && inv.food > 0){
			play_sample(bite,128,128,1000,false);
			inv.food--;
			foodMeter += 30;
		} 
		
		if (mapMatrix[charY/64][charX/64] == 1) speed = 1;
		else speed = 10;
		
		if (keyrel(KEY_ESC)) exitGame = true;
		if (HP >0){
			if ((key[KEY_W] || key[KEY_UP]) && !sleep && canPlayerMove){
				walk = true;
				charY-=speed;
				if (charY <= 0) charY = 0;
				fireWeapon = 0;
			}
			else if ((key[KEY_S] || key[KEY_DOWN]) && !sleep && canPlayerMove){
				walk = true;
				charY+=speed;
				if (charY >= BufferWidth) charY = BufferWidth;
				fireWeapon = 0;
			}
			if ((key[KEY_D] || key[KEY_RIGHT]) && !sleep && canPlayerMove){
				walk = true;			
				charX+=speed;
				if (charX >= BufferHeight) charX = BufferHeight;
				direction = RIGHT;
				fireWeapon = 0;
			}
			else if ((key[KEY_A] || key[KEY_LEFT]) && !sleep && canPlayerMove){
				walk = true;
				charX-=speed;
				if (charX <= 0) charX = 0;
				direction = LEFT;
				fireWeapon = 0;
			}
			else if (key[KEY_SPACE] && canPressSpace){
				canPressSpace = false;
				
				// Hus szedes
				for(int i=0;i<animalNumber;i++){
					if (o[i].getHP() <= 0){
						if (o[i].getY() + o[i].getPicHeight() >= charY && o[i].getY() <= charY && o[i].getX() <= charX && o[i].getX() + o[i].getPicWidth() >= charX){
							inv.meat++;
							animalNumber--;
							for (int j=i;j<animalNumber;j++) o[i] = o[i+1]; 
						}
					}
				}
				
				// Favagas
				if (mapMatrix[charY/64][charX/64] == 2){
					for (int i=0;i<woodNumber;i++){
						if (holz[i].Mx == charX/64 && holz[i].My == charY/64) {
							play_sample(woodcut,128,128,1000,false);
							holz[i].cut = true;
							inv.wood++;
						}
					}
				}
				// Almaszedes
				else if (mapMatrix[charY/64][charX/64] >= 3 && mapMatrix[charY/64][charX/64] <= 8){
					for (int i=0;i<appleTreeFoodNumber;i++){
						if (appleTreeFood[i].Mx == charX/64 && appleTreeFood[i].My == charY/64){
							if (appleTreeFood[i].foods > 0){
								if (foodMeter < 100){
									play_sample(bite,128,128,1000,false);
									appleTreeFood[i].foods--;
									foodMeter += 30;
									effood--;
								}
								else{
									play_sample(pick,128,128,1000,false);
									appleTreeFood[i].foods--;
									inv.food++;
									effood--;
								}
							} 
						}
					} 
				}
				// Bogyoszedes
				else if (mapMatrix[charY/64][charX/64] >= 10 && mapMatrix[charY/64][charX/64] <= 14){
					for (int i=0;i<bushFoodNumber;i++){
						if (bushFood[i].Mx == charX/64 && bushFood[i].My == charY/64){
							if (bushFood[i].foods){
								if (foodMeter < 100){
									play_sample(bite,128,128,1000,false);
									bushFood[i].foods--;
									foodMeter += 10;
									effood--;
								}
								else{
									play_sample(pick,128,128,1000,false);
									bushFood[i].foods--;
									inv.food++;
									effood--;
								}
							} 
						}
					} 
				} 
				// Ko
				else if (mapMatrix[charY/64][charX/64] == 9){
					for (int i=0;i<=stoneNumber;i++){
						if (stein[i].Mx == charX/64 && stein[i].My == charY/64){
							play_sample(mine2,128,128,1000,false);
							stein[i].stones--;
							inv.stone++;
							if (stein[i].stones == 0) mapMatrix[charY/64][charX/64] = 0;
						}
					}
				}
				//Vaserc
				else if (mapMatrix[charY/64][charX/64] == 15){
					for (int i=0;i<=ironOreNumber;i++){
						if (eisenErz[i].Mx == charX/64 && eisenErz[i].My == charY/64){
							play_sample(mine1,128,128,1000,false);
							eisenErz[i].stones--;
							inv.ironOre++;
							if (eisenErz[i].stones == 0) mapMatrix[charY/64][charX/64] = 0;
						}
					}
				}
				// Lakohaz
				else if (mapMatrix[charY/64][charX/64] == 30 && !sleep){
					sleep = true;
					showHouseMenu = true;
				}
				else if (mapMatrix[charY/64][charX/64] == 30 && sleep){
					sleep = false;
					showHouseMenu = false;
				}
				// Smithy
				else if (mapMatrix[charY/64][charX/64] == 31 && !showCraftMenu){
					showCraftMenu = true;
					canPlayerMove = false;
				}
				else if (mapMatrix[charY/64][charX/64] == 31 && showCraftMenu){
					showCraftMenu = false;
					canPlayerMove = true;
				}
			}
			else if (key[KEY_E]){
				
				//Inventory Refresh
				inventoryOn = true;
				for (int i=0;i<12;i++){
					if (slot[i].has == "EMPTY"){
						if (inv.food > 0 && !inv.foodTaken){
							slot[i].pic = foodPic;
							slot[i].nr = inv.food;
							slot[i].has = "FOOD";
							inv.foodTaken = true;
						}
						else if (inv.wood > 0 && !inv.woodTaken){
							slot[i].pic = woodPic;
							slot[i].nr = inv.wood;
							slot[i].has = "WOOD";
							inv.woodTaken = true;
						}
						else if (inv.stone > 0 && !inv.stoneTaken){
							slot[i].pic = stonePic;
							slot[i].nr = inv.stone;
							slot[i].has = "STONE";
							inv.stoneTaken = true;
						}
						else if (inv.bow > 0 && !inv.bowTaken){
							slot[i].pic = bowPic;
							slot[i].nr = inv.bow;
							slot[i].has = "BOW";
							inv.bowTaken = true;
						}
						else if (inv.gun > 0 && !inv.gunTaken){
							slot[i].pic = gunPic;
							slot[i].nr = inv.gun;
							slot[i].has = "GUN";
							inv.gunTaken = true;
						}
						else if (inv.meat > 0 && !inv.meatTaken){
							slot[i].pic = meatPic;
							slot[i].nr = inv.meat;
							slot[i].has = "MEAT";
							inv.meatTaken = true;
						}
						else if (inv.kugel > 0 && !inv.kugelTaken){
							slot[i].pic = kugelPic;
							slot[i].nr = inv.kugel;
							slot[i].has = "KUGEL";
							inv.kugelTaken = true;
						}
						else if (inv.arrow > 0 && !inv.arrowTaken){
							slot[i].pic = arrowPic;
							slot[i].nr = inv.arrow;
							slot[i].has = "ARROW";
							inv.arrowTaken = true;
						}
						else if (inv.iron > 0 && !inv.ironTaken){
							slot[i].pic = ironBarPic;
							slot[i].nr = inv.iron;
							slot[i].has = "IRON";
							inv.ironTaken = true;
						}
						else if (inv.ironOre > 0 && !inv.ironOreTaken){
							slot[i].pic = ironOrePic;
							slot[i].nr = inv.ironOre;
							slot[i].has = "ORE";
							inv.ironOreTaken = true;
						}
						else if (inv.stake > 0 && !inv.stakeTaken){
							slot[i].pic = cookedMeatPic;
							slot[i].nr = inv.stake;
							slot[i].has = "STAKE";
							inv.stakeTaken = true;
						}
					} 
					else if (slot[i].has == "FOOD" && inv.food != slot[i].nr){
						if (inv.food == 0){
							inv.foodTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.food;
					}
					else if (slot[i].has == "WOOD" && inv.wood != slot[i].nr){
						if (inv.wood == 0){
							inv.woodTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.wood;
					}
					else if (slot[i].has == "STONE" && inv.stone != slot[i].nr){
						if (inv.stone == 0){
							inv.stoneTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.stone;
					}
					else if (slot[i].has == "BOW" && inv.bow != slot[i].nr){
						if (inv.bow == 0){
							inv.bowTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.bow;
					}
					else if (slot[i].has == "GUN" && inv.gun != slot[i].nr){
						if (inv.gun == 0){
							inv.gunTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.gun;
					}
					else if (slot[i].has == "MEAT" && inv.meat != slot[i].nr){
						if (inv.meat == 0){
							inv.meatTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.meat;
					}
					else if (slot[i].has == "KUGEL" && inv.kugel != slot[i].nr){
						if (inv.kugel == 0){
							inv.kugelTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.kugel;
					}
					else if (slot[i].has == "ARROW" && inv.arrow != slot[i].nr){
						if (inv.arrow == 0){
							inv.arrowTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.arrow;
					}
					else if (slot[i].has == "IRON" && inv.iron != slot[i].nr){
						if (inv.iron == 0){
							inv.ironTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.iron;
					}
					else if (slot[i].has == "ORE" && inv.ironOre != slot[i].nr){
						if (inv.ironOre == 0){
							inv.ironOreTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.ironOre;
					}
					else if (slot[i].has == "STAKE" && inv.stake != slot[i].nr){
						if (inv.stake == 0){
							inv.stakeTaken = false;
							slot[i].pic = def;
							slot[i].nr = 0;
							slot[i].has = "EMPTY";
						}else slot[i].nr = inv.stake;
					}
				}
			}
			else if (key[KEY_TAB]){ HP += 100;}
			else  if (key[KEY_B]){
				drawBuildMenu = true;
				
				if (key[KEY_1]){
					if (mapMatrix[charY/64][charX/64] == 0 && inv.wood >= 6){
						play_sample(hammer,128,128,1000,false);
						mapMatrix[charY/64][charX/64] = 30;
						inv.wood -= 6;
					}
				}
				if (key[KEY_2]){
					if (mapMatrix[charY/64][charX/64] == 0 && inv.wood >= 6){
						play_sample(hammer,128,128,1000,false);
						mapMatrix[charY/64][charX/64] = 31;
						inv.wood -= 6;
					}
				}
			}
			else if (key[KEY_F] && canPressF){
				canPressF = false;
				projectileNumber++;
				if (inv.gun > 0 && inv.kugel > 0){
					if (direction == LEFT) projectile[projectileNumber-1].initialize(charX-10,charY-20,direction,50,100-rand()%50,bullet,1,playerBulletIndex);
					else if (direction == RIGHT) projectile[projectileNumber-1].initialize(charX+17,charY-20,direction,50,100-rand()%50,bullet,1,playerBulletIndex);
					play_sample(gunFire,128,128,1000,false);
					inv.kugel--;
					fireWeapon = 1;
				}
				else if (inv.bow > 0 && inv.arrow > 0){
					if (direction == LEFT) projectile[projectileNumber-1].initialize(charX-10,charY-20,direction,20,rand()%80,arrow,0,playerBulletIndex);
					else if (direction == RIGHT) projectile[projectileNumber-1].initialize(charX+10,charY-20,direction,20,rand()%80,arrow,0,playerBulletIndex);
					play_sample(bowFire,128,128,1000,false);
					inv.arrow--;
					fireWeapon = 2;
				}
			}
			else if (showCraftMenu){
				if (key[KEY_1] && inv.ironOre > 0 && canPressNumber1){
					inv.ironOre--;
					inv.iron++;
					canPressNumber1 = false;
				}
				else if (key[KEY_2] && inv.iron > 0 && canPressNumber2){
					inv.iron--;
					inv.kugel += 3;
					canPressNumber2 = false;
				}
				else if (key[KEY_3] && inv.wood > 0 && canPressNumber3){
					inv.wood--;
					inv.arrow += 2;
					canPressNumber3 = false;
				}
				else if (key[KEY_4] && inv.wood >= 5 && canPressNumber4){
					inv.wood -= 5;
					inv.bow++;
					canPressNumber4 = false;
				}
				else if (key[KEY_5] && inv.iron >= 5 && canPressNumber5){
					inv.iron -= 5;
					inv.gun++;
					canPressNumber5 = false;
				}
			}
			else if (houseMenu){
				if (key[KEY_1] && inv.meat > 0 && canPressNumber1){
					inv.meat--;
					inv.stake++;
					canPressNumber1 = false;
				}
			}
		}
		
		if (sleep && HP < 100) HP += 0.1;
		
		// _______________CLEAR_BUFFER_______________
		clear_bitmap(buffer); 
		drawMapGeoToBuffer(currentX,currentY);
		
		//Animals
		for (int i=0;i<animalNumber;i++) o[i].doStuff();
		
		//Humans
		Karcsi.doStuff();
		if (Karcsi.getHP() > 0){
			if (Karcsi.fire()){
				projectileNumber++;
					if (Karcsi.hasAGun()){
						play_sample(gunFire,128,128,1000,false);
						if (Karcsi.getDir() == LEFT) projectile[projectileNumber-1].initialize(Karcsi.getX(),Karcsi.getY()+16,Karcsi.getDir(),50,100-rand()%50,bullet,1,Karcsi.getBulletIndex());
						else if (Karcsi.getDir() == RIGHT) projectile[projectileNumber-1].initialize(Karcsi.getX() + 20,Karcsi.getY()+16,Karcsi.getDir(),50,100-rand()%50,bullet,1,Karcsi.getBulletIndex());
					}
					else {
						play_sample(bowFire,128,128,1000,false);
						if (Karcsi.getDir() == LEFT) projectile[projectileNumber-1].initialize(Karcsi.getX(),Karcsi.getY()+16,Karcsi.getDir(),20,rand()%80,arrow,0,Karcsi.getBulletIndex());
						else if (Karcsi.getDir() == RIGHT) projectile[projectileNumber-1].initialize(Karcsi.getX() + 20,Karcsi.getY()+16,Karcsi.getDir(),20,rand()%80,arrow,0,Karcsi.getBulletIndex());
					}
			}
		}
		
		for (int i=0;i<5;i++){
			people1[i].doStuff();
			if (people1[i].getHP() > 0){
				if (people1[i].fire()){
					projectileNumber++;
					if (people1[i].hasAGun()){
						play_sample(gunFire,128,128,1000,false);
						if (people1[i].getDir() == LEFT) projectile[projectileNumber-1].initialize(people1[i].getX(),people1[i].getY()+16,people1[i].getDir(),50,100-rand()%50,bullet,1,people1[i].getBulletIndex());
						else if (people1[i].getDir() == RIGHT) projectile[projectileNumber-1].initialize(people1[i].getX() + 20,people1[i].getY()+16,people1[i].getDir(),50,100-rand()%50,bullet,1,people1[i].getBulletIndex());
					}
					else {
						play_sample(bowFire,128,128,1000,false);
						if (people1[i].getDir() == LEFT) projectile[projectileNumber-1].initialize(people1[i].getX(),people1[i].getY()+16,people1[i].getDir(),20,rand()%80,arrow,0,people1[i].getBulletIndex());
						else if (people1[i].getDir() == RIGHT) projectile[projectileNumber-1].initialize(people1[i].getX() + 20,people1[i].getY()+16,people1[i].getDir(),20,rand()%80,arrow,0,people1[i].getBulletIndex());
					}
				}
			}
		}
		for (int i=0;i<5;i++){
			people2[i].doStuff();
			if (people2[i].getHP() > 0){
				if (people2[i].fire()){
					projectileNumber++;
					if (people2[i].hasAGun()){
						play_sample(gunFire,128,128,1000,false);
						if (people2[i].getDir() == LEFT) projectile[projectileNumber-1].initialize(people2[i].getX(),people2[i].getY()+16,people2[i].getDir(),50,100,bullet,1,people2[i].getBulletIndex());
						else if (people2[i].getDir() == RIGHT) projectile[projectileNumber-1].initialize(people2[i].getX() + 20,people2[i].getY()+16,people2[i].getDir(),50,100,bullet,1,people2[i].getBulletIndex());
					}else {
						play_sample(bowFire,128,128,1000,false);
						if (people2[i].getDir() == LEFT) projectile[projectileNumber-1].initialize(people2[i].getX(),people2[i].getY()+16,people2[i].getDir(),20,rand()%80,arrow,0,people2[i].getBulletIndex());
						else if (people2[i].getDir() == RIGHT) projectile[projectileNumber-1].initialize(people2[i].getX() + 20,people2[i].getY()+16,people2[i].getDir(),20,rand()%80,arrow,0,people2[i].getBulletIndex());
					}
				}
			}
		}
		
		//Projectiles
		if (projectileNumber > 0){
			for (int i=0; i<projectileNumber;i++){
				projectile[i].doStuff();
				if (projectile[i].isDestroyed()){
					projectileNumber--;
					for (int j=i;j<projectileNumber;j++) projectile[i] = projectile[i+1];
				}
			}
		}
		
		// A karakter Jarasi es mas animacioja
		if (HP > 0 && !sleep){
			if (fireWeapon){
				if (fireWeapon == 1){
					if (direction == RIGHT) masked_blit(chr,buffer,0,108,charX-10,charY-36,30,36);
					else if (direction == LEFT) masked_blit(chr,buffer,30,108,charX-21,charY-36,30,36);
				}
				if (fireWeapon == 2){
					if (direction == RIGHT) masked_blit(chr,buffer,0,144,charX-10,charY-36,30,36);
					else if (direction == LEFT) masked_blit(chr,buffer,30,144,charX-21,charY-36,30,36);
				}
			}else{
				if (!walk){
					if (direction == RIGHT) masked_blit(chr,buffer,0,0,charX-10,charY-36,20,36);
					else if (direction == LEFT) masked_blit(chr,buffer,0,36,charX-10,charY-36,20,36);
				}
				else{
					if (direction == RIGHT){
						masked_blit(chr,buffer,charPicCol,0,charX-10,charY-36,20,36);
						if (charPicCol == 40) charPicCol = 0;
						else charPicCol+=20;
					}
					else if (direction == LEFT){
						masked_blit(chr,buffer,charPicCol,36,charX-10,charY-36,20,36);
						if (charPicCol == 40) charPicCol = 0;
						else charPicCol+=20;
					}
				}
			if (foodMeter <= 30.1) textout_ex(buffer,font,"EHES!",charX,charY-10,makecol(255,0,0),-1);
			}
		}
		else if (sleep && HP > 0) textout_ex(buffer,font,"zzZZ...",charX+20,charY,White,-1);
		else if (HP <= 0){
			draw_sprite(buffer,dead,charX-(dead->w/2),charY-dead->h);
			playerIsDead = true;
		}
		
		// A kamera korlatjai
		if (charX >= ScreenWidth / 2 && charX <= BufferWidth-(ScreenWidth)/2) currentX = charX - ScreenWidth/2;  
		if (charY >= ScreenHeight / 2 && charY <= BufferWidth-(ScreenWidth)/2) currentY = charY - ScreenHeight/2;
		if (currentY < 0) currentY = 0; else if (currentY > BufferHeight-ScreenHeight) currentY = BufferHeight;
		if (currentX < 0) currentX = 0; else if (currentX > BufferWidth-ScreenWidth) currentX = BufferWidth;
		
		//Inventory Kiirasa
		if (inventoryOn){
			rectfill(invScreen,0,0,InvWidth,InvHeight,White);
			textout_ex(invScreen,font,"Inventory",20,20,Black,-1);
			for (int i=0;i<12;i++){
				draw_sprite(invScreen,slot[i].pic,slot[i].x,slot[i].y);
				itoa(slot[i].nr,R,10);
				textout_ex(invScreen,font,R,slot[i].x+50,slot[i].y+54,Black,-1);
			}
			draw_sprite(buffer,invScreen,currentX+(ScreenWidth/2)-(InvWidth/2),currentY+(ScreenHeight/2)-(InvHeight/2));
		}
		
		//Buildinv kiirasa
		if (drawBuildMenu){
			rectfill(buildMenu,0,0,buildMenu->w,buildMenu->h,White);
			textout_ex(buildMenu,font,"Buildings Menu",20,20,Black,-1);
			draw_sprite(buildMenu,def,20,40);
			draw_sprite(buildMenu,house,20,40);
			draw_sprite(buildMenu,def,104,40);
			draw_sprite(buildMenu,smithy,104,40);
			draw_sprite(buildMenu,def,188,40);
			draw_sprite(buildMenu,def,272,40);
			draw_sprite(buffer,buildMenu,currentX+(ScreenWidth/2)-(buildMenu->w/2),currentY+(ScreenHeight/2)-(buildMenu->h/2));
		}
		
		//Crafting menu kiirasa
		if (showCraftMenu){
			rectfill(craftMenu,0,0,craftMenu->w,craftMenu->h,White);
			textout_ex(craftMenu,font,"Crafting ",20,20,Black,-1);
			textout_ex(craftMenu,font,"Inventory",20,124,Black,-1);
			// 1
			draw_sprite(craftMenu,ironBarPic,20,40); 
			itoa(inv.iron,R,10);
			textout_ex(craftMenu,font,R,74,94,Black,-1);
			draw_sprite(craftMenu,ironOrePic,20,144);
			itoa(inv.ironOre,R,10);
			textout_ex(craftMenu,font,R,74,198,Black,-1);
			// 2
			draw_sprite(craftMenu,kugelPic,104,40); 
			itoa(inv.kugel,R,10);
			textout_ex(craftMenu,font,R,158,94,Black,-1);
			draw_sprite(craftMenu,ironBarPic,104,144);
			itoa(inv.iron,R,10);
			textout_ex(craftMenu,font,R,158,198,Black,-1);
			// 3
			draw_sprite(craftMenu,arrowPic,188,40);
			itoa(inv.arrow,R,10);
			textout_ex(craftMenu,font,R,242,94,Black,-1);
			draw_sprite(craftMenu,woodPic,188,144);
			itoa(inv.wood,R,10);
			textout_ex(craftMenu,font,R,242,198,Black,-1);
			// 4
			draw_sprite(craftMenu,bowPic,272,40);
			itoa(inv.bow,R,10);
			textout_ex(craftMenu,font,R,272+54,40+54,Black,-1);
			draw_sprite(craftMenu,woodPic,272,144);
			itoa(inv.wood,R,10);
			textout_ex(craftMenu,font,R,272+54,144+54,Black,-1);
			// 5			
			draw_sprite(craftMenu,gunPic,272+84,40);
			itoa(inv.gun,R,10);
			textout_ex(craftMenu,font,R,272+84+54,94,Black,-1);
			draw_sprite(craftMenu,ironBarPic,272+84,144);
			itoa(inv.iron,R,10);
			textout_ex(craftMenu,font,R,272+84+54,198,Black,-1);
			
			draw_sprite(buffer,craftMenu,currentX+(ScreenWidth/2)-(craftMenu->w/2),currentY+(ScreenHeight/2)-(craftMenu->h/2));
		}
		
		// Haz Menuje
		if (showHouseMenu){
			rectfill(houseMenu,0,0,houseMenu->w,houseMenu->h,White);
			textout_ex(houseMenu,font,"Cooking",10,20,Black,-1);
			draw_sprite(houseMenu,cookedMeatPic,20,40);
			itoa(inv.stake,R,10);
			textout_ex(houseMenu,font,R,74,94,Black,-1);
			textout_ex(houseMenu,font,"Inventory",10,124,Black,-1);
			draw_sprite(houseMenu,meatPic,20,144);
			itoa(inv.meat,R,10);
			textout_ex(houseMenu,font,R,74,198,Black,-1);
			draw_sprite(buffer,houseMenu,currentX+(ScreenWidth/2)-(houseMenu->w/2),currentY+(ScreenHeight/2)-(houseMenu->h/2));
		}
		
		// Buffer kiirasa
		masked_blit(buffer,screen,currentX,currentY,0,0,currentX+ScreenWidth,currentY+ScreenHeight);
		
		// Developer Output
		itoa(effood,R,10);
		textout_right_ex(screen,font,R,ScreenWidth-10,ScreenHeight-10,makecol(255,255,255),-1);
		itoa((int)HP,R,10);
		textout_ex(screen,font,R,10,ScreenHeight-10,makecol(255,0,0),-1);
		itoa((int)foodMeter,R,10);
		textout_ex(screen,font,R,50,ScreenHeight-10,makecol(100,0,100),-1);
		itoa(projectileNumber,R,10);
		textout_ex(screen,font,R,300,ScreenHeight-10,makecol(155,155,0),-1);
		
		rest(20);
	}
}END_OF_FUNCTION();

// Resolution
void settings(){
	bool exitSettings = false;
	while(!exitSettings){
		if(keyrel(KEY_ESC)) exitSettings = true;
		
		textout_ex(apbuff,font,"Beallitasok Menue",10,10,White,-1);
		
		//textout_ex(apbufcf
		draw_sprite(screen,apbuff,0,0);
		clear_bitmap(apbuff);
	}
}

void howToPlay(){
	bool exitHowToPlay = false;
	while(!exitHowToPlay){
		if (keyrel(KEY_ESC)) exitHowToPlay = true;
		
		textout_ex(apbuff,font,"Hogyan Kell Jatszani Menue",10,10,White,-1);
		textout_ex(apbuff,font,"Iranyitas:",20,50,White,-1);
		textout_ex(apbuff,font,"W vagy UP: Felfele valo jaras",30,60,White,-1);
		textout_ex(apbuff,font,"S vagy DOWN: Lefele valo jaras",30,70,White,-1);
		textout_ex(apbuff,font,"D vagy RIGHT: Jobbra valo jaras",30,80,White,-1);
		textout_ex(apbuff,font,"A vagy LEFT: Ballra valo jaras",30,90,White,-1);
		textout_ex(apbuff,font,"B: Epuletek menujenek megjelenitese",30,100,White,-1);
		textout_ex(apbuff,font,"F: Fegyver hasznalata",30,110,White,-1);
		textout_ex(apbuff,font,"E: Hatizsak tartalmanak a megtekintese",30,120,White,-1);
		textout_ex(apbuff,font,"SPACE: Nyersanyaggyujtes/ altalanos akciogomb",30,130,White,-1);
		
		draw_sprite(screen,apbuff,0,0);
	}
}

void loadSettings(){
	string garbage;
	string gold;
	bool FULLSCREEN;
	ifstream settingsFile("settings.txt",ios::in);
	// Fullscreen
	settingsFile >> garbage;
	settingsFile >> gold;
	if (gold == "false") FULLSCREEN = false;
	else if (gold == "true") FULLSCREEN = true;
	
	// Resolution
	settingsFile >> garbage; 
	settingsFile >> ScreenWidth;
	settingsFile >> ScreenHeight;
	settingsFile.close();
	
	if (!FULLSCREEN) set_gfx_mode(GFX_AUTODETECT_WINDOWED,ScreenWidth,ScreenHeight,0,0);
	else set_gfx_mode(GFX_AUTODETECT,ScreenWidth,ScreenHeight,0,0);
	apbuff = create_bitmap(ScreenWidth,ScreenWidth);
	menuScrn  = load_bitmap("GFX/menuscrn.bmp",NULL); 
}

void loadScreen(){
	clear_bitmap(apbuff);
	rectfill(apbuff,0,0,ScreenWidth,ScreenHeight,Black);
	textout_centre_ex(apbuff,font,"A Jatek Betoeltoedik...",ScreenWidth/2,ScreenHeight/2,White,-1);
	draw_sprite(screen,apbuff,0,0);
}

void menu(){
	bool exitGame = false;
	
	
	
	while (!exitGame){
		
		
		if (key[KEY_ESC])exitGame = true;
		if (key[KEY_1]){
			loadScreen();
			if (!areBitmapsAndSoundsLoaded) loadBitmapsAndSound();
			game();
		}
		if (key[KEY_2]) howToPlay();
		if (key[KEY_3]) exitGame = true;
		
		draw_sprite(apbuff,menuScrn,0,0);
		textout_ex(apbuff,font,"PROSOMO",ScreenWidth/2,ScreenHeight/2-50,White,-1);
		textout_ex(apbuff,font,"1. Uj Jatek",ScreenWidth/2,ScreenHeight/2,White,-1);
		textout_ex(apbuff,font,"2. Hogyan kell jatszani",ScreenWidth/2,ScreenHeight/2+10,White,-1);
		textout_ex(apbuff,font,"3. Kilepes",ScreenWidth/2,ScreenHeight/2+20,White,-1);
		
		
		draw_sprite(screen,apbuff,0,0);
		clear_bitmap(apbuff);
	}
}

int main(){
	initializeAllegro();
	loadSettings();
	menu();
	deinitializeAllegroAndStuff();
	return 0;
}END_OF_MAIN()
