/*-----------------------------------------------------------------

 Copyright (C) 2010  Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include <nds.h>
#include <nds/fifocommon.h>
#include <fat.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "nds_loader_arm9.h"

#include "inifile.h"

int main( int argc, char **argv) {

	//consoleDemoInit();
	if (fatInitDefault()) {
	
		if( access( "sd:/_nds/dstwoautoboot.ini", F_OK ) != -1 ) {
			FILE * inifilesd, * inifileslot1;
			int numr,numw;
			struct VariablesToRead {
			char var1[10];
			u16 var1nl;
			char var2[273];
			u16 var2nl;
			char var3[9];
			u16 var3nl;
			} IniFile;

			
			inifilesd = fopen("sd:/_nds/dstwoautoboot.ini","rb");
			inifileslot1 = fopen("fat:/_dstwo/autoboot.ini","wb");
			numr = fread(&IniFile,1,sizeof(IniFile),inifilesd);
			numw = fwrite(&IniFile,1,numr,inifileslot1);
			fclose(inifilesd);
			fclose(inifileslot1);
			
			if( access( "sd:/_nds/twloader/settings.ini", F_OK ) != -1 ) {
				CIniFile twloaderini( "sd:/_nds/twloader/settings.ini" );
				
				if(twloaderini.GetInt("TWL-MODE","SLOT1_KEEPSD",0) == 1) {
					// printf("SD access on\n");
					twloaderini.SetInt("TWL-MODE","SLOT1_KEEPSD",0);	// Set to 0 to lock SD access from forwarders
					twloaderini.SaveIniFile( "sd:/_nds/twloader/settings.ini");
				} else {
					// printf("SD access off\n");
					// Tell arm7 to lock SCFG_EXT
					fifoSendValue32(FIFO_USER_01, 1);
					if (REG_SCFG_EXT == 0x83002000)
						REG_SCFG_EXT = 0x03002000;
					else
						REG_SCFG_EXT = 0x03000000;
				}
			} else {
				// printf("SD access off\n");
				// Tell arm7 to lock SCFG_EXT
				fifoSendValue32(FIFO_USER_01, 1);
				if (REG_SCFG_EXT == 0x83002000)
					REG_SCFG_EXT = 0x03002000;
				else
					REG_SCFG_EXT = 0x03000000;
			}

			chdir("fat:/");
			runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL);

		} else {
			runNdsFile("fat:/_dstwo/_dstwo.nds", 0, NULL);
		}
	
	} else {
		consoleDemoInit();
		printf("FAT init failed!\n");
	}

	while(1) {
	swiWaitForVBlank(); }
}
