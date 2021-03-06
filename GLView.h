#ifndef GLVIEW_H
#define GLVIEW_H


#include "View.h"
#include "World.h"

class GLView;

extern GLView* GLVIEW;

void gl_processNormalKeys(unsigned char key, int x, int y);
void gl_processSpecialKeys(int key, int x, int y);
void gl_processReleasedKeys(unsigned char key, int x, int y);
void gl_menu(int key);
void gl_processMouse(int button, int state, int x, int y);
void gl_processMouseActiveMotion(int x, int y);
void gl_processMousePassiveMotion(int x, int y);
void gl_changeSize(int w, int h);
void gl_handleIdle();
void gl_renderScene();

class GLView : public View
{

public:
    GLView(World* w);
    virtual ~GLView();
    
    virtual void drawAgent(const Agent &a);
    virtual void drawCell(int x, int y, float quantity);
    virtual void drawMisc();
    
    void setWorld(World* w);
    
    //GLUT functions
    void processNormalKeys(unsigned char key, int x, int y);
	void processSpecialKeys(int key, int x, int y);
	void processReleasedKeys(unsigned char key, int x, int y);
	void menu(int key);
	void menuS(int key);
    void processMouse(int button, int state, int x, int y);
    void processMouseActiveMotion(int x, int y);
	void processMousePassiveMotion(int x, int y);
    void changeSize(int w, int h);
    void handleIdle();
    void renderScene();

	void glCreateMenu(void);
	int m_id; //main context menu
    
private:
    
    World *world;
    bool paused; //are we paused?
    bool draw; //are we drawing?
	bool debug; //are we debugging?
    int skipdraw; //are we skipping some frames?
	int layer; //what cell layer is currently active? 0= off, 1= plant food, 2= meat, 3= hazards, 4= temperature
    char buf[100];
    char buf2[10];
    int modcounter; //tick counter
    int lastUpdate;
    int frames;
    
    
    float scalemult;
    float xtranslate, ytranslate;
    int downb[3];
    int mousex, mousey;
    
    int following;
	char filename [30];
    
};

#endif // GLVIEW_H
