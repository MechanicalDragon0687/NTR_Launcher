/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <nds.h>
#include <fat.h>
#include <nds/fifocommon.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <list>

#include "inifile.h"
#include "bootsplash.h"
#include "nds_card.h"
#include "launch_engine.h"
#include "crc.h"
#include "version.h" 

// volatile u32* SCFG_EXT = (volatile u32*)0x4004008;
// volatile u32* SCFG_MC = (volatile u32*)0x4004010;
// volatile u32* SCFG_ROM = (volatile u32*)0x4004000;

int main(int argc, const char* argv[]) {

	REG_SCFG_CLK = 0x85;
	REG_SCFG_EXT = 0x8307F100;

	// NTR Mode/Splash used by default
	bool UseNTRSplash = true;

	swiWaitForVBlank();

	dsi_forceTouchDsmode();

	u32 ndsHeader[0x80];
	char gameid[4];
	uint32_t headerCRC;
	
	scanKeys();
	int pressed = keysDown();

	if (fatInitDefault()) {
		CIniFile ntrlauncher_config( "sd:/nds/ntr_forwarder.ini" );
		
		if(ntrlauncher_config.GetInt("NTRFORWARDER","NTRCLOCK",0) == 0) { UseNTRSplash = false; }

		if(ntrlauncher_config.GetInt("NTRFORWARDER","DISABLEANIMATION",0) == 1) {
			if(REG_SCFG_MC == 0x11) { BootSplashInit(UseNTRSplash); } else { if( UseNTRSplash == true ) { REG_SCFG_CLK = 0x80; } }
		} else {
			if( pressed & KEY_B ) { if(REG_SCFG_MC == 0x11) { BootSplashInit(UseNTRSplash); } } else { BootSplashInit(UseNTRSplash); }
		}

	} else {
		if ( pressed & KEY_B ) { if(REG_SCFG_MC == 0x11) { BootSplashInit(UseNTRSplash); } } else { BootSplashInit(UseNTRSplash); }
	}

	// Tell Arm7 to start Cart Reset
	fifoSendValue32(FIFO_USER_01, 1);
	// Wait for Arm7 to finish Cart Reset
	fifoWaitValue32(FIFO_USER_03);

	// Wait for card to stablize before continuing
	for (int i = 0; i < 20; i++) { swiWaitForVBlank(); }

	sysSetCardOwner (BUS_OWNER_ARM9);

	getHeader (ndsHeader);

	for (int i = 0; i < 20; i++) { swiWaitForVBlank(); }
	
	memcpy (gameid, ((const char*)ndsHeader) + 12, 4);
	headerCRC = crc32((const char*)ndsHeader, sizeof(ndsHeader));

	FILE* inifile;
	struct VariablesToSave {
	char var1[10];
	u16 var1nl;
	char var2[273];
	u16 var2nl;
	char var3[9];
	u16 var3nl;
	} IniFile;

	strcpy(IniFile.var1,"[Dir Info]");
	IniFile.var1nl = 0x0A0D;
	strcpy(IniFile.var2,"fullName=fat1:/<<<Start NDS Path                                                                                                                                                                                                                                  End NDS Path>>>");
	IniFile.var2nl = 0x0A0D;
	strcpy(IniFile.var3,"RomType=0");
	IniFile.var3nl = 0x0A0D;
	
	inifile = fopen("sd:/_dstwofwd/autoboot.ini","wb");
	fwrite(&IniFile,1,sizeof(IniFile),inifile);
	fclose(inifile);	
	
	while(1) {
		if(REG_SCFG_MC == 0x11) { 
		break; } else {
			runLaunchEngine ();
		}
	}
	return 0;
}
