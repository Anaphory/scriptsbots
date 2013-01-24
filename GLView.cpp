#include "GLView.h"
#include "ReadWrite.h"

#include <ctime>

#include "config.h"
#ifdef LOCAL_GLUT32
#include "glut.h"
#else
#include <GL/glut.h>
#endif

#include <stdio.h>

void gl_processNormalKeys(unsigned char key, int x, int y)
{
	GLVIEW->processNormalKeys(key, x, y);
}
void gl_processSpecialKeys(int key, int x, int y)
{
	GLVIEW->processSpecialKeys(key, x, y);
}
void gl_processReleasedKeys(unsigned char key, int x, int y)
{
	GLVIEW->processReleasedKeys(key, x, y);
}
void gl_menu(int key)
{
	GLVIEW->menu(key);
}
void gl_changeSize(int w, int h)
{
	GLVIEW->changeSize(w,h);
}
void gl_handleIdle()
{
	GLVIEW->handleIdle();
}
void gl_processMouse(int button, int state, int x, int y)
{
	GLVIEW->processMouse(button, state, x, y);
}
void gl_processMouseActiveMotion(int x, int y)
{
	GLVIEW->processMouseActiveMotion(x,y);
}
void gl_processMousePassiveMotion(int x, int y)
{
	GLVIEW->processMousePassiveMotion(x,y);
}
void gl_renderScene()
{
	GLVIEW->renderScene();
}


void RenderString(float x, float y, void *font, const char* string, float r, float g, float b)
{
	glColor3f(r,g,b);
	glRasterPos2f(x, y);
	int len = (int) strlen(string);
	for (int i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
}

void drawCircle(float x, float y, float r) {
	float n;
	for (int k=0;k<17;k++) {
		n = k*(M_PI/8);
		glVertex3f(x+r*sin(n),y+r*cos(n),0);
	}
}


GLView::GLView(World *s) :
		world(world),
		paused(false),
		draw(true),
		skipdraw(1),
		layer(1),
		modcounter(0),
		frames(0),
		lastUpdate(0)
{

	xtranslate= 0.0;
	ytranslate= 0.0;
	scalemult= 0.2;
	downb[0]=0;downb[1]=0;downb[2]=0;
	mousex=0;mousey=0;
	
	following = false;
}

GLView::~GLView()
{

}
void GLView::changeSize(int w, int h)
{
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,conf::WWIDTH,conf::WHEIGHT,0,0,1);
}

void GLView::processMouse(int button, int state, int x, int y)
{
	if(world->isDebug()) printf("MOUSE EVENT: button=%i state=%i x=%i y=%i\n", button, state, x, y);
	
	//have world deal with it. First translate to world coordinates though
	if(button==0){
		int wx= (int) ((x-conf::WWIDTH/2)/scalemult-xtranslate);
		int wy= (int) ((y-conf::WHEIGHT/2)/scalemult-ytranslate);

		world->processMouse(button, state, wx, wy);
	}
	
	mousex=x; mousey=y;
	downb[button]=1-state; //state is backwards, ah well
}

void GLView::processMouseActiveMotion(int x, int y)
{
	if(world->isDebug()) printf("MOUSE MOTION x=%i y=%i, %i %i %i\n", x, y, downb[0], downb[1], downb[2]);
	if (downb[0]==1) {
		//left mouse button drag: pan around
		xtranslate += (x-mousex)/scalemult;
		ytranslate += (y-mousey)/scalemult;
		if (abs(x-mousex)>8 || abs(x-mousex)>8) following= 0;
		//for releasing follow if the mouse is used to drag screen, but there's a threshold
	}  
	if (downb[1]==1) {
		//mouse wheel. Change scale
		scalemult -= 0.002*(y-mousey);
		if(scalemult<0.01) scalemult=0.01;
	}
/*	if(downb[2]==1){ //disabled
		//right mouse button.
	}*/
   
	mousex=x; mousey=y;
}

void GLView::processMousePassiveMotion(int x, int y)
{
	//(GPA) for mouse scrolling. DISABLED
/*	if(y<=30) ytranslate += 2*(30-y);
	if(y>=conf::WHEIGHT-30) ytranslate -= 2*(y-(conf::WHEIGHT-30));
	if(x<=30) xtranslate += 2*(30-x);
	if(x>=conf::WWIDTH-30) xtranslate -= 2*(x-(conf::WWIDTH-30));*/
}

void GLView::menu(int key) //(GPA)
{
	ReadWrite* savehelper= new ReadWrite(); //for loading/saving
	if (key == 27) {
	  unsigned long int time_now = std::time(0);
	  char filename[40]; // enough to hold all numbers up to 64-bits
	  sprintf(filename, "W%d.bots", time_now);
	  printf("AUTOSAVING WORLD TO %s\n", filename);
	  savehelper->saveWorld(world, xtranslate, ytranslate, filename);
	  exit(0);
	} else if (key=='q') {
	  unsigned long int time_now = std::time(0);
	  char filename[40]; // enough to hold all numbers up to 64-bits
	  sprintf(filename, "W%d.bots", time_now);
	  printf("QUICKSAVING WORLD TO %s\n", filename);
	  savehelper->saveWorld(world, xtranslate, ytranslate, filename);
	} else if (key=='0') {
	  unsigned long int time_now = std::time(0);
	  char filename[40]; // enough to hold all numbers up to 64-bits
	  sprintf(filename, "W%d.bots", time_now);
	  printf("AUTOSAVING WORLD TO %s\n", filename);
	  savehelper->saveWorld(world, xtranslate, ytranslate, filename);
	  world->reset();
	  printf("Agents reset\n");
	} else if (key=='p') {
		//pause
		paused= !paused;
	} else if (key=='m') { //drawing
		draw= !draw;
	} else if (key==43) { //+
		skipdraw++;
	} else if (key==45) { //-
		skipdraw--;
	} else if (key=='n' || key=='r') { //layer switch; n= "next", r= "previous"
		if (key=='n') layer++;
		else layer--;
		if (layer>LAYERS) layer= 0;
		if (layer<0) layer= LAYERS;
		glutGet(GLUT_MENU_NUM_ITEMS); //this brings up the right-click menu, so that we can change entries based on world settings. Just aesthetic
		//begin glut for menu changes. Note that it can only show the "next" layer
		if (layer==0) glutChangeToMenuEntry(2, "Show: Plant Food", 'l');
		else if (layer==1) glutChangeToMenuEntry(2, "Show: Meat Food", 'l');
		else if (layer==2) glutChangeToMenuEntry(2, "Show: Hazards", 'l');
		else if (layer==3) glutChangeToMenuEntry(2, "Show: Temperature", 'l');
		else if (layer==LAYERS) glutChangeToMenuEntry(2, "Show: None", 'l');
		else glutChangeToMenuEntry(2, "Show: next layer",'l');
		glutSetMenu(m_id);
	} else if (key=='x') {
		world->addRandomBots(5);
	} else if (key=='c') {
		world->addRandomBots(5, 2);
	} else if (key=='v') {
		world->addRandomBots(5, 1);
	} else if (key==9) { //[tab] toggles closed/open world
		world->setClosed( !world->isClosed() );
		glutGet(GLUT_MENU_NUM_ITEMS);
		if (world->isClosed()) glutChangeToMenuEntry(4, "Open World", 'c');
		else glutChangeToMenuEntry(4, "Close World", 'c');
		glutSetMenu(m_id);
	} else if (key=='f') {
		if(following==0) following= 2; //follow selected agent
		else following= 0;
	} else if(key=='o') {
		if(following==0) following= 1; //follow oldest agent
		else following= 0;
	} else if(key=='g') {
		if(following==0) following= 3; //follow most advanced generation agent
		else following= 0;
	} else if(key=='h') {
		if(following==0) following= 4; //follow healthiest
		else following= 0;
	}else if (key==127) { //delete
		world->deleting= 1;
	}else if (key=='>') { //zoom+ >
		scalemult += 0.012;
		if(scalemult<0.01) scalemult=0.01;
	}else if (key=='<') { //zoom- <
		scalemult -= 0.012;
	}else if (key==32) { //spacebar input [pressed]
		world->pinput1= 1;
	}else if (key=='l') { //l (move faster)
		world->pcontrol= true;
		world->pleft= cap(world->pleft + 0.08);
		world->pright= cap(world->pright + 0.08);
	}else if (key=='i') { //i (turn left)
		world->pcontrol= true;
		world->pleft= cap(world->pleft - 0.05 + (world->pright-world->pleft)*0.05); //this extra code helps with turning out of tight circles
		world->pright= cap(world->pright + 0.05 + (world->pleft-world->pright)*0.05);
	}else if (key=='a') { //a (move slower)
		world->pcontrol= true;
		world->pleft= cap(world->pleft - 0.08);
		world->pright= cap(world->pright - 0.08);
	}else if (key=='e') { //e (turn right)
		world->pcontrol= true;
		world->pleft= cap(world->pleft + 0.05 + (world->pright-world->pleft)*0.05);
		world->pright= cap(world->pright - 0.05 + (world->pleft-world->pright)*0.05);
	} else if (key==228) { //ä (toggle player control)
		world->setControl(!world->pcontrol);
		glutGet(GLUT_MENU_NUM_ITEMS);
		if (world->pcontrol) glutChangeToMenuEntry(5, "Release Agent", 999);
		else glutChangeToMenuEntry(5, "Control Agent", 999);
		glutSetMenu(m_id);
	}else if (key=='s') { //s (save world)
		printf("SAVING WORLD\n");
		printf("Type a valid file name (ex: world.bots): ");
		scanf("%s", filename);
		savehelper->saveWorld(world, xtranslate, ytranslate, filename);
	}else if (key=='k') { //k (load world)
		printf("LOADING WORLD\n");
		printf("Type a valid file name (ex: world.bots): ");
		scanf("%s", filename);
		//reset first
		world->reset();
		savehelper->loadWorld(world, xtranslate, ytranslate, filename);
	}else if (key=='d') { //menu only, debug mode
		world->setDebug( !world->isDebug() );
		glutGet(GLUT_MENU_NUM_ITEMS);
		if (world->isDebug()){
			glutChangeToMenuEntry(18, "Exit Debug Mode", 1005);
			printf("Entered Debug Mode\n");
		} else glutChangeToMenuEntry(18, "Enter Debug Mode", 1005);
		glutSetMenu(m_id);
	} else {
		printf("Unknown key pressed: %i\n", key);
	}
	//other keys: '1':49, '2':50, ..., '0':48
}

void GLView::menuS(int key) // (GPA) movement control
{
	if (key == GLUT_KEY_UP) {
	   ytranslate += 20/scalemult;
	} else if (key == GLUT_KEY_LEFT) {
		xtranslate += 20/scalemult;
	} else if (key == GLUT_KEY_DOWN) {
		ytranslate -= 20/scalemult;
	} else if (key == GLUT_KEY_RIGHT) {
		xtranslate -= 20/scalemult;
	}
}

void GLView::processNormalKeys(unsigned char key, int x, int y)
{
	menu(key);	
}

void GLView::processSpecialKeys(int key, int x, int y)
{
	menuS(key);	
}

void GLView::processReleasedKeys(unsigned char key, int x, int y)
{
	if (key==32) {//spacebar input [released]
			world->pinput1= 0;
	}
}

void GLView::glCreateMenu(void)
{
	m_id = glutCreateMenu(gl_menu); //right-click context menu
	glutAddMenuEntry("Fast Mode (m)", 'm');
	glutAddMenuEntry("Show: Meat Food (k,l)", 'l'); //line contains mode-specific text, see menu function above
	glutAddMenuEntry("Pause (p)", 'p');
	glutAddMenuEntry("Close World (c)", 'c'); //line contains mode-specific text, see menu function above
	glutAddMenuEntry("Control Selected (w,a,s,d)", 999); //line contains mode-specific text, see menu function above
	glutAddMenuEntry("Follow Selected (f)", 'f');
	glutAddMenuEntry("Follow Oldest (o)", 'o');
	glutAddMenuEntry("Follow Highest Gen (g)", 'g');
	glutAddMenuEntry("Follow Healthiest (h)", 'h');
	glutAddMenuEntry("-------------------",-1);
	glutAddMenuEntry("Spawn Agents", 1002);
	glutAddMenuEntry("Spawn Herbivores", 1003);
	glutAddMenuEntry("Spawn Carnivores", 1004);
	glutAddMenuEntry("Delete Agent (del)", 127);
	glutAddMenuEntry("Save World",1000);
	glutAddMenuEntry("Load World",1001);
	glutAddMenuEntry("-------------------",-1);
	glutAddMenuEntry("Enter Debug Mode", 1005); //line contains mode-specific text, see menu function above
	glutAddMenuEntry("Reset Agents (tab)", 9);
	glutAddMenuEntry("Exit (esc)", 27);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void GLView::handleIdle()
{
	modcounter++;
	if (!paused) world->update();

	//show FPS
	int currentTime = glutGet( GLUT_ELAPSED_TIME );
	frames++;
	if ((currentTime - lastUpdate) >= 1000) {
		std::pair<int,int> num_herbs_carns = world->numHerbCarnivores();
		sprintf( buf, "FPS: %d speed: %d NumAgents: %d Carnivors: %d Herbivors: %d Epoch: %d",
			frames, skipdraw, world->numAgents(), num_herbs_carns.second, num_herbs_carns.first, world->epoch() );
		glutSetWindowTitle( buf );
		frames = 0;
		lastUpdate = currentTime;
	}
	if (skipdraw<=0 && draw) {
		clock_t endwait;
		float mult=-0.005*(skipdraw-1); //ugly, ah well
		endwait = clock () + mult * CLOCKS_PER_SEC ;
		while (clock() < endwait) {}
	}

	if (draw) {
		if (skipdraw>0) {
			if (modcounter%skipdraw==0) renderScene();	//increase fps by skipping drawing
		}
		else renderScene(); //we will decrease fps by waiting using clocks
	}
}

void GLView::renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glTranslatef(conf::WWIDTH/2, conf::WHEIGHT/2, 0.0f);	
	glScalef(scalemult, scalemult, 1.0f);
	
	if(following!=0){
		float xi=0, yi=0;
		world->positionOfInterest(following, xi, yi);

		if(xi!=0 && yi!=0){
			xtranslate+= 0.1*(-xi-xtranslate); ytranslate+= 0.1*(-yi-ytranslate);
		}
	}
	glTranslatef(xtranslate, ytranslate, 0.0f);
	
	world->draw(this, layer);

	glPopMatrix();
	glutSwapBuffers();
}

void GLView::drawAgent(const Agent& agent)
{
	float n;
	float r= conf::BOTRADIUS;
	float rp= conf::BOTRADIUS+2.5;

	//handle selected agent
	if (agent.selectflag>0) {

		//draw selection
		glBegin(GL_POLYGON);
		glColor4f(1,1,1,0.5);
		drawCircle(agent.pos.x, agent.pos.y, conf::BOTRADIUS+5);
		glEnd();

		glPushMatrix();
		glTranslatef(agent.pos.x-80,agent.pos.y+20,0);
		//draw inputs, outputs
		float col;
		float yy=15;
		float xx=15;
		float ss=16;
		glBegin(GL_QUADS);
		for (int j=0;j<INPUTSIZE;j++) {
			col= agent.in[j];
			glColor3f(col,col,col);
			glVertex3f(0+ss*j, 0, 0.0f);
			glVertex3f(xx+ss*j, 0, 0.0f);
			glVertex3f(xx+ss*j, yy, 0.0f);
			glVertex3f(0+ss*j, yy, 0.0f);
		}
		yy+=5;
		for (int j=0;j<OUTPUTSIZE;j++) {
			col= agent.out[j];
			glColor3f(col,col,col);
			glVertex3f(0+ss*j, yy, 0.0f);
			glVertex3f(xx+ss*j, yy, 0.0f);
			glVertex3f(xx+ss*j, yy+ss, 0.0f);
			glVertex3f(0+ss*j, yy+ss, 0.0f);
		}
		yy+=ss*2;

		//draw brain.
		
		float offx=0;
		ss=8;
		xx=ss;
		for (int j=0;j<BRAINSIZE;j++) {
			col = agent.brain.boxes[j].out;
			glColor3f(col,col,col);
			
			glVertex3f(offx+0+ss*j, yy, 0.0f);
			glVertex3f(offx+xx+ss*j, yy, 0.0f);
			glVertex3f(offx+xx+ss*j, yy+ss, 0.0f);
			glVertex3f(offx+ss*j, yy+ss, 0.0f);
			
			if ((j+1)%30==0) {
				yy+=ss;
				offx-=ss*30;
			}
		}
		
		
		/*
		glEnd();
		glBegin(GL_LINES);
		float offx=0;
		ss=30;
		xx=ss;
		for (int j=0;j<BRAINSIZE;j++) {
			for(int k=0;k<CONNS;k++){
				int j2= agent.brain.boxes[j].id[k];
				
				//project indices j and j2 into pixel space
				float x1= 0;
				float y1= 0;
				if(j<INPUTSIZE) { x1= j*ss; y1= yy; }
				else { 
					x1= ((j-INPUTSIZE)%30)*ss;
					y1= yy+ss+2*ss*((int) (j-INPUTSIZE)/30);
				}
				
				float x2= 0;
				float y2= 0;
				if(j2<INPUTSIZE) { x2= j2*ss; y2= yy; }
				else { 
					x2= ((j2-INPUTSIZE)%30)*ss;
					y2= yy+ss+2*ss*((int) (j2-INPUTSIZE)/30);
				}
				
				float ww= agent.brain.boxes[j].w[k];
				if(ww<0) glColor3f(-ww, 0, 0);
				else glColor3f(0,0,ww);
				
				glVertex3f(x1,y1,0);
				glVertex3f(x2,y2,0);
			}
		}
		*/

		glEnd();
		glPopMatrix();
	}

	//draw giving/receiving
	if(agent.dfood!=0){
		glBegin(GL_POLYGON);
		float mag=cap(abs(agent.dfood)/conf::FOODTRANSFER/3);
		if(agent.dfood>0) glColor3f(0,mag,0);
		else glColor3f(mag,0,0); //draw sharing as a thick green or red outline
		for (int k=0;k<17;k++){
			n = k*(M_PI/8);
			glVertex3f(agent.pos.x+rp*sin(n),agent.pos.y+rp*cos(n),0);
			n = (k+1)*(M_PI/8);
			glVertex3f(agent.pos.x+rp*sin(n),agent.pos.y+rp*cos(n),0);
		}
		glEnd();
	}

	//draw indicator of this agent... used for various events
	 if (agent.indicator>0) {
		 glBegin(GL_POLYGON);
		 glColor4f(agent.ir,agent.ig,agent.ib,0.5);
		 drawCircle(agent.pos.x, agent.pos.y, conf::BOTRADIUS+((int)agent.indicator));
		 glEnd();
	 }
	
	
	//draw eyes
	glBegin(GL_LINES);
	glColor3f(0.5,0.5,0.5);
	for(int q=0;q<NUMEYES;q++) {
		glVertex3f(agent.pos.x,agent.pos.y,0);
		float aa= agent.angle+agent.eyedir[q];
		glVertex3f(agent.pos.x+(conf::BOTRADIUS*4)*cos(aa),
				   agent.pos.y+(conf::BOTRADIUS*4)*sin(aa),
				   0);
	}
	glEnd();
	
	glBegin(GL_POLYGON); 
	//body
	glColor3f(agent.red,agent.gre,agent.blu);
	drawCircle(agent.pos.x, agent.pos.y, conf::BOTRADIUS);
	glEnd();

	glBegin(GL_LINES);
	//outline
	glColor3f(0,0,0);
	if (agent.boost) glColor3f(0.8,0,0); //draw boost as red outline
	if (agent.jump>0) glColor3f(0,0,0.8); //draw jumping as blue outline
	
	for (int k=0;k<17;k++)
	{
		n = k*(M_PI/8);
		glVertex3f(agent.pos.x+r*sin(n),agent.pos.y+r*cos(n),0);
		n = (k+1)*(M_PI/8);
		glVertex3f(agent.pos.x+r*sin(n),agent.pos.y+r*cos(n),0);
	}
	//and spike
	glColor3f(0.5,0,0);
	glVertex3f(agent.pos.x,agent.pos.y,0);
	glVertex3f(agent.pos.x+(3*r*agent.spikeLength)*cos(agent.angle),agent.pos.y+(3*r*agent.spikeLength)*sin(agent.angle),0);
	glEnd();

	//debug sight lines
	for (int i=0;i<world->linesA.size();i++) {
		glBegin(GL_LINES);
		glColor3f(1,1,1);
		glVertex3f(world->linesA[i].x,world->linesA[i].y,0);
		glVertex3f(world->linesB[i].x,world->linesB[i].y,0);
		glEnd();
	}
	world->linesA.resize(0);
	world->linesB.resize(0);

	if(scalemult > .3) {//hide extra visual data if really far away
		//health
		int xo=18;
		int yo=-15;
		glBegin(GL_QUADS);
		glColor3f(0,0,0);
		glVertex3f(agent.pos.x+xo,agent.pos.y+yo,0);
		glVertex3f(agent.pos.x+xo+5,agent.pos.y+yo,0);
		glVertex3f(agent.pos.x+xo+5,agent.pos.y+yo+40,0);
		glVertex3f(agent.pos.x+xo,agent.pos.y+yo+40,0);

		glColor3f(0,0.8,0);
		glVertex3f(agent.pos.x+xo,agent.pos.y+yo+20*(2-agent.health),0);
		glVertex3f(agent.pos.x+xo+5,agent.pos.y+yo+20*(2-agent.health),0);
		glVertex3f(agent.pos.x+xo+5,agent.pos.y+yo+40,0);
		glVertex3f(agent.pos.x+xo,agent.pos.y+yo+40,0);

		//hybrid marker
		if (agent.hybrid) {
			glColor3f(0,0,0.8);
			glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo,0);
			glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo,0);
			glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+10,0);
			glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+10,0);
		}

		//stomach dichotomy indicator
		glColor3f(1-agent.herbivore,agent.herbivore,0);
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+12,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+12,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+22,0);
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+22,0);

		//sound volume indicator
		glColor3f(agent.soundmul,agent.soundmul,agent.soundmul);
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+24,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+24,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+34,0);
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+34,0);

		//temp discomfort indicator
		//calculate temperature at the agents spot. (based on distance from horizontal equator) Orange is bad, white is good
		//convert into calc based on agent's spot on the cell grid?
		float dd= 2.0*abs(agent.pos.y/conf::HEIGHT - 0.5);
		float discomfort= cap(sqrt(abs(dd-agent.temperature_preference)));
		if (discomfort<0.08) discomfort=0;
		glColor3f(1,pow(2-discomfort,2)/4,(1-discomfort));
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+36,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+36,0);
		glVertex3f(agent.pos.x+xo+12,agent.pos.y+yo+46,0);
		glVertex3f(agent.pos.x+xo+6,agent.pos.y+yo+46,0);
	}

	glEnd();

	//print stats
	if(scalemult > .7) { // hide the number stats when zoomed out
		//generation count
		sprintf(buf2, "%i", agent.gencount);
		RenderString(agent.pos.x-conf::BOTRADIUS*1.5, agent.pos.y+conf::BOTRADIUS*1.8, GLUT_BITMAP_TIMES_ROMAN_24, buf2, 0.8f, 1.0f, 1.0f);

		//age
		sprintf(buf2, "%i", agent.age);
		float x = (float) agent.age/conf::MAXAGE; //will be redder the closer it is to MAXAGE
		if(x>1)x=1;
		RenderString(agent.pos.x-conf::BOTRADIUS*1.5, agent.pos.y+conf::BOTRADIUS*1.8+12, GLUT_BITMAP_TIMES_ROMAN_24, buf2, 0.8f, 1.0-x, 1.0-x);

		//health
		sprintf(buf2, "%.2f", agent.health);
		RenderString(agent.pos.x-conf::BOTRADIUS*1.5, agent.pos.y+conf::BOTRADIUS*1.8+24, GLUT_BITMAP_TIMES_ROMAN_24, buf2, 0.8f, 1.0f, 1.0f);

		//repcounter
		float dr = agent.metabolism/conf::MAXMETABOLISM; //red if high metabolism, blue if low 
		sprintf(buf2, "%.2f", agent.repcounter);
		RenderString(agent.pos.x-conf::BOTRADIUS*1.5, agent.pos.y+conf::BOTRADIUS*1.8+36, GLUT_BITMAP_TIMES_ROMAN_24, buf2, dr/2+0.5, dr/2+0.5, (1.0-dr)/2+0.5);
	}
}

void GLView::drawMisc()
{
	float mm = 3;
	//draw misc info
	glBegin(GL_LINES);
	glColor3f(0,0,0.8); //hybrid count
	for(int q=0;q<world->numHybrid.size()-1;q++) {
		if(q==world->ptr-1) continue;
		glVertex3f(q*10,-20 -mm*world->numHybrid[q],0);
		glVertex3f((q+1)*10,-20 -mm*world->numHybrid[q+1],0);
	}
	glColor3f(0,1,0); //herbivore count
	for(int q=0;q<world->numHerbivore.size()-1;q++) {
		if(q==world->ptr-1) continue;
		glVertex3f(q*10,-20 -mm*world->numHerbivore[q],0);
		glVertex3f((q+1)*10,-20 -mm*world->numHerbivore[q+1],0);
	}
	glColor3f(1,0,0); //carnivore count
	for(int q=0;q<world->numCarnivore.size()-1;q++) {
		if(q==world->ptr-1) continue;
		glVertex3f(q*10,-20 -mm*world->numCarnivore[q],0);
		glVertex3f((q+1)*10,-20 -mm*world->numCarnivore[q+1],0);
	}
	glColor3f(0,0,0); //total count
	for(int q=0;q<world->numTotal.size()-1;q++) {
		if(q==world->ptr-1) continue;
		glVertex3f(q*10,-20 -mm*world->numTotal[q],0);
		glVertex3f((q+1)*10,-20 -mm*world->numTotal[q+1],0);
	}
	glVertex3f(world->ptr*10,-20,0);
	glVertex3f(world->ptr*10,-20 -mm*world->numAgents(),0);
	glEnd();
	sprintf(buf2, "%i", world->numAgents());
	RenderString(world->ptr*10 + 3,-20 -mm*world->numAgents(), GLUT_BITMAP_TIMES_ROMAN_24, buf2, 0.0f, 0.0f, 0.0f);
	
	RenderString(2700, -80, GLUT_BITMAP_TIMES_ROMAN_24, "Right click the world for actions. 'm' disables drawing, 'p' pauses the sim.", 0.0f, 0.0f, 0.0f);
	RenderString(2700, -20, GLUT_BITMAP_TIMES_ROMAN_24, "Try clicking on a bot and using 'w,a,s,d' to control it.", 0.0f, 0.0f, 0.0f);
	if(paused) RenderString(3500, -80, GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED", 0.0f, 0.0f, 0.0f);
	if(following!=0) RenderString(-xtranslate+(10-conf::WWIDTH/2)/scalemult, -ytranslate+(30-conf::WHEIGHT/2)/scalemult, GLUT_BITMAP_TIMES_ROMAN_24, "FOLLOWING", 0.5f, 0.5f, 0.5f);
	if(world->isClosed()) RenderString(4000, -80, GLUT_BITMAP_TIMES_ROMAN_24, "CLOSED WORLD", 0.0f, 0.0f, 0.0f);
}

void GLView::drawCell(int x, int y, float quantity)
{
	if (layer!=0) { //none: white
		glBegin(GL_QUADS);
		if (layer==1) { //plant food: green w/ navy blue background
			glColor4f(0.0,quantity,0.1,0.5);
		} else if (layer==2) { //meat food: red/burgundy w/ navy blue bg
			glColor4f(quantity,0.0,0.1,0.5);
		} else if (layer==3) { //hazards: purple/magenta w/ navy blue bg
			glColor4f(quantity,0.0,quantity*0.9+0.1,0.5);
		} else if (layer==4) { //temperature: orange for 1, navy blue for 0
			glColor4f(quantity,quantity/4,0.1-0.1*quantity,0.5);
		}
		glVertex3f(x*conf::CZ,y*conf::CZ,0);
		glVertex3f(x*conf::CZ+conf::CZ,y*conf::CZ,0);
		glVertex3f(x*conf::CZ+conf::CZ,y*conf::CZ+conf::CZ,0);
		glVertex3f(x*conf::CZ,y*conf::CZ+conf::CZ,0);
		glEnd();
	}
}

void GLView::setWorld(World* w)
{
	world = w;
}
