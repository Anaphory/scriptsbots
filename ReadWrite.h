#ifndef READWRITE_H
#define READWRITE_H

#include "Agent.h"
#include "World.h"
#include "GLView.h"
#include <stdio.h>
#include "settings.h"

//#include <Windows.h>

class ReadWrite
{

public:
	ReadWrite();

	void getOurFile() const;
	void setOurFile() const;

	void saveWorld(World *world, float xpos, float ypos, const char *filename); //save world
	void loadWorld(World *world, float &xtranslate, float &ytranslate, const char *filename); //load world

private:
	const char *ourfile;
	void saveAgent(World &w, const Agent &agent); //save a single agent
//	Agent transagent; //???
};

#endif // READWRITE_H
