//Read/Write module. Built by Julian R. Hershey, 2012. Use as you wish. Beware of human-made errors!

//The way this works is it includes agent.h, world.h, and glview.h, and is included by glview.cpp.
//I was unable to have this between world.h and glview.h. If you can find a better way to use this,
//please do so and let me know @ julian.hershey@gmail.com

//Currently, GLView has to create an instance of this class whenever saving or loading is needed (see GLView::menu)
//once that is done, the world and other data must be fed to the functions of ReadWrite in order to fully save all data
//It's not pretty, but it works

//Data can be saved in any file format, as the data is put into raw text form. I tried to emulate XML, but I did not
//strictly follow those rules. There must be a space between the equals sign and the data being loaded, or there
//will be failures. Also, cells use a different style of data retreival, so beware. I've made use of a mode var
//to better control when certain objects and data should be expected.

//For loading agents: I've used <agent> and </agent> tags to outline individual agents. Upon loading, a fake agent
//is created, all attributes which have data to be retrieved are set, and the </agent> tag signals the copy of the
//fake agent's variables into a real agent placed in the world array.

//IN THE FILES: Order does not matter, as each line is read individually from the others. HOWEVER, cell data MUST
//be contained inside <cells>, and agent data MUST be included in its own <agent>. If something goes wrong and 
//two lines modify the same attribute, the latter one will be used.

//VERSION CHECKING: I've included simple version checking ability to help users. If versionnum doesn't equal the code's 
//preprogramed CURRENTVERSION, then a caution message is displayed. As of now, however, the system seems to deal
//with missing data quite harmlessly; in the case of agent vars missing, init data values are assigned; in the case of
//world data, nothing can break down because those values are set by the program. Cells may break since it's more 
//complex, so beware. Once loading and saving of cell sizes, world sizes, and brain specifications become enabled, 
//then I forsee possible problems, but for now, I've not had any major errors.
//Even too much data (eg some old data that is no longer used by the program) isn't a
//big issue; simply remove the already removed variable and the line check for it.
//I strongly suggest that one study this code before modifying it. If help is needed on the definitions of built-in
//functions, I found http://www.cplusplus.com/reference/ to be a great reference for such things.

#include "ReadWrite.h"

#include "settings.h"
#include "helpers.h"
#include <stdio.h>
#include <iostream>

using namespace std;

ReadWrite::ReadWrite()
{
	ourfile= "WORLD.SCB";
}

/*void ReadWrite::getOurFile() const
{
	// Displays an OpenFileDialog so the user can select a Cursor.
	OpenFileDialog ^ openFileDialog1 = new OpenFileDialog();
	openFileDialog1->Filter = "ScriptBots Saves|*.SCB";
	openFileDialog1->Title = "Select a save file";

	// Show the Dialog.
	// If the user clicked OK in the dialog and
	// a .SCB file was selected, open it.
	if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		// Assign ourfile to be the name of this file
		ourfile= 
	}
}i*/

void ReadWrite::saveWorld(World *world, float xpos, float ypos, const char *filename)
{
	//if no filename given, use default
	if(!filename){
		filename= ourfile;
		printf("CAUTION: No filename given. Loading default %s instead.\n", ourfile);
	}		
	
	//check the filename given to see if it exists yet
	FILE* ck = fopen(filename, "r");
	if(ck){
		printf("CAUTION: %s already exists! Nothing I can do atm, sorry.\n", filename);
	}
	//open save file, write over any previous
	FILE* fs = fopen(filename, "w");
	ourfile= filename;
	
	//start with saving world vars and food layer
	float f= 0;
	fprintf(fs,"<world>\n");
	fprintf(fs,"version= %i\n", CURRENTVERSION);
	fprintf(fs,"inputs= %i\n", INPUTSIZE);
	fprintf(fs,"outputs= %i\n", OUTPUTSIZE);
	fprintf(fs,"brainsize= %i\n", BRAINSIZE);
	fprintf(fs,"connections= %i\n", CONNS);
	fprintf(fs,"width= %i\n", conf::WIDTH);
	fprintf(fs,"height= %i\n", conf::HEIGHT);
	fprintf(fs,"cellsize= %i\n", conf::CZ);
	fprintf(fs,"epoch= %i\n", world->current_epoch);
	fprintf(fs,"mod= %i\n", world->modcounter);
	fprintf(fs,"xpos= %f\n", xpos);
	fprintf(fs,"ypos= %f\n", ypos); //GLView xtranslate and ytranslate
	for(int cx=0;cx<world->CW;cx++){ //start with the layers
		for(int cy=0;cy<world->CH;cy++){
			float food= world->cells[0][cx][cy];
			float meat= world->cells[1][cx][cy];
			float hazard= world->cells[2][cx][cy];
			if(food>0 || meat>0 || hazard>0){ //only cells which have data on at least one layer are written, because all cells are set to zero on world reset
				fprintf(fs, "<cell>\n"); //signals the writting of a cell
				fprintf(fs, "cx= %i\n", cx);
				fprintf(fs, "cy= %i\n", cy);
				if (food>0) fprintf(fs, "food= %f\n", food); //to further save space, we needn't report a value of a layer for the cell if it's zero
				if (meat>0) fprintf(fs, "meat= %f\n", meat);
				if (hazard>0) fprintf(fs, "hazard= %f\n", hazard);
				fprintf(fs,"</cell>\n");
			}
		}
	}
	int mini= -1;
	for(int i=0;i<world->agents.size();i++) {
		// here we save all agents. All simulation-significant data must be stored, from the pos and angle, to the change in food the bot was experiencing
		Agent* a= &world->agents[i];

		fprintf(fs, "<agent>\n"); //signals the writing of a new agent
	//	fprintf(fs, "id= %i\n", a->id); //id not loaded
		fprintf(fs, "posx= %f\nposy= %f\n", a->pos.x, a->pos.y);
		fprintf(fs, "angle= %f\n", a->angle);
		fprintf(fs, "health= %f\n", a->health);
	//	fprintf(fs, "red= %f\ngre= %f\nblu= %f\n", a->red, a->gre, a->blu);
	//	fprintf(fs, "w1= %f\nw2= %f\n", w1, w2);
	//	fprintf(fs, "boost= %i\n", (int) a->boost);
		fprintf(fs, "herbivore= %f\n", a->herbivore);
		fprintf(fs, "species= %i\n", a->species);
		fprintf(fs, "spike= %f\n", a->spikeLength);
		fprintf(fs, "jump= %f\n", a->jump); //version 6 addition
		fprintf(fs, "dfood= %f\n", a->dfood);
		fprintf(fs, "age= %i\n", a->age);
		fprintf(fs, "gen= %i\n", a->gencount);
		fprintf(fs, "hybrid= %i\n", (int) a->hybrid);
		fprintf(fs, "cl1= %f\ncl2= %f\n", a->clockf1, a->clockf2);
		fprintf(fs, "smellmod= %f\n", a->smellmod);
		fprintf(fs, "soundmod= %f\n", a->soundmod);
		fprintf(fs, "hearmod= %f\n", a->hearmod);
		fprintf(fs, "bloodmod= %f\n", a->bloodmod);
		fprintf(fs, "eyesensemod= %f\n", a->eyesensmod);
		for(int q=0;q<NUMEYES;q++) {
			fprintf(fs, "<eye>\n");
			fprintf(fs, "eye#= %i\n", q);
			fprintf(fs, "eyedir= %f\n", a->eyedir[q]);
			fprintf(fs, "eyefov= %f\n", a->eyefov[q]);
			fprintf(fs, "</eye>\n");
		}
		fprintf(fs, "metabolism= %f\n", a->metabolism);
		fprintf(fs, "repcounter= %f\n", a->repcounter);
		fprintf(fs, "killed= %i\n", (int) a->spiked);
		fprintf(fs, "temppref= %f\n", a->temperature_preference);
	//	fprintf(fs, "indicator= %f\n", a->indicator);
	//	fprintf(fs, "ir= %f\nig= %f\nib= %f\n", a->ir, a->ig, a->ib);
	//	fprintf(fs, "give= %f\n", a->give);
		fprintf(fs, "mutrate1= %f\nmutrate2= %f\n", a->MUTRATE1, a->MUTRATE2);
		fprintf(fs, "<brain>\n"); //signals the writing of the brain (more for organization than proper loading)

		for(int b=0;b<BRAINSIZE;b++){
			fprintf(fs, "<box>\n"); //signals the writing of a specific numbered box
			fprintf(fs, "box#= %i\n", b);
			fprintf(fs, "kp= %f\n", a->brain.boxes[b].kp);
			fprintf(fs, "bias= %f\n", a->brain.boxes[b].bias);
			fprintf(fs, "globalw= %f\n", a->brain.boxes[b].gw);
			fprintf(fs, "target= %f\n", a->brain.boxes[b].target);
			fprintf(fs, "out= %f\n", a->brain.boxes[b].out);
			fprintf(fs, "oldout= %f\n", a->brain.boxes[b].oldout);
			for(int c=0;c<CONNS;c++){
				fprintf(fs, "<conn>\n"); //signals the writing of a specific connection for a specific box
				fprintf(fs, "conn#= %i\n", c);
				fprintf(fs, "type= %i\n", a->brain.boxes[b].type[c]);
				fprintf(fs, "w= %f\n", a->brain.boxes[b].w[c]);
				fprintf(fs, "cid= %i\n", a->brain.boxes[b].id[c]);
				fprintf(fs, "</conn>\n"); //end of connection
			}
			fprintf(fs, "</box>\n"); //end of box
		}
		fprintf(fs, "</brain>\n"); //end of brain
		fprintf(fs, "</agent>\n"); //end of agent
		if(a->selectflag) mini= i;
	}
	fprintf(fs,"selected= %i\n", mini); //write down which bot was selected
	fprintf(fs,"</world>");
	fclose(fs);
}

void ReadWrite::loadWorld(World *world, float &xtranslate, float &ytranslate, const char *filename)
{
	char line[64], *pos;
	char var[16];
	char dataval[16];
	int cxl;
	int cyl;
	int mode= -1;//loading mode: -1= off, 0= world, 1= cell, 2= agent, 3= box, 4= connection, 5= eyes
	int versionnum= CURRENTVERSION;

	Agent xa; //mock agent. should get moved and deleted after loading

	int eyenum= -1; //counters
	int boxnum= -1;
	int connnum= -1;
	int i; //integer buffer
	float f; //float buffer

	//if no filename given, use default
	if(!filename){
		filename= ourfile;
		printf("CAUTION: No filename given. Loading default %s instead.\n", ourfile);
	}

	//version check.
	if (versionnum!=CURRENTVERSION){
		printf("CAUTION: version number of the save file (%i) does not match current version number (%i). World data may be incompatable and/or bots may not behave as expected!\n", versionnum, CURRENTVERSION);
	}

	FILE *fl;
	fl= fopen(filename, "r");
	if(fl){
		while(!feof(fl)){
			fgets(line, sizeof(line), fl);
			pos= strtok(line,"\n");
			sscanf(line, "%s%s", var, dataval);
			//version 5 (7/2012) has changed cell loading to follow the same pattern, so dataval2 and var2 have been removed

			if(mode==-1){ //mode @ -1 = off
				if(strcmp(var, "<world>")==0){ //strcmp= 0 when the arguements equal
					//if we find a <world> tag, enable world loading. simple
					mode= 0;
				}
			}else if(mode==0){ //mode @ 0 = world
				if(strcmp(var, "version=")==0){
					//set versionnum to dataval to check version for new/removed features
					sscanf(dataval, "%i", &i);
					versionnum= i;
				}else if(strcmp(var, "inputs=")==0){
					//inputs count; NOT CURRENTLY USED
				}else if(strcmp(var, "outputs=")==0){
					//outputs count; NOT CURRENTLY USED
				}else if(strcmp(var, "brainsize=")==0){
					//brainsize count; NOT CURRENTLY USED
				}else if(strcmp(var, "connections=")==0){
					//conns count; NOT CURRENTLY USED
				}else if(strcmp(var, "width=")==0){
					//world width; NOT CURRENTLY USED
//					sscanf(dataval, "%i", &i);
//					conf::WIDTH= i;
				}else if(strcmp(var, "height=")==0){
					//world height; NOT CURRENTLY USED
//					sscanf(dataval, "%i", &i);
//					conf::HEIGHT= i;
				}else if(strcmp(var, "cellsize=")==0){
					//CZ; NOT CURRENTLY USED
//					sscanf(dataval, "%i", &i);
//					conf::CZ= i;
				}else if(strcmp(var, "epoch=")==0){
					//epoch
					sscanf(dataval, "%i", &i);
					world->current_epoch= i;
				}else if(strcmp(var, "mod=")==0){
					//mod count
					sscanf(dataval, "%i", &i);
					world->modcounter= i;
				}else if(strcmp(var, "xpos=")==0){
					//veiw screen location x
					sscanf(dataval, "%f", &f);
					xtranslate= f;
				}else if(strcmp(var, "ypos=")==0){
					//veiw screen location y
					sscanf(dataval, "%f", &f);
					ytranslate= f;
				}else if(strcmp(var, "selected=")==0){
					//selected agent
					sscanf(dataval, "%i", &i);
					if (i>=0 && i<world->agents.size()) world->agents[i].selectflag= true;
				}else if(strcmp(var, "<cell>")==0){ //version 5 (9/2012) has changed cell loading to follow the same pattern as that of agents
					//cells tag activates cell reading mode
					mode= 1;
				}else if(strcmp(var, "<agent>")==0){
					//agent tag activates agent reading mode
					mode= 2;
				}
			}else if(mode==1){ //mode @ 1 = cell
				//version 3 (6/2012) no longer allows old food data to be loaded. Version 5 (9/2012) has changed structure of save, so old cell data is worthless
				if(strcmp(var, "</cell>")==0){
					//end_cell tag is checked for first, because of else condition
					mode= 0;
				}else if(strcmp(var, "cx=")==0){
					sscanf(dataval, "%i", &i);
					cxl= i;
				}else if(strcmp(var, "cy=")==0){
					sscanf(dataval, "%i", &i);
					cyl= i;
				}else if(strcmp(var, "food=")==0){
					sscanf(dataval, "%f", &f);
					world->cells[0][cxl][cyl]= f;
				}else if(strcmp(var, "meat=")==0){
					sscanf(dataval, "%f", &f);
					world->cells[1][cxl][cyl]= f;
				}else if(strcmp(var, "hazard=")==0){
					sscanf(dataval, "%f", &f);
					world->cells[2][cxl][cyl]= f;
				}				
			}else if(mode==2){ //mode @ 2 = agent
				if(strcmp(var, "</agent>")==0){
					//end_agent tag is checked for, and when found, copies agent xa
					mode= 0;
					Agent loadee = xa;
					loadee.id= world->idcounter;
					world->idcounter++;
					world->agents.push_back(loadee);
				}else if(strcmp(var, "posx=")==0){
					sscanf(dataval, "%f", &f);
					xa.pos.x= f;
				}else if(strcmp(var, "posy=")==0){
					sscanf(dataval, "%f", &f);
					xa.pos.y= f;
				}else if(strcmp(var, "angle=")==0){
					sscanf(dataval, "%f", &f);
					xa.angle= f;
				}else if(strcmp(var, "health=")==0){
					sscanf(dataval, "%f", &f);
					xa.health= f;
				}else if(strcmp(var, "herbivore=")==0){
					sscanf(dataval, "%f", &f);
					xa.herbivore= f;
				}else if(strcmp(var, "species=")==0){
					//new as of the version released 6/2012
					sscanf(dataval, "%i", &i);
					xa.species= i;
				}else if(strcmp(var, "spike=")==0){
					sscanf(dataval, "%f", &f);
					xa.spikeLength= f;
				}else if(strcmp(var, "jump=")==0){
					//new as of version 5 (9/2012)
					sscanf(dataval, "%f", &f);
					xa.jump= f;
				}else if(strcmp(var, "dfood=")==0){
					sscanf(dataval, "%f", &f);
					xa.dfood= f;
				}else if(strcmp(var, "age=")==0){
					sscanf(dataval, "%i", &i);
					xa.age= i;
				}else if(strcmp(var, "gen=")==0){
					sscanf(dataval, "%i", &i);
					xa.gencount= i;
				}else if(strcmp(var, "hybrid=")==0){
					sscanf(dataval, "%i", &i);
					if(i==1) xa.hybrid= true;
					else xa.hybrid= false;
				}else if(strcmp(var, "cl1=")==0){
					sscanf(dataval, "%f", &f);
					xa.clockf1= f;
				}else if(strcmp(var, "cl2=")==0){
					sscanf(dataval, "%f", &f);
					xa.clockf2= f;
				}else if(strcmp(var, "smellmod=")==0){
					sscanf(dataval, "%f", &f);
					xa.smellmod= f;
				}else if(strcmp(var, "soundmod=")==0){
					sscanf(dataval, "%f", &f);
					xa.soundmod= f;
				}else if(strcmp(var, "hearmod=")==0){
					sscanf(dataval, "%f", &f);
					xa.hearmod= f;
				}else if(strcmp(var, "bloodmod=")==0){
					sscanf(dataval, "%f", &f);
					xa.bloodmod= f;
				}else if(strcmp(var, "eyesensemod=")==0){
					sscanf(dataval, "%f", &f);
					xa.eyesensmod= f;
				}else if(strcmp(var, "metabolism=")==0){
					sscanf(dataval, "%f", &f);
					xa.metabolism= f;
				}else if(strcmp(var, "repcounter=")==0){
					sscanf(dataval, "%f", &f);
					xa.repcounter= f;
				}else if(strcmp(var, "killed=")==0){
					// v 5/2012: added if killed check
					sscanf(dataval, "%i", &i);
					if (i==1) xa.spiked= true;
					else xa.spiked= false;
				}else if(strcmp(var, "temppref=")==0){
					sscanf(dataval, "%f", &f);
					xa.temperature_preference= f;
				}else if(strcmp(var, "mutrate1=")==0){
					sscanf(dataval, "%f", &f);
					xa.MUTRATE1= f;
				}else if(strcmp(var, "mutrate2=")==0){
					sscanf(dataval, "%f", &f);
					xa.MUTRATE2= f;
				}else if(strcmp(var, "<eye>")==0){
					mode= 5;
				}else if(strcmp(var, "<box>")==0){
					mode= 3;
				}
			}else if(mode==5){ //mode @ 5 = eye (of agent)
				if(strcmp(var, "</eye>")==0){
					mode= 2;
				}else if(strcmp(var, "eye#=")==0){
					sscanf(dataval, "%i", &i);
					eyenum= i;
				}else if(strcmp(var, "eyedir=")==0){
					sscanf(dataval, "%f", &f);
					xa.eyedir[eyenum]= f;
				}else if(strcmp(var, "eyefov=")==0){
					sscanf(dataval, "%f", &f);
					xa.eyefov[eyenum]= f;
				}
			}else if(mode==3){ //mode @ 3 = brain box (of agent)
				if(strcmp(var, "</box>")==0){
					mode= 2;
				}else if(strcmp(var, "box#=")==0){
					sscanf(dataval, "%i", &i);
					boxnum= i;
				}else if(strcmp(var, "kp=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].kp= f;
				}else if(strcmp(var, "bias=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].bias= f;
				}else if(strcmp(var, "globalw=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].gw= f;
				}else if(strcmp(var, "target=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].target= f;
				}else if(strcmp(var, "out=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].out= f;
				}else if(strcmp(var, "oldout=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].oldout= f;
				}else if(strcmp(var, "<conn>")==0){
					mode= 4;
				}
			}else if(mode==4){ //mode @ 4 = connection (of brain box of agent)
				if(strcmp(var, "</conn>")==0){
					mode= 3;
				}else if(strcmp(var, "conn#=")==0){
					sscanf(dataval, "%i", &i);
					connnum= i;
				}else if(strcmp(var, "type=")==0){
					sscanf(dataval, "%i", &i);
					xa.brain.boxes[boxnum].type[connnum]= i;
				}else if(strcmp(var, "w=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].w[connnum]= f;
				}else if(strcmp(var, "cid=")==0){
					sscanf(dataval, "%f", &f);
					xa.brain.boxes[boxnum].id[connnum]= f;
				}
			}
		}
		fclose(fl);
	} else { //DOH! the file doesn't exist!
		printf("ERROR: Save file specified (%s) doesn't exist!\n", filename);
	}

	world->setInputs();
	world->brainsTick();
}
