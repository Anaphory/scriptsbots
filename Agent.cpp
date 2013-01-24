#include "Agent.h"

#include "settings.h"
#include "helpers.h"
#include <stdio.h>
//#include <iostream>
#include <string>
#include "DWRAONBrain.h"
#include "MLPBrain.h"
#include "AssemblyBrain.h"

using namespace std;
Agent::Agent()
{
	//randomly spawned bots get the following attributes:
	pos= Vector2f(randf(0,conf::WIDTH),randf(0,conf::HEIGHT));
	angle= randf(-M_PI,M_PI);
	health= 0.65;
	max_health= health;
	age=0;
	species=randi(-10,10);
	spikeLength=0;
	red= 0;
	gre= 0;
	blu= 0;
	w1=0;
	w2=0;
	soundmul=1;
	give=0;
	clockf1= randf(5,100);
	clockf2= randf(5,100);
	boost=false;
	jump= 0;
	indicator=0;
	gencount=0;
	selectflag=0;
	ir=0;
	ig=0;
	ib=0;
	temperature_preference= cap(randn(2.0*abs(pos.y/conf::HEIGHT - 0.5),0.05));
	hybrid= false;
	herbivore= randf(0,1);
	repcounter= conf::REPRATE;
	metabolism= 1; //randf(0.2,conf::MAXMETABOLISM);
	// If a low metabolism has no advantage, why bother?

	id=0;
	
	smellmod= randf(0.1, 0.5);
	soundmod= randf(0.2, 0.6);
	hearmod= randf(0.7, 1.3);
	eyesensmod= randf(1, 3);
	bloodmod= randf(1, 3);
	
	MUTRATE1= 0.01; //randf(0.001, 0.005);
	MUTRATE2= 0.01; //randf(0.01, 0.5);

	spiked= false;
	
	in.resize(INPUTSIZE, 0);
	out.resize(OUTPUTSIZE, 0);
	
	eyefov.resize(NUMEYES, 0);
	eyedir.resize(NUMEYES, 0);
	for(int i=0;i<NUMEYES;i++) {
		eyefov[i] = randf(0.5, 2);
		eyedir[i] = randf(0, 2*M_PI);
	}
}

void Agent::printSelf()
{
	printf("Agent species=%i\n", species);
	for (int i=0;i<mutations.size();i++) {
		cout << mutations[i];
	}
}

void Agent::initEvent(float size, float r, float g, float b)
{
	indicator=size;
	ir=r;
	ig=g;
	ib=b;
}

void Agent::tick()
{
	brain.tick(in, out);
}
Agent Agent::reproduce(Agent that, float MR, float MR2)
{
	//create baby. Note that if the bot selects itself to mate with, this function acts also as asexual reproduction
	//NOTES: Agent "this" is mother, Agent "that" is father, Agent "a2" is daughter
	Agent a2;

	//spawn the baby somewhere closeby behind agent
	//we want to spawn behind so that agents dont accidentally eat their young right away
	Vector2f fb(conf::BOTRADIUS,0);
	fb.rotate(-a2.angle);
	a2.pos= this->pos + fb + Vector2f(randf(-conf::BOTRADIUS*2,conf::BOTRADIUS*2), randf(-conf::BOTRADIUS*2,conf::BOTRADIUS*2));
	if (a2.pos.x<0) a2.pos.x= conf::WIDTH+a2.pos.x;
	if (a2.pos.x>conf::WIDTH) a2.pos.x= a2.pos.x-conf::WIDTH;
	if (a2.pos.y<0) a2.pos.y= conf::HEIGHT+a2.pos.y;
	if (a2.pos.y>conf::HEIGHT) a2.pos.y= a2.pos.y-conf::HEIGHT;

	//basic trait inheritance
	a2.gencount= max(this->gencount+1,that.gencount+1);
	a2.metabolism= 1; //randf(0,1)<0.5 ? this->metabolism : that.metabolism;
	// If a low metabolism has no advantage, why bother?
	a2.herbivore= randf(0,1)<0.5 ? this->herbivore : that.herbivore;
	a2.species= randf(0,1)<0.5 ? this->species : that.species;

	a2.MUTRATE1= randf(0,1)<0.5 ? this->MUTRATE1 : that.MUTRATE1;
	a2.MUTRATE2= randf(0,1)<0.5 ? this->MUTRATE2 : that.MUTRATE2;
	a2.clockf1= randf(0,1)<0.5 ? this->clockf1 : that.clockf1;
	a2.clockf2= randf(0,1)<0.5 ? this->clockf2 : that.clockf2;

	a2.smellmod = randf(0,1)<0.5 ? this->smellmod : that.smellmod;
	a2.soundmod = randf(0,1)<0.5 ? this->soundmod : that.soundmod;
	a2.hearmod = randf(0,1)<0.5 ? this->hearmod : that.hearmod;
	a2.eyesensmod = randf(0,1)<0.5 ? this->eyesensmod : that.eyesensmod;
	a2.bloodmod = randf(0,1)<0.5 ? this->bloodmod : that.bloodmod;

	a2.temperature_preference= randf(0,1)<0.5 ? this->temperature_preference : that.temperature_preference;
	
	a2.eyefov = randf(0,1)<0.5 ? this->eyefov : that.eyefov;
	a2.eyedir = randf(0,1)<0.5 ? this->eyedir : that.eyedir;

	//mutations

	// If a low metabolism has no advantage, why bother?
	//if (randf(0,1)<MR/2) a2.metabolism= randn(a2.metabolism, MR2);
	//if (a2.metabolism<0) a2.metabolism= 0; //not going to bother limiting to 0.1; if it can survive, I don't even care if it's 0.000...1
	//if (a2.metabolism>conf::MAXMETABOLISM) a2.metabolism= conf::MAXMETABOLISM;

	if (randf(0,1)<MR) a2.herbivore= cap(randn(a2.herbivore, MR2));
	if (randf(0,1)<MR*20) a2.species= (int) randn(a2.species, 100*MR2);

	if (randf(0,1)<MR) a2.MUTRATE1= randn(a2.MUTRATE1, conf::METAMUTRATE1);
	if (randf(0,1)<MR) a2.MUTRATE2= randn(a2.MUTRATE2, conf::METAMUTRATE2);
	if (a2.MUTRATE1<0) a2.MUTRATE1= 0;
	if (a2.MUTRATE2<0) a2.MUTRATE2= 0;

	if (randf(0,1)<MR*4) a2.clockf1= randn(a2.clockf1, MR2/2);
	if (a2.clockf1<2) a2.clockf1= 2;
	if (randf(0,1)<MR*4) a2.clockf2= randn(a2.clockf2, MR2/2);
	if (a2.clockf2<2) a2.clockf2= 2;

	if (randf(0,1)<MR) a2.smellmod= randn(a2.smellmod, MR2);
	if (randf(0,1)<MR) a2.soundmod= randn(a2.soundmod, MR2);
	if (randf(0,1)<MR) a2.hearmod= randn(a2.hearmod, MR2);
	if (randf(0,1)<MR) a2.eyesensmod= randn(a2.eyesensmod, MR2);
	if (randf(0,1)<MR) a2.bloodmod= randn(a2.bloodmod, MR2);

	if (randf(0,1)<MR/2) a2.temperature_preference= cap(randn(a2.temperature_preference, MR2));

	for(int i=0;i<NUMEYES;i++){
		if(randf(0,1)<MR/2) a2.eyefov[i] = randn(a2.eyefov[i], MR2);
		if(a2.eyefov[i]<0) a2.eyefov[i] = 0;
		if(a2.eyefov[i]>M_PI) a2.eyefov[i] = M_PI; //eyes cannot wrap around bot
		
		if(randf(0,1)<MR) a2.eyedir[i] = randn(a2.eyedir[i], MR2);
		if(a2.eyedir[i]<0) a2.eyedir[i] = 0;
		if(a2.eyedir[i]>2*M_PI) a2.eyedir[i] = 2*M_PI;
	}
	
	//create brain here
	a2.brain= this->brain.crossover(that.brain);
	a2.brain.mutate(MR,MR2);
	
	return a2;

}
