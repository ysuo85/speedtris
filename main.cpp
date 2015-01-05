//FALLING BLOCK GAME!
//main.cpp
//tell compiler to not include many unneeded header files.
#define WIN32_LEAN_AND_MEAN
//need this for windows stuff.
#include <windows.h>
//need this for srand and rand
#include <stdlib.h>
#include <iostream>
//now let's include our bitmapobject definitions
#include "bitmapobject.h"

//let's give our window a name
#define WINDOWCLASS "FallingBlockGame"
//let's give our window a title...er caption.
#define WINDOWTITLE "Welcome to Tetris!"

//since we're using square blocks, let's only use a single size.
const int TILESIZE=16;
//now for the map...
const int MAPWIDTH=10;
const int MAPHEIGHT=30;
const int GREY=8;

// color specifications for bitmap blocks, 10 variations including a "do not draw" option.
const int TILENODRAW=-1;
const int TILEBLACK=0;
const int TILEGREY=1;
const int TILEBLUE=2;
const int TILERED=3;
const int TILEGREEN=4;
const int TILEYELLOW=5;
const int TILEWHITE=6;
const int TILESTEEL=7;
const int TILEPURPLE=8;

bool GameInit(); // game initialization function
void GameLoop(); //where the game actually takes place
void GameDone(); //clean up!
void DrawTile(int x, int y, int tile); //coordinates & tile type
void DrawMap(); //draw the whole map.. render function, basically
void NewBlock(); //create a new block!
void RotateBlock(); //rotate a block.. if you can!
void Move(int x, int y); //coordinates to move.
int CollisionTest(int nx, int ny); //test collision of blocks
void RemoveRow(int row); //remove a row.. that would be the 'x'.
void NewGame(); //make a new game!

int rowsCleared = 0;
DWORD speedInterval = 1000;


HINSTANCE hInstMain=NULL; //main app handle
HWND hWndMain=NULL; //main window handle
int Map[MAPWIDTH][MAPHEIGHT+1]; //the game map!

// game piece
struct Piece {
	int size[4][4];
	int x;
	int y;
};

Piece sPrePiece; //preview piece.
Piece sPiece; //the 's' prefixes indicate this is a 'structure'

DWORD start_time;  //used in timing
bool GAMESTARTED=false; //used by NewBlock()

//map for the program
BitMapObject bmoMap;
//block images
BitMapObject bmoBlocks;

LRESULT CALLBACK TheWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	//which message did we get?
	switch(uMsg)
	{
	case WM_KEYDOWN:
		{
			//check for escape key
			if(wParam==VK_ESCAPE)
			{
				DestroyWindow(hWndMain);
				return(0);//handled message
			}
			else if(wParam==VK_DOWN) //check for down arrow key
			{
				Move(0,1);
				return(0);//handled message
			}
			else if(wParam==VK_UP) //check for up arrow key
			{
				RotateBlock();
				return(0);//handled message
			}
			else if(wParam==VK_LEFT) //check for left arrow key
			{
				Move(-1,0);
				return(0);//handled message
			}
			else if(wParam==VK_RIGHT) //check for right arrow key
			{
				Move(1,0);
				return(0);//handled message
			}
			else if(wParam==VK_SPACE) //check for space key
            {
                while(!CollisionTest(0,1)){ //move block all the way to bottom
                    Move(0,1);
                }
                return(0);//handled message
            }
		}break;
	case WM_DESTROY://the window is being destroyed
		{

			//tell the application we are quitting
			PostQuitMessage(0);

			//handled message, so return 0
			return(0);

		}break;
	case WM_PAINT://the window needs repainting
		{
			//a variable needed for painting information
			PAINTSTRUCT ps;

			//start painting
			HDC hdc=BeginPaint(hwnd,&ps);

			//redraw the map
			BitBlt(hdc,0,0,TILESIZE*MAPWIDTH+TILESIZE*GREY,TILESIZE*MAPHEIGHT,bmoMap,0,0,SRCCOPY);

			//end painting
			EndPaint(hwnd,&ps);

			//handled message, so return 0
			return(0);
		}break;
	}

	//pass along any other message to default message handler
	return(DefWindowProc(hwnd,uMsg,wParam,lParam));
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	//assign instance to global variable
	hInstMain=hInstance;

	//create window class
	WNDCLASSEX wcx;

	//set the size of the structure
	wcx.cbSize=sizeof(WNDCLASSEX);

	//class style
	wcx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	//window procedure
	wcx.lpfnWndProc=TheWindowProc;

	//class extra
	wcx.cbClsExtra=0;

	//window extra
	wcx.cbWndExtra=0;

	//application handle
	wcx.hInstance=hInstMain;

	//icon
	wcx.hIcon=LoadIcon(NULL,IDI_APPLICATION);

	//cursor
	wcx.hCursor=LoadCursor(NULL,IDC_ARROW);

	//background color
	wcx.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);

	//menu
	wcx.lpszMenuName=NULL;

	//class name
	wcx.lpszClassName=WINDOWCLASS;

	//small icon
	wcx.hIconSm=NULL;

	//register the window class, return 0 if not successful
	if(!RegisterClassEx(&wcx)) return(0);

	//create main window
	hWndMain=CreateWindowEx(0,WINDOWCLASS,WINDOWTITLE, WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,0,0,320,240,NULL,NULL,hInstMain,NULL);

	//error check
	if(!hWndMain) return(0);

	//if program initialization failed, then return with 0
	if(!GameInit()) return(0);

	//message structure
	MSG msg;

	//message pump
	for( ; ; )
	{
		//look for a message
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			//there is a message

			//check that we arent quitting
			if(msg.message==WM_QUIT) break;

			//translate message
			TranslateMessage(&msg);

			//dispatch message
			DispatchMessage(&msg);
		}

		//run main game loop
		GameLoop();

	}

	//clean up program data
	GameDone();

	//return the wparam from the WM_QUIT message
	return(msg.wParam);
}

bool GameInit()
{
	//set the client area size
	RECT rcTemp;
	SetRect(&rcTemp,0,0,MAPWIDTH*TILESIZE+TILESIZE*GREY,MAPHEIGHT*TILESIZE);//160x480 client area
	AdjustWindowRect(&rcTemp,WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,FALSE);//adjust the window size based on desired client area
	SetWindowPos(hWndMain,NULL,0,0,rcTemp.right-rcTemp.left,rcTemp.bottom-rcTemp.top,SWP_NOMOVE);//set the window width and height

	//create map image
	HDC hdc=GetDC(hWndMain);
	bmoMap.Create(hdc,MAPWIDTH*TILESIZE+TILESIZE*GREY,MAPHEIGHT*TILESIZE);
	FillRect(bmoMap,&rcTemp,(HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(hWndMain,hdc);

	bmoBlocks.Load(NULL,"blocks.bmp");
	NewGame();

	return(true);//return success
}

void GameDone()
{
	//clean up code goes here
}

void GameLoop()
{
	if( (GetTickCount() - start_time) > speedInterval)
	{
	    if(rowsCleared >= 1){
            rowsCleared = 0;
            if(speedInterval > 100){
                speedInterval -= 100;
                std::cout << "Debug: speedInterval = " << speedInterval << "\n";
            }
            else if(speedInterval > 50){
                speedInterval = speedInterval / 2;
                std::cout << "Debug: speedInterval = " << speedInterval << "\n";
            }
	    }
		Move(0,1);
		start_time=GetTickCount();
	}
}

void NewGame()
{
	start_time=GetTickCount();
	GAMESTARTED=false;

	//start out the map
	for(int x=0;x< MAPWIDTH;x++)
	{
		for(int y=0;y< MAPHEIGHT+1;y++)
		{
			if(y==MAPHEIGHT) //makes Y-collision easier.
				Map[x][y]=TILEGREY;
			else
				Map[x][y]=TILEBLACK;
		}
	}
	NewBlock();

	DrawMap();
}

void DrawTile(int x,int y,int tile)//put a tile
{
	//mask first
	BitBlt(bmoMap,x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE,bmoBlocks,tile*TILESIZE,TILESIZE,SRCAND);
	//then image
	BitBlt(bmoMap,x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE,bmoBlocks,tile*TILESIZE,0,SRCPAINT);
}

void DrawMap()//draw screen
{
	int xmy, ymx;

	//place the toolbar
	for(xmy=MAPWIDTH; xmy< MAPWIDTH+GREY; xmy++)
		for(ymx=0; ymx< MAPHEIGHT; ymx++)
			DrawTile(xmy, ymx, TILEGREY);

	//draw preview block
	for(xmy=0; xmy<4; xmy++)
		for(ymx=0; ymx<4; ymx++)
			if(sPrePiece.size[xmy][ymx] != TILENODRAW)
				DrawTile(sPrePiece.x+xmy, sPrePiece.y+ymx, sPrePiece.size[xmy][ymx]);

	//draw the map
	//loop through the positions
	for(xmy=0;xmy< MAPWIDTH;xmy++)
		for(ymx=0;ymx< MAPHEIGHT;ymx++)
				DrawTile(xmy,ymx,Map[xmy][ymx]);

	//draw moving block
	for(xmy=0; xmy<4; xmy++)
		for(ymx=0; ymx<4; ymx++)
			if(sPiece.size[xmy][ymx] != TILENODRAW)
				DrawTile(sPiece.x+xmy, sPiece.y+ymx, sPiece.size[xmy][ymx]);

	//invalidate the window rect
	InvalidateRect(hWndMain,NULL,FALSE);
}

void NewBlock()
{
	int newblock;
	int i,j;
	//  0   1   2   3   4    5   6
	//   X                             These
	//   X   XX   X  XX   XX  XX   XX  are
	//   X   XX  XXX  XX XX    X   X   block
	//   X                     X   X   types

	//begin game! make generate a block and then one in preview.

	srand(GetTickCount());


	//initialize the piece to all blank.
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			sPiece.size[ i ][j]=TILENODRAW;

	sPiece.x=MAPWIDTH/2-2;
	sPiece.y=-1;

	//let's see if the game's started yet
	if(GAMESTARTED == false)
	{
		//guess not..
		//Generate a piece right off.
		//From now on, use previous preview block.
		GAMESTARTED=true;

		newblock=rand()%7;

		switch (newblock)
		{
		case 0: //Tower!
			{
				sPiece.size[1][0]=TILERED;
				sPiece.size[1][1]=TILERED;
				sPiece.size[1][2]=TILERED;
				sPiece.size[1][3]=TILERED;
				sPiece.y=0;
			}break;
		case 1: //Box!
			{
				sPiece.size[1][1]=TILEBLUE;
				sPiece.size[1][2]=TILEBLUE;
				sPiece.size[2][1]=TILEBLUE;
				sPiece.size[2][2]=TILEBLUE;
			}break;
		case 2: //Pyramid!
			{
				sPiece.size[1][1]=TILESTEEL;
				sPiece.size[0][2]=TILESTEEL;
				sPiece.size[1][2]=TILESTEEL;
				sPiece.size[2][2]=TILESTEEL;
			}break;
		case 3://Left Leaner
			{
				sPiece.size[0][1]=TILEYELLOW;
				sPiece.size[1][1]=TILEYELLOW;
				sPiece.size[1][2]=TILEYELLOW;
				sPiece.size[2][2]=TILEYELLOW;
			}break;
		case 4://Right Leaner
			{
				sPiece.size[2][1]=TILEGREEN;
				sPiece.size[1][1]=TILEGREEN;
				sPiece.size[1][2]=TILEGREEN;
				sPiece.size[0][2]=TILEGREEN;
			}break;
		case 5://Left Knight
			{
				sPiece.size[1][1]=TILEWHITE;
				sPiece.size[2][1]=TILEWHITE;
				sPiece.size[2][2]=TILEWHITE;
				sPiece.size[2][3]=TILEWHITE;
			}break;
		case 6://Right Knight
			{
				sPiece.size[2][1]=TILEPURPLE;
				sPiece.size[1][1]=TILEPURPLE;
				sPiece.size[1][2]=TILEPURPLE;
				sPiece.size[1][3]=TILEPURPLE;
			}break;
		}
	}
	else
	{
		for(i=0; i<4; i++)
			for(j=0; j<4; j++)
				sPiece.size[ i ][j]=sPrePiece.size[ i ][j];

	}

	newblock=rand()%7;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			sPrePiece.size[ i ][j]=TILENODRAW;

	sPrePiece.x=MAPWIDTH+GREY/4;
	sPrePiece.y=GREY/4;

	switch (newblock)
	{
		case 0: //Tower!
			{
				sPrePiece.size[1][0]=TILERED;
				sPrePiece.size[1][1]=TILERED;
				sPrePiece.size[1][2]=TILERED;
				sPrePiece.size[1][3]=TILERED;
			}break;
		case 1: //Box!
			{
				sPrePiece.size[1][1]=TILEBLUE;
				sPrePiece.size[1][2]=TILEBLUE;
				sPrePiece.size[2][1]=TILEBLUE;
				sPrePiece.size[2][2]=TILEBLUE;
			}break;
		case 2: //Pyramid!
			{
				sPrePiece.size[1][1]=TILESTEEL;
				sPrePiece.size[0][2]=TILESTEEL;
				sPrePiece.size[1][2]=TILESTEEL;
				sPrePiece.size[2][2]=TILESTEEL;
			}break;
		case 3://Left Leaner
			{
				sPrePiece.size[0][1]=TILEYELLOW;
				sPrePiece.size[1][1]=TILEYELLOW;
				sPrePiece.size[1][2]=TILEYELLOW;
				sPrePiece.size[2][2]=TILEYELLOW;
			}break;
		case 4://Right Leaner
			{
				sPrePiece.size[2][1]=TILEGREEN;
				sPrePiece.size[1][1]=TILEGREEN;
				sPrePiece.size[1][2]=TILEGREEN;
				sPrePiece.size[0][2]=TILEGREEN;
			}break;
		case 5://Left Knight
			{
				sPrePiece.size[1][1]=TILEWHITE;
				sPrePiece.size[2][1]=TILEWHITE;
				sPrePiece.size[2][2]=TILEWHITE;
				sPrePiece.size[2][3]=TILEWHITE;
			}break;
		case 6://Right Knight
			{
				sPrePiece.size[2][1]=TILEPURPLE;
				sPrePiece.size[1][1]=TILEPURPLE;
				sPrePiece.size[1][2]=TILEPURPLE;
				sPrePiece.size[1][3]=TILEPURPLE;
			}break;
	}

	DrawMap();
}

void RotateBlock()
{
	int i, j, temp[4][4];

	//copy &rotate the piece to the temporary array
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			temp[3-j][ i ]=sPiece.size[ i ][j];

	//check collision of the temporary array with map borders
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			if(temp[ i ][j] != TILENODRAW)
				if(sPiece.x + i < 0 || sPiece.x + i > MAPWIDTH - 1 ||
					sPiece.y + j < 0 || sPiece.y + j > MAPHEIGHT - 1)
					return;

	//check collision of the temporary array with the blocks on the map
	for(int x=0; x< MAPWIDTH; x++)
		for(int y=0; y< MAPHEIGHT; y++)
			if(x >= sPiece.x && x < sPiece.x + 4)
				if(y >= sPiece.y && y < sPiece.y +4)
					if(Map[x][y] != TILEBLACK)
						if(temp[x - sPiece.x][y - sPiece.y] != TILENODRAW)
							return;

	//end collision check

	//successful!  copy the rotated temporary array to the original piece
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			sPiece.size[ i ][j]=temp[ i ][j];

	DrawMap();

	return;
}

void Move(int x, int y)
{
	if(CollisionTest(x, y))
	{
		if(y == 1)
		{
			if(sPiece.y<1)
			{
				//you lose!  new game.
				NewGame();
			}
			else
			{
				bool killblock=false;
				int i,j;
				//new block time! add this one to the list!
				for(i=0; i<4; i++)
					for(j=0; j<4; j++)
						if(sPiece.size[ i ][j] != TILENODRAW)
							Map[sPiece.x+i][sPiece.y+j] = sPiece.size[ i ][j];

				//check for cleared row!
				for(j=0; j< MAPHEIGHT; j++)
				{
					bool filled=true;
					for(i=0; i< MAPWIDTH; i++)
						if(Map[ i ][j] == TILEBLACK)
							filled=false;

					if(filled)
					{
						RemoveRow(j);
						killblock=true;
					}
				}

				if(killblock)
				{
					for(i=0; i<4; i++)
						for(j=0; j<4; j++)
							sPiece.size[ i ][j]=TILENODRAW;
				}
				NewBlock();
			}
		}

	}
	else
	{
		sPiece.x+=x;
		sPiece.y+=y;
	}

	DrawMap();
}

int CollisionTest(int nx, int ny)
{
	int newx=sPiece.x+nx;
	int newy=sPiece.y+ny;

	int i,j,x,y;

	for(i=0; i< 4; i++)
		for(j=0; j< 4; j++)
			if(sPiece.size[ i ][j] != TILENODRAW)
				if(newx + i < 0 || newx + i > MAPWIDTH - 1 ||
					newy + j < 0 || newy + j > MAPHEIGHT - 1)
					return 1;

	for(x=0; x< MAPWIDTH; x++)
		for(y=0; y< MAPHEIGHT; y++)
			if(x >= newx && x < newx + 4)
				if(y >= newy && y < newy +4)
					if(Map[x][y] != TILEBLACK)
						if(sPiece.size[x - newx][y - newy] != TILENODRAW)
							return 1;
	return 0;
}

void RemoveRow(int row)
{
	int x,y;

	for(x=0; x< MAPWIDTH; x++){
		for(y=row; y>0; y--){
            Map[x][y]=Map[x][y-1];
            rowsCleared++;
        }
	}
}

