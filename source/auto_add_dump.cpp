#include "auto_add_dump.h"

unsigned char *decompSzsMalloc(unsigned char *src, unsigned int *ddestSize){
    unsigned int destSize = 0;
    memcpy(&destSize, src + 4, 4);
    *ddestSize = destSize;
    unsigned char* dest = (unsigned char*)calloc(destSize, sizeof(unsigned char));
    unsigned char* dest2 = dest;
    unsigned char group_head = 0; // group header byte ...
    int group_head_len  = 0; // ... and it's length to manage groups

    //https://wiki.tockdom.com/wiki/YAZ0_(File_Format)#Decompression
    unsigned char *dest_end = dest + destSize;
    src += 0x10;
    while (dest < dest_end )
    {
        if (!group_head_len)
        {
	    //*** start a new data group and read the group header byte.

            group_head = *src++;
            group_head_len = 8;
        }

        group_head_len--;
        if ( group_head & 0x80 )
        {
            //*** bit in group header byte is set -> copy 1 byte direct

        *dest++ = *src++;
        }
        else
        {
        //*** bit in group header byte is not set -> run length encoding

	    // read the first 2 bytes of the chunk
            const unsigned char b1 = *src++;
            const unsigned char b2 = *src++;
        
	    // calculate the source position
            const unsigned char * copy_src = dest - (( b1 & 0x0f ) << 8 | b2 ) - 1;

	    // calculate the number of bytes to copy.
            int n = b1 >> 4;

            if (!n)
                n = *src++ + 0x12; // N==0 -> read third byte
            else
                n += 2; // add 2 to length

	    // a validity check
            if (dest + n > dest_end ){
                free(dest2);
                return NULL;
            }

	        // copy chunk data.
            // don't use memcpy() or memmove() here because
            // they don't work with self referencing chunks.
            while ( n-- > 0 )
                *dest++ = *copy_src++;
        }

        // shift group header byte
        group_head <<= 1;
    }
    return dest2;
}

void freeConfig(szs_subfile_dump_config* config){
    unsigned int index = 0;
    while(1){
        if((config + index)->szs_filename == NULL)break;
        free((config + index)->subfile_filename);
        index++;
    }
    free(config);
}

szs_subfile_dump_config* getAutoAddDumpConfig(void){
    //https://github.com/mkw-sp/mkw-sp/blob/623553448a7760825909ed3f0748911751c7124b/payload/sp/WU8Library.cc#L33
    szs_subfile_dump_config *config = (szs_subfile_dump_config*)calloc(0x3A, sizeof(szs_subfile_dump_config));
    (config + 0)->szs_filename = "castle_course.szs";
    (config + 0)->subfile_filename = (const char**)calloc(10, sizeof(const char*));
    *((config + 0)->subfile_filename + 0) = "./Mdush.brres";
    *((config + 0)->subfile_filename + 1) = "./Mdush.kcl";
    *((config + 0)->subfile_filename + 2) = "./castleballoon1.brres";
    *((config + 0)->subfile_filename + 3) = "./castleflower1.brres";
    *((config + 0)->subfile_filename + 4) = "./effect/wanwan/rk_wanwan.breff";
    *((config + 0)->subfile_filename + 5) = "./effect/wanwan/rk_wanwan.breft";
    *((config + 0)->subfile_filename + 6) = "./group_enemy_e.brres";
    *((config + 0)->subfile_filename + 7) = "./group_enemy_f.brres";
    *((config + 0)->subfile_filename + 8) = "./wanwan.brres";
    *((config + 0)->subfile_filename + 9) = NULL;
    (config + 1)->szs_filename = "farm_course.szs";
    (config + 1)->subfile_filename = (const char**)calloc(16, sizeof(const char*));
    *((config + 1)->subfile_filename + 0) = "./FlagA2.brres";
    *((config + 1)->subfile_filename + 1) = "./brasd/cow/eat.brasd";
    *((config + 1)->subfile_filename + 2) = "./brasd/cow/eat_ed.brasd";
    *((config + 1)->subfile_filename + 3) = "./brasd/cow/run.brasd";
    *((config + 1)->subfile_filename + 4) = "./brasd/cow/surprise.brasd";
    *((config + 1)->subfile_filename + 5) = "./brasd/cow/walk.brasd";
    *((config + 1)->subfile_filename + 6) = "./castletree1.brres";
    *((config + 1)->subfile_filename + 7) = "./castletree2.brres";
    *((config + 1)->subfile_filename + 8) = "./choropu.brres";
    *((config + 1)->subfile_filename + 9) = "./cow.brres";
    *((config + 1)->subfile_filename + 10) = "./effect/choropu/rk_choropuMoko.breff";
    *((config + 1)->subfile_filename + 11) = "./effect/choropu/rk_choropuMoko.breft";
    *((config + 1)->subfile_filename + 12) = "./posteffect/posteffect2.bblm";
    *((config + 1)->subfile_filename + 13) = "./posteffect/posteffect3.bblm";
    *((config + 1)->subfile_filename + 14) = "./windmill.brres";
    *((config + 1)->subfile_filename + 15) = NULL;
    (config + 2)->szs_filename = "kinoko_course.szs";
    (config + 2)->subfile_filename = (const char**)calloc(12, sizeof(const char*));
    *((config + 2)->subfile_filename + 0) = "./brasd/kuribo/walk_l.brasd";
    *((config + 2)->subfile_filename + 1) = "./brasd/kuribo/walk_r.brasd";
    *((config + 2)->subfile_filename + 2) = "./kinoko.brres";
    *((config + 2)->subfile_filename + 3) = "./kinokoT1.brres";
    *((config + 2)->subfile_filename + 4) = "./kinoko_d_g.kcl";
    *((config + 2)->subfile_filename + 5) = "./kinoko_d_r.kcl";
    *((config + 2)->subfile_filename + 6) = "./kinoko_g.kcl";
    *((config + 2)->subfile_filename + 7) = "./kinoko_r.kcl";
    *((config + 2)->subfile_filename + 8) = "./kuribo.brres";
    *((config + 2)->subfile_filename + 9) = "./posteffect/posteffect2.blight";
    *((config + 2)->subfile_filename + 10) = "./posteffect/posteffect2.blmap";
    *((config + 2)->subfile_filename + 11) = NULL;
    (config + 3)->szs_filename = "volcano_course.szs";
    (config + 3)->subfile_filename = (const char**)calloc(71, sizeof(const char*));
    *((config + 3)->subfile_filename + 0) = "./FireSnake.brres";
    *((config + 3)->subfile_filename + 1) = "./FlamePole_v.brres";
    *((config + 3)->subfile_filename + 2) = "./VolcanoBall1.brres";
    *((config + 3)->subfile_filename + 3) = "./VolcanoPiece0.kcl";
    *((config + 3)->subfile_filename + 4) = "./VolcanoPiece0b.kcl";
    *((config + 3)->subfile_filename + 5) = "./VolcanoPiece0c.kcl";
    *((config + 3)->subfile_filename + 6) = "./VolcanoPiece1.brres";
    *((config + 3)->subfile_filename + 7) = "./VolcanoPiece1.kcl";
    *((config + 3)->subfile_filename + 8) = "./VolcanoPiece11.kcl";
    *((config + 3)->subfile_filename + 9) = "./VolcanoPiece11b.kcl";
    *((config + 3)->subfile_filename + 10) = "./VolcanoPiece11c.kcl";
    *((config + 3)->subfile_filename + 11) = "./VolcanoPiece13.kcl";
    *((config + 3)->subfile_filename + 12) = "./VolcanoPiece13b.kcl";
    *((config + 3)->subfile_filename + 13) = "./VolcanoPiece13c.kcl";
    *((config + 3)->subfile_filename + 14) = "./VolcanoPiece14.kcl";
    *((config + 3)->subfile_filename + 15) = "./VolcanoPiece14b.kcl";
    *((config + 3)->subfile_filename + 16) = "./VolcanoPiece14c.kcl";
    *((config + 3)->subfile_filename + 17) = "./VolcanoPiece16.kcl";
    *((config + 3)->subfile_filename + 18) = "./VolcanoPiece16b.kcl";
    *((config + 3)->subfile_filename + 19) = "./VolcanoPiece16c.kcl";
    *((config + 3)->subfile_filename + 20) = "./VolcanoPiece18.kcl";
    *((config + 3)->subfile_filename + 21) = "./VolcanoPiece18b.kcl";
    *((config + 3)->subfile_filename + 22) = "./VolcanoPiece18c.kcl";
    *((config + 3)->subfile_filename + 23) = "./VolcanoPiece1b.kcl";
    *((config + 3)->subfile_filename + 24) = "./VolcanoPiece1c.kcl";
    *((config + 3)->subfile_filename + 25) = "./VolcanoPiece2.kcl";
    *((config + 3)->subfile_filename + 26) = "./VolcanoPiece2b.kcl";
    *((config + 3)->subfile_filename + 27) = "./VolcanoPiece2c.kcl";
    *((config + 3)->subfile_filename + 28) = "./VolcanoPiece3.kcl";
    *((config + 3)->subfile_filename + 29) = "./VolcanoPiece3b.kcl";
    *((config + 3)->subfile_filename + 30) = "./VolcanoPiece3c.kcl";
    *((config + 3)->subfile_filename + 31) = "./VolcanoPiece4.kcl";
    *((config + 3)->subfile_filename + 32) = "./VolcanoPiece4b.kcl";
    *((config + 3)->subfile_filename + 33) = "./VolcanoPiece4c.kcl";
    *((config + 3)->subfile_filename + 34) = "./VolcanoPiece5.kcl";
    *((config + 3)->subfile_filename + 35) = "./VolcanoPiece5b.kcl";
    *((config + 3)->subfile_filename + 36) = "./VolcanoPiece5c.kcl";
    *((config + 3)->subfile_filename + 37) = "./VolcanoPiece7.kcl";
    *((config + 3)->subfile_filename + 38) = "./VolcanoPiece7b.kcl";
    *((config + 3)->subfile_filename + 39) = "./VolcanoPiece7c.kcl";
    *((config + 3)->subfile_filename + 40) = "./VolcanoPiece8.kcl";
    *((config + 3)->subfile_filename + 41) = "./VolcanoPiece8b.kcl";
    *((config + 3)->subfile_filename + 42) = "./VolcanoPiece8c.kcl";
    *((config + 3)->subfile_filename + 43) = "./VolcanoPiece9.kcl";
    *((config + 3)->subfile_filename + 44) = "./VolcanoPiece9b.kcl";
    *((config + 3)->subfile_filename + 45) = "./VolcanoPiece9c.kcl";
    *((config + 3)->subfile_filename + 46) = "./VolcanoRock1.brres";
    *((config + 3)->subfile_filename + 47) = "./VolcanoRock1.kcl";
    *((config + 3)->subfile_filename + 48) = "./VolcanoRock2.kcl";
    *((config + 3)->subfile_filename + 49) = "./effect/EnvFire/rk_EnvFire.breff";
    *((config + 3)->subfile_filename + 50) = "./effect/EnvFire/rk_EnvFire.breft";
    *((config + 3)->subfile_filename + 51) = "./effect/FireSnake/rk_fireSnake.breff";
    *((config + 3)->subfile_filename + 52) = "./effect/FireSnake/rk_fireSnake.breft";
    *((config + 3)->subfile_filename + 53) = "./effect/EnvFire/rk_EnvFire.breff";
    *((config + 3)->subfile_filename + 54) = "./effect/EnvFire/rk_EnvFire.breft";
    *((config + 3)->subfile_filename + 55) = "./effect/FlamePole_v/rk_flamePole.breff";
    *((config + 3)->subfile_filename + 56) = "./effect/FlamePole_v/rk_flamePole.breft";
    *((config + 3)->subfile_filename + 57) = "./effect/VolcanoBall1/rk_volc.breff";
    *((config + 3)->subfile_filename + 58) = "./effect/VolcanoBall1/rk_volc.breft";
    *((config + 3)->subfile_filename + 59) = "./effect/pochaYogan/rk_pochaYogan.breff";
    *((config + 3)->subfile_filename + 60) = "./effect/pochaYogan/rk_pochaYogan.breft";
    *((config + 3)->subfile_filename + 61) = "./effect/taimatsu/rk_taimatsu.breff";
    *((config + 3)->subfile_filename + 62) = "./effect/taimatsu/rk_taimatsu.breft";
    *((config + 3)->subfile_filename + 63) = "./effect/taimatsu/rk_taimatsu.breff";
    *((config + 3)->subfile_filename + 64) = "./effect/taimatsu/rk_taimatsu.breft";
    *((config + 3)->subfile_filename + 65) = "./effect/taimatsu/rk_taimatsu.breff";
    *((config + 3)->subfile_filename + 66) = "./effect/volsmk/rk_volsmk.breff";
    *((config + 3)->subfile_filename + 67) = "./effect/volsmk/rk_volsmk.breft";
    *((config + 3)->subfile_filename + 68) = "./pochaYogan.brres";
    *((config + 3)->subfile_filename + 69) = "./posteffect/posteffect.bti";
    *((config + 3)->subfile_filename + 70) = NULL;
    (config + 4)->szs_filename = "factory_course.szs";
    (config + 4)->subfile_filename = (const char**)calloc(30, sizeof(const char*));
    *((config + 4)->subfile_filename + 0) = "./BeltCrossing.brres";
    *((config + 4)->subfile_filename + 1) = "./BeltCurveA.brres";
    *((config + 4)->subfile_filename + 2) = "./BeltEasy.brres";
    *((config + 4)->subfile_filename + 3) = "./Block.brres";
    *((config + 4)->subfile_filename + 4) = "./Crane.brres";
    *((config + 4)->subfile_filename + 5) = "./Crane.kcl";
    *((config + 4)->subfile_filename + 6) = "./MiiSighKino.brres";
    *((config + 4)->subfile_filename + 7) = "./Piston.brres";
    *((config + 4)->subfile_filename + 8) = "./Press.brres";
    *((config + 4)->subfile_filename + 9) = "./bulldozer_left.brres";
    *((config + 4)->subfile_filename + 10) = "./bulldozer_left.kcl";
    *((config + 4)->subfile_filename + 11) = "./bulldozer_right.brres";
    *((config + 4)->subfile_filename + 12) = "./bulldozer_right.kcl";
    *((config + 4)->subfile_filename + 13) = "./effect/Alarm/rk_alarm.breff";
    *((config + 4)->subfile_filename + 14) = "./effect/Alarm/rk_alarm.breft";
    *((config + 4)->subfile_filename + 15) = "./effect/Block/rk_block.breff";
    *((config + 4)->subfile_filename + 16) = "./effect/Block/rk_block.breft";
    *((config + 4)->subfile_filename + 17) = "./effect/Press/rk_press.breff";
    *((config + 4)->subfile_filename + 18) = "./effect/Press/rk_press.breft";
    *((config + 4)->subfile_filename + 19) = "./effect/Steam/rk_steam.breff";
    *((config + 4)->subfile_filename + 20) = "./effect/Steam/rk_steam.breft";
    *((config + 4)->subfile_filename + 21) = "./effect/pocha/rk_pocha.breff";
    *((config + 4)->subfile_filename + 22) = "./effect/pocha/rk_pocha.breft";
    *((config + 4)->subfile_filename + 23) = "./effect/woodbox/rk_woodbox.breff";
    *((config + 4)->subfile_filename + 24) = "./effect/woodbox/rk_woodbox.breft";
    *((config + 4)->subfile_filename + 25) = "./group_enemy_c.brres";
    *((config + 4)->subfile_filename + 26) = "./pocha.brres";
    *((config + 4)->subfile_filename + 27) = "./seagull.brres";
    *((config + 4)->subfile_filename + 28) = "./woodbox.brres";
    *((config + 4)->subfile_filename + 29) = NULL;
    (config + 5)->szs_filename = "shopping_course.szs";
    (config + 5)->subfile_filename = (const char**)calloc(16, sizeof(const char*));
    *((config + 5)->subfile_filename + 0) = "./CarA1.brres";
    *((config + 5)->subfile_filename + 1) = "./CarA2.brres";
    *((config + 5)->subfile_filename + 2) = "./CarA3.brres";
    *((config + 5)->subfile_filename + 3) = "./CarB.brres";
    *((config + 5)->subfile_filename + 4) = "./PalmTree.brres";
    *((config + 5)->subfile_filename + 5) = "./ShMiiObj01.brres";
    *((config + 5)->subfile_filename + 6) = "./ShMiiObj02.brres";
    *((config + 5)->subfile_filename + 7) = "./ShMiiObj03.brres";
    *((config + 5)->subfile_filename + 8) = "./escalator.brres";
    *((config + 5)->subfile_filename + 9) = "./escalator.kcl";
    *((config + 5)->subfile_filename + 10) = "./group_monte_a.brres";
    *((config + 5)->subfile_filename + 11) = "./mii_balloon.brres";
    *((config + 5)->subfile_filename + 12) = "./miiposter.brres";
    *((config + 5)->subfile_filename + 13) = "./monte_a.brres";
    *((config + 5)->subfile_filename + 14) = "./parasol.brres";
    *((config + 5)->subfile_filename + 15) = NULL;
    (config + 6)->szs_filename = "boardcross_course.szs";
    (config + 6)->subfile_filename = (const char**)calloc(16, sizeof(const char*));
    *((config + 6)->subfile_filename + 0) = "./DonkyCannon_wii.brres";
    *((config + 6)->subfile_filename + 1) = "./K_chairlift00.brres";
    *((config + 6)->subfile_filename + 2) = "./K_sticklift00.brres";
    *((config + 6)->subfile_filename + 3) = "./MiiStatueM2.brres";
    *((config + 6)->subfile_filename + 4) = "./brasd/DonkyCannon_wii/DonkyCannon_wii.brasd";
    *((config + 6)->subfile_filename + 5) = "./dk_miiobj00.brres";
    *((config + 6)->subfile_filename + 6) = "./dkmonitor.brres";
    *((config + 6)->subfile_filename + 7) = "./effect/DonkyCannon_wii/rk_dkCannon.breff";
    *((config + 6)->subfile_filename + 8) = "./effect/DonkyCannon_wii/rk_dkCannon.breft";
    *((config + 6)->subfile_filename + 9) = "./effect/EnvSnow/rk_EnvSnow.breff";
    *((config + 6)->subfile_filename + 10) = "./effect/EnvSnow/rk_EnvSnow.breft";
    *((config + 6)->subfile_filename + 11) = "./effect/Flash_B/rk_flashBCross.breff";
    *((config + 6)->subfile_filename + 12) = "./effect/Flash_B/rk_flashBCross.breft";
    *((config + 6)->subfile_filename + 13) = "./heyho.brres";
    *((config + 6)->subfile_filename + 14) = "./heyho2.brres";
    *((config + 6)->subfile_filename + 15) = NULL;
    (config + 7)->szs_filename = "truck_course.szs";
    (config + 7)->subfile_filename = (const char**)calloc(11, sizeof(const char*));
    *((config + 7)->subfile_filename + 0) = "./FlagA1.brres";
    *((config + 7)->subfile_filename + 1) = "./FlagB1.brres";
    *((config + 7)->subfile_filename + 2) = "./MiiSignWario.brres";
    *((config + 7)->subfile_filename + 3) = "./TruckWagon.brres";
    *((config + 7)->subfile_filename + 4) = "./basabasa.brres";
    *((config + 7)->subfile_filename + 5) = "./effect/TruckWagon/rk_truck.breff";
    *((config + 7)->subfile_filename + 6) = "./effect/TruckWagon/rk_truck.breft";
    *((config + 7)->subfile_filename + 7) = "./effect/truckChimSmk/rk_truckChimSmk.breff";
    *((config + 7)->subfile_filename + 8) = "./effect/truckChimSmk/rk_truckChimSmk.breft";
    *((config + 7)->subfile_filename + 9) = "./truckChimSmk.brres";
    *((config + 7)->subfile_filename + 10) = NULL;
    (config + 8)->szs_filename = "beginner_course.szs";
    (config + 8)->subfile_filename = (const char**)calloc(15, sizeof(const char*));
    *((config + 8)->subfile_filename + 0) = "./FlagB2.brres";
    *((config + 8)->subfile_filename + 1) = "./MiiKanban.brres";
    *((config + 8)->subfile_filename + 2) = "./MiiObj01.brres";
    *((config + 8)->subfile_filename + 3) = "./MiiObj02.brres";
    *((config + 8)->subfile_filename + 4) = "./MiiObj03.brres";
    *((config + 8)->subfile_filename + 5) = "./MiiStatueL3.brres";
    *((config + 8)->subfile_filename + 6) = "./dokan_sfc.brres";
    *((config + 8)->subfile_filename + 7) = "./effect/Flash_L/rk_flashBeginner.breff";
    *((config + 8)->subfile_filename + 8) = "./effect/Flash_L/rk_flashBeginner.breft";
    *((config + 8)->subfile_filename + 9) = "./effect/Hanabi/rk_stHanabi.breff";
    *((config + 8)->subfile_filename + 10) = "./effect/Hanabi/rk_stHanabi.breft";
    *((config + 8)->subfile_filename + 11) = "./itembox.brres";
    *((config + 8)->subfile_filename + 12) = "./skyship.brres";
    *((config + 8)->subfile_filename + 13) = "./sun.brres";
    *((config + 8)->subfile_filename + 14) = NULL;
    (config + 9)->szs_filename = "senior_course.szs";
    (config + 9)->subfile_filename = (const char**)calloc(15, sizeof(const char*));
    *((config + 9)->subfile_filename + 0) = "./MiiObjD01.brres";
    *((config + 9)->subfile_filename + 1) = "./MiiObjD02.brres";
    *((config + 9)->subfile_filename + 2) = "./MiiObjD03.brres";
    *((config + 9)->subfile_filename + 3) = "./MiiStatueBD1.brres";
    *((config + 9)->subfile_filename + 4) = "./MiiStatueBL1.brres";
    *((config + 9)->subfile_filename + 5) = "./MiiStatueD1.brres";
    *((config + 9)->subfile_filename + 6) = "./MiiStatueL1.brres";
    *((config + 9)->subfile_filename + 7) = "./cruiser.brres";
    *((config + 9)->subfile_filename + 8) = "./effect/cruiser/rk_cruiser.breff";
    *((config + 9)->subfile_filename + 9) = "./effect/cruiser/rk_cruiser.breft";
    *((config + 9)->subfile_filename + 10) = "./group_enemy_a.brres";
    *((config + 9)->subfile_filename + 11) = "./group_enemy_b.brres";
    *((config + 9)->subfile_filename + 12) = "./light_house.brres";
    *((config + 9)->subfile_filename + 13) = "./pylon01.brres";
    *((config + 9)->subfile_filename + 14) = NULL;
    (config + 0xA)->szs_filename = "ridgehighway_course.szs";
    (config + 0xA)->subfile_filename = (const char**)calloc(13, sizeof(const char*));
    *((config + 0xA)->subfile_filename + 0) = "./K_bomb_car.brres";
    *((config + 0xA)->subfile_filename + 1) = "./K_car_body.brres";
    *((config + 0xA)->subfile_filename + 2) = "./K_truck.brres";
    *((config + 0xA)->subfile_filename + 3) = "./Ksticketc.brres";
    *((config + 0xA)->subfile_filename + 4) = "./RhMiiKanban.brres";
    *((config + 0xA)->subfile_filename + 5) = "./effect/Fall_MH/rk_ridgeFall.breff";
    *((config + 0xA)->subfile_filename + 6) = "./effect/Fall_MH/rk_ridgeFall.breft";
    *((config + 0xA)->subfile_filename + 7) = "./effect/K_bomb_car/rk_K_bomb_car.breff";
    *((config + 0xA)->subfile_filename + 8) = "./effect/K_bomb_car/rk_K_bomb_car.breft";
    *((config + 0xA)->subfile_filename + 9) = "./effect/pochaMori/rk_pochaLeaf.breff";
    *((config + 0xA)->subfile_filename + 10) = "./effect/pochaMori/rk_pochaLeaf.breft";
    *((config + 0xA)->subfile_filename + 11) = "./ridgemii00.brres";
    *((config + 0xA)->subfile_filename + 12) = NULL;
    (config + 0xB)->szs_filename = "treehouse_course.szs";
    (config + 0xB)->subfile_filename = (const char**)calloc(20, sizeof(const char*));
    *((config + 0xB)->subfile_filename + 0) = "./ami.brres";
    *((config + 0xB)->subfile_filename + 1) = "./brasd/hanachan/head_wait2.brasd";
    *((config + 0xB)->subfile_filename + 2) = "./brasd/hanachan/walk.brasd";
    *((config + 0xB)->subfile_filename + 3) = "./brasd/tree_cannon/tree_cannon.brasd";
    *((config + 0xB)->subfile_filename + 4) = "./effect/EnvKareha/rk_EnvKareha.breff";
    *((config + 0xB)->subfile_filename + 5) = "./effect/EnvKareha/rk_EnvKareha.breft";
    *((config + 0xB)->subfile_filename + 6) = "./effect/EnvKarehaUp/rk_EnvKarehaUp.breff";
    *((config + 0xB)->subfile_filename + 7) = "./effect/EnvKarehaUp/rk_EnvKarehaUp.breft";
    *((config + 0xB)->subfile_filename + 8) = "./effect/hanachan/rk_hana.breff";
    *((config + 0xB)->subfile_filename + 9) = "./effect/hanachan/rk_hana.breft";
    *((config + 0xB)->subfile_filename + 10) = "./effect/karehayama/rk_karehayama.breff";
    *((config + 0xB)->subfile_filename + 11) = "./effect/karehayama/rk_karehayama.breft";
    *((config + 0xB)->subfile_filename + 12) = "./effect/leaf_effect/rk_leaf_effect.breff";
    *((config + 0xB)->subfile_filename + 13) = "./effect/leaf_effect/rk_leaf_effect.breft";
    *((config + 0xB)->subfile_filename + 14) = "./effect/tree_cannon/rk_dkCannon.breff";
    *((config + 0xB)->subfile_filename + 15) = "./effect/tree_cannon/rk_dkCannon.breft";
    *((config + 0xB)->subfile_filename + 16) = "./hanachan.brres";
    *((config + 0xB)->subfile_filename + 17) = "./karehayama.brres";
    *((config + 0xB)->subfile_filename + 18) = "./tree_cannon.brres";
    *((config + 0xB)->subfile_filename + 19) = NULL;
    (config + 0xC)->szs_filename = "koopa_course.szs";
    (config + 0xC)->subfile_filename = (const char**)calloc(18, sizeof(const char*));
    *((config + 0xC)->subfile_filename + 0) = "./FlamePole.brres";
    *((config + 0xC)->subfile_filename + 1) = "./FlamePole.kcl";
    *((config + 0xC)->subfile_filename + 2) = "./TwistedWay.brres";
    *((config + 0xC)->subfile_filename + 3) = "./WLfirebarGC.brres";
    *((config + 0xC)->subfile_filename + 4) = "./brasd/koopaFigure/vomit.brasd";
    *((config + 0xC)->subfile_filename + 5) = "./brasd/koopaFigure/wait.brasd";
    *((config + 0xC)->subfile_filename + 6) = "./course.0";
    *((config + 0xC)->subfile_filename + 7) = "./dossun.brres";
    *((config + 0xC)->subfile_filename + 8) = "./effect/FlamePole/rk_flamePole.breff";
    *((config + 0xC)->subfile_filename + 9) = "./effect/FlamePole/rk_flamePole.breft";
    *((config + 0xC)->subfile_filename + 10) = "./effect/dossun/rk_dossun.breff";
    *((config + 0xC)->subfile_filename + 11) = "./effect/dossun/rk_dossun.breft";
    *((config + 0xC)->subfile_filename + 12) = "./effect/koopaBall/rk_koopaBall.breff";
    *((config + 0xC)->subfile_filename + 13) = "./effect/koopaBall/rk_koopaBall.breft";
    *((config + 0xC)->subfile_filename + 14) = "./koopaBall.brres";
    *((config + 0xC)->subfile_filename + 15) = "./koopaFigure.brres";
    *((config + 0xC)->subfile_filename + 16) = "./koopaFirebar.brres";
    *((config + 0xC)->subfile_filename + 17) = NULL;
    (config + 0xD)->szs_filename = "rainbow_course.szs";
    (config + 0xD)->subfile_filename = (const char**)calloc(12, sizeof(const char*));
    *((config + 0xD)->subfile_filename + 0) = "./EarthRing.brres";
    *((config + 0xD)->subfile_filename + 1) = "./InsekiA.brres";
    *((config + 0xD)->subfile_filename + 2) = "./InsekiB.brres";
    *((config + 0xD)->subfile_filename + 3) = "./KmoonZ.brres";
    *((config + 0xD)->subfile_filename + 4) = "./SpaceSun.brres";
    *((config + 0xD)->subfile_filename + 5) = "./StarRing.brres";
    *((config + 0xD)->subfile_filename + 6) = "./aurora.brres";
    *((config + 0xD)->subfile_filename + 7) = "./effect/StarRing/rk_StarRing.breff";
    *((config + 0xD)->subfile_filename + 8) = "./effect/StarRing/rk_StarRing.breft";
    *((config + 0xD)->subfile_filename + 9) = "./effect/entry/rk_entry.breff";
    *((config + 0xD)->subfile_filename + 10) = "./effect/entry/rk_entry.breft";
    *((config + 0xD)->subfile_filename + 11) = NULL;
    (config + 0xE)->szs_filename = "desert_course.szs";
    (config + 0xE)->subfile_filename = (const char**)calloc(15, sizeof(const char*));
    *((config + 0xE)->subfile_filename + 0) = "./MiiSphinxY2.brres";
    *((config + 0xE)->subfile_filename + 1) = "./brasd/sanbo/revive.brasd";
    *((config + 0xE)->subfile_filename + 2) = "./dc_pillar.brres";
    *((config + 0xE)->subfile_filename + 3) = "./dc_pillar.kcl";
    *((config + 0xE)->subfile_filename + 4) = "./dc_pillar_base.kcl";
    *((config + 0xE)->subfile_filename + 5) = "./dc_sandcone.brres";
    *((config + 0xE)->subfile_filename + 6) = "./dc_sandcone.kcl";
    *((config + 0xE)->subfile_filename + 7) = "./effect/dc_pillar/rk_dc_pillar.breff";
    *((config + 0xE)->subfile_filename + 8) = "./effect/dc_pillar/rk_dc_pillar.breft";
    *((config + 0xE)->subfile_filename + 9) = "./effect/dc_sandcone/rk_dc_sandcone.breff";
    *((config + 0xE)->subfile_filename + 10) = "./effect/dc_sandcone/rk_dc_sandcone.breft";
    *((config + 0xE)->subfile_filename + 11) = "./effect/sanbo/rk_sanbo.breff";
    *((config + 0xE)->subfile_filename + 12) = "./effect/sanbo/rk_sanbo.breft";
    *((config + 0xE)->subfile_filename + 13) = "./sanbo.brres";
    *((config + 0xE)->subfile_filename + 14) = NULL;
    (config + 0xF)->szs_filename = "water_course.szs";
    (config + 0xF)->subfile_filename = (const char**)calloc(15, sizeof(const char*));
    *((config + 0xF)->subfile_filename + 0) = "./Epropeller.brres";
    *((config + 0xF)->subfile_filename + 1) = "./FlagA5.brres";
    *((config + 0xF)->subfile_filename + 2) = "./FlagB3.brres";
    *((config + 0xF)->subfile_filename + 3) = "./FlagB4.brres";
    *((config + 0xF)->subfile_filename + 4) = "./MiiSignNoko.brres";
    *((config + 0xF)->subfile_filename + 5) = "./UtsuboDokan.brres";
    *((config + 0xF)->subfile_filename + 6) = "./effect/Epropeller/rk_epropeller.breff";
    *((config + 0xF)->subfile_filename + 7) = "./effect/Epropeller/rk_epropeller.breft";
    *((config + 0xF)->subfile_filename + 8) = "./effect/FallBsA/rk_fallbasin.breff";
    *((config + 0xF)->subfile_filename + 9) = "./effect/FallBsA/rk_fallbasin.breft";
    *((config + 0xF)->subfile_filename + 10) = "./effect/FallBsB/rk_fallbasin.breff";
    *((config + 0xF)->subfile_filename + 11) = "./effect/FallBsB/rk_fallbasin.breft";
    *((config + 0xF)->subfile_filename + 12) = "./moray.brres";
    *((config + 0xF)->subfile_filename + 13) = "./pukupuku.brres";
    *((config + 0xF)->subfile_filename + 14) = NULL;
    (config + 0x10)->szs_filename = "old_peach_gc.szs";
    (config + 0x10)->subfile_filename = (const char**)calloc(13, sizeof(const char*));
    *((config + 0x10)->subfile_filename + 0) = "./PeachHunsuiGC.brres";
    *((config + 0x10)->subfile_filename + 1) = "./Psea.brres";
    *((config + 0x10)->subfile_filename + 2) = "./brasd/poihana/throw.brasd";
    *((config + 0x10)->subfile_filename + 3) = "./brasd/poihana/walk.brasd";
    *((config + 0x10)->subfile_filename + 4) = "./effect/PeachHunsuiGC/rk_hunsui.breff";
    *((config + 0x10)->subfile_filename + 5) = "./effect/PeachHunsuiGC/rk_hunsui.breft";
    *((config + 0x10)->subfile_filename + 6) = "./effect/poihana/rk_poihana.breff";
    *((config + 0x10)->subfile_filename + 7) = "./effect/poihana/rk_poihana.breft";
    *((config + 0x10)->subfile_filename + 8) = "./mare_a.brres";
    *((config + 0x10)->subfile_filename + 9) = "./mare_b.brres";
    *((config + 0x10)->subfile_filename + 10) = "./peachtreeGC.brres";
    *((config + 0x10)->subfile_filename + 11) = "./poihana.brres";
    *((config + 0x10)->subfile_filename + 12) = NULL;
    (config + 0x11)->szs_filename = "old_mario_gc.szs";
    (config + 0x11)->subfile_filename = (const char**)calloc(7, sizeof(const char*));
    *((config + 0x11)->subfile_filename + 0) = "./MashBalloonGC.brres";
    *((config + 0x11)->subfile_filename + 1) = "./brasd/pakkun_f/attack.brasd";
    *((config + 0x11)->subfile_filename + 2) = "./brasd/pakkun_f/wait.brasd";
    *((config + 0x11)->subfile_filename + 3) = "./mariotreeGC.brres";
    *((config + 0x11)->subfile_filename + 4) = "./pakkun_dokan.brres";
    *((config + 0x11)->subfile_filename + 5) = "./pakkun_f.brres";
    *((config + 0x11)->subfile_filename + 6) = NULL;
    (config + 0x12)->szs_filename = "old_waluigi_gc.szs";
    (config + 0x12)->subfile_filename = (const char**)calloc(9, sizeof(const char*));
    *((config + 0x12)->subfile_filename + 0) = "./WLarrowGC.brres";
    *((config + 0x12)->subfile_filename + 1) = "./WLdokanGC.brres";
    *((config + 0x12)->subfile_filename + 2) = "./WLdokanGC.kcl";
    *((config + 0x12)->subfile_filename + 3) = "./WLscreenGC.brres";
    *((config + 0x12)->subfile_filename + 4) = "./WLwallGC.brres";
    *((config + 0x12)->subfile_filename + 5) = "./WLwallGC.kcl";
    *((config + 0x12)->subfile_filename + 6) = "./effect/Flash_W/rk_flashWaluigi.breff";
    *((config + 0x12)->subfile_filename + 7) = "./effect/Flash_W/rk_flashWaluigi.breft";
    *((config + 0x12)->subfile_filename + 8) = NULL;
    (config + 0x13)->szs_filename = "old_donkey_gc.szs";
    (config + 0x13)->subfile_filename = (const char**)calloc(12, sizeof(const char*));
    *((config + 0x13)->subfile_filename + 0) = "./DKrockGC.brres";
    *((config + 0x13)->subfile_filename + 1) = "./DKturibashiGC.brres";
    *((config + 0x13)->subfile_filename + 2) = "./DonkyCannonGC.brres";
    *((config + 0x13)->subfile_filename + 3) = "./bird.brres";
    *((config + 0x13)->subfile_filename + 4) = "./brasd/DonkyCannonGC/DonkyCannonGC.brasd";
    *((config + 0x13)->subfile_filename + 5) = "./donkytree1GC.brres";
    *((config + 0x13)->subfile_filename + 6) = "./donkytree2GC.brres";
    *((config + 0x13)->subfile_filename + 7) = "./effect/DKrockGC/rk_DKrockGC.breff";
    *((config + 0x13)->subfile_filename + 8) = "./effect/DKrockGC/rk_DKrockGC.breft";
    *((config + 0x13)->subfile_filename + 9) = "./effect/DonkyCannonGC/rk_dkCannon.breff";
    *((config + 0x13)->subfile_filename + 10) = "./effect/DonkyCannonGC/rk_dkCannon.breft";
    *((config + 0x13)->subfile_filename + 11) = NULL;
    (config + 0x14)->szs_filename = "old_falls_ds.szs";
    (config + 0x14)->subfile_filename = (const char**)calloc(3, sizeof(const char*));
    *((config + 0x14)->subfile_filename + 0) = "./effect/Fall_Y/rk_yoshiFalls.breff";
    *((config + 0x14)->subfile_filename + 1) = "./effect/Fall_Y/rk_yoshiFalls.breft";
    *((config + 0x14)->subfile_filename + 2) = NULL;
    (config + 0x15)->szs_filename = "old_desert_ds.szs";
    (config + 0x15)->subfile_filename = (const char**)calloc(2, sizeof(const char*));
    *((config + 0x15)->subfile_filename + 0) = "./sunDS.brres";
    *((config + 0x15)->subfile_filename + 1) = NULL;
    (config + 0x16)->szs_filename = "old_garden_ds.szs";
    (config + 0x16)->subfile_filename = (const char**)calloc(2, sizeof(const char*));
    *((config + 0x16)->subfile_filename + 0) = "./gardentreeDS.brres";
    *((config + 0x16)->subfile_filename + 1) = NULL;
    (config + 0x17)->szs_filename = "old_town_ds.szs";
    (config + 0x17)->subfile_filename = (const char**)calloc(8, sizeof(const char*));
    *((config + 0x17)->subfile_filename + 0) = "./FlagA3.brres";
    *((config + 0x17)->subfile_filename + 1) = "./SentakuDS.brres";
    *((config + 0x17)->subfile_filename + 2) = "./TownBridgeDS.brres";
    *((config + 0x17)->subfile_filename + 3) = "./TownBridgeDS.kcl";
    *((config + 0x17)->subfile_filename + 4) = "./TownBridgeDS2.kcl";
    *((config + 0x17)->subfile_filename + 5) = "./TownBridgeDS3.kcl";
    *((config + 0x17)->subfile_filename + 6) = "./TownTreeDS.brres";
    *((config + 0x17)->subfile_filename + 7) = NULL;
    (config + 0x18)->szs_filename = "old_mario_sfc.szs";
    (config + 0x18)->subfile_filename = (const char**)calloc(2, sizeof(const char*));
    *((config + 0x18)->subfile_filename + 0) = "./oilSFC.brres";
    *((config + 0x18)->subfile_filename + 1) = NULL;
    (config + 0x19)->szs_filename = "old_obake_sfc.szs";
    (config + 0x19)->subfile_filename = (const char**)calloc(4, sizeof(const char*));
    *((config + 0x19)->subfile_filename + 0) = "./BGteresaSFC.brres";
    *((config + 0x19)->subfile_filename + 1) = "./b_teresa.brres";
    *((config + 0x19)->subfile_filename + 2) = "./obakeblockSFC.brres";
    *((config + 0x19)->subfile_filename + 3) = NULL;
    (config + 0x1A)->szs_filename = "old_mario_64.szs";
    (config + 0x1A)->subfile_filename = (const char**)calloc(6, sizeof(const char*));
    *((config + 0x1A)->subfile_filename + 0) = "./MarioGo64.brres";
    *((config + 0x1A)->subfile_filename + 1) = "./brasd/puchi_pakkun/puchi_pakkun.brasd";
    *((config + 0x1A)->subfile_filename + 2) = "./effect/Flash_M/rk_flash64mario.breff";
    *((config + 0x1A)->subfile_filename + 3) = "./effect/Flash_M/rk_flash64mario.breft";
    *((config + 0x1A)->subfile_filename + 4) = "./puchi_pakkun.brres";
    *((config + 0x1A)->subfile_filename + 5) = NULL;
    (config + 0x1B)->szs_filename = "old_sherbet_64.szs";
    (config + 0x1B)->subfile_filename = (const char**)calloc(15, sizeof(const char*));
    *((config + 0x1B)->subfile_filename + 0) = "./brasd/penguin_m/walk.brasd";
    *((config + 0x1B)->subfile_filename + 1) = "./brasd/penguin_s/dive.brasd";
    *((config + 0x1B)->subfile_filename + 2) = "./brasd/penguin_s/stand_up.brasd";
    *((config + 0x1B)->subfile_filename + 3) = "./brasd/penguin_s/walk.brasd";
    *((config + 0x1B)->subfile_filename + 4) = "./effect/penguin_l/rk_penguin_l.breff";
    *((config + 0x1B)->subfile_filename + 5) = "./effect/penguin_l/rk_penguin_l.breft";
    *((config + 0x1B)->subfile_filename + 6) = "./effect/penguin_m/rk_penguin_m.breff";
    *((config + 0x1B)->subfile_filename + 7) = "./effect/penguin_m/rk_penguin_m.breft";
    *((config + 0x1B)->subfile_filename + 8) = "./effect/penguin_s/rk_penguin_s.breff";
    *((config + 0x1B)->subfile_filename + 9) = "./effect/penguin_s/rk_penguin_s.breft";
    *((config + 0x1B)->subfile_filename + 10) = "./ice.brres";
    *((config + 0x1B)->subfile_filename + 11) = "./penguin_l.brres";
    *((config + 0x1B)->subfile_filename + 12) = "./penguin_m.brres";
    *((config + 0x1B)->subfile_filename + 13) = "./penguin_s.brres";
    *((config + 0x1B)->subfile_filename + 14) = NULL;
    (config + 0x1C)->szs_filename = "old_koopa_64.szs";
    (config + 0x1C)->subfile_filename = (const char**)calloc(6, sizeof(const char*));
    *((config + 0x1C)->subfile_filename + 0) = "./KoopaFigure64.brres";
    *((config + 0x1C)->subfile_filename + 1) = "./effect/FlamePole_v_big/rk_flamePoleL.breff";
    *((config + 0x1C)->subfile_filename + 2) = "./effect/FlamePole_v_big/rk_flamePoleL.breft";
    *((config + 0x1C)->subfile_filename + 3) = "./effect/KoopaFigure64/rk_koopaFire.breff";
    *((config + 0x1C)->subfile_filename + 4) = "./effect/KoopaFigure64/rk_koopaFire.breft";
    *((config + 0x1C)->subfile_filename + 5) = NULL;
    (config + 0x1D)->szs_filename = "old_donkey_64.szs";
    (config + 0x1D)->subfile_filename = (const char**)calloc(8, sizeof(const char*));
    *((config + 0x1D)->subfile_filename + 0) = "./DKship64.brres";
    *((config + 0x1D)->subfile_filename + 1) = "./DKtreeA64.brres";
    *((config + 0x1D)->subfile_filename + 2) = "./DKtreeB64.brres";
    *((config + 0x1D)->subfile_filename + 3) = "./effect/DKfalls/rk_64DKfalls.breff";
    *((config + 0x1D)->subfile_filename + 4) = "./effect/DKfalls/rk_64DKfalls.breft";
    *((config + 0x1D)->subfile_filename + 5) = "./effect/DKship64/rk_DKship64.breff";
    *((config + 0x1D)->subfile_filename + 6) = "./effect/DKship64/rk_DKship64.breft";
    *((config + 0x1D)->subfile_filename + 7) = NULL;
    (config + 0x1E)->szs_filename = "old_koopa_gba.szs";
    (config + 0x1E)->subfile_filename = (const char**)calloc(4, sizeof(const char*));
    *((config + 0x1E)->subfile_filename + 0) = "./boble.brres";
    *((config + 0x1E)->subfile_filename + 1) = "./effect/boble/rk_boble.breff";
    *((config + 0x1E)->subfile_filename + 2) = "./effect/boble/rk_boble.breft";
    *((config + 0x1E)->subfile_filename + 3) = NULL;
    (config + 0x1F)->szs_filename = "old_heyho_gba.szs";
    (config + 0x1F)->subfile_filename = (const char**)calloc(13, sizeof(const char*));
    *((config + 0x1F)->subfile_filename + 0) = "./HeyhoBallGBA.brres";
    *((config + 0x1F)->subfile_filename + 1) = "./HeyhoShipGBA.brres";
    *((config + 0x1F)->subfile_filename + 2) = "./HeyhoTreeGBA.brres";
    *((config + 0x1F)->subfile_filename + 3) = "./brasd/crab/wait.brasd";
    *((config + 0x1F)->subfile_filename + 4) = "./brasd/crab/walk_l.brasd";
    *((config + 0x1F)->subfile_filename + 5) = "./brasd/crab/walk_r.brasd";
    *((config + 0x1F)->subfile_filename + 6) = "./crab.brres";
    *((config + 0x1F)->subfile_filename + 7) = "./effect/HeyhoBallGBA/rk_HeyhoBallGBA.breff";
    *((config + 0x1F)->subfile_filename + 8) = "./effect/HeyhoBallGBA/rk_HeyhoBallGBA.breft";
    *((config + 0x1F)->subfile_filename + 9) = "./effect/crab/rk_crab.breff";
    *((config + 0x1F)->subfile_filename + 10) = "./effect/crab/rk_crab.breft";
    *((config + 0x1F)->subfile_filename + 11) = "./r_parasol.brres";
    *((config + 0x1F)->subfile_filename + 12) = NULL;
    (config + 0x20)->szs_filename = "venice_battle.szs";
    (config + 0x20)->subfile_filename = (const char**)calloc(13, sizeof(const char*));
    *((config + 0x20)->subfile_filename + 0) = "./effect/truckChimSmkW/rk_truckChimSmkW.breff";
    *((config + 0x20)->subfile_filename + 1) = "./effect/truckChimSmkW/rk_truckChimSmkW.breft";
    *((config + 0x20)->subfile_filename + 2) = "./effect/venice_hasi/rk_venice.breff";
    *((config + 0x20)->subfile_filename + 3) = "./effect/venice_hasi/rk_venice.breft";
    *((config + 0x20)->subfile_filename + 4) = "./truckChimSmkW.brres";
    *((config + 0x20)->subfile_filename + 5) = "./venice_hasi.brres";
    *((config + 0x20)->subfile_filename + 6) = "./venice_hasi.kcl";
    *((config + 0x20)->subfile_filename + 7) = "./venice_hasi_b.kcl";
    *((config + 0x20)->subfile_filename + 8) = "./venice_hasi_wall.kcl";
    *((config + 0x20)->subfile_filename + 9) = "./venice_nami.brres";
    *((config + 0x20)->subfile_filename + 10) = "./venice_saku.brres";
    *((config + 0x20)->subfile_filename + 11) = "./venice_saku.kcl";
    *((config + 0x20)->subfile_filename + 12) = NULL;
    (config + 0x21)->szs_filename = "block_battle.szs";
    (config + 0x21)->subfile_filename = (const char**)calloc(27, sizeof(const char*));
    *((config + 0x21)->subfile_filename + 0) = "./MiiStatueD2.brres";
    *((config + 0x21)->subfile_filename + 1) = "./MiiStatueL2.brres";
    *((config + 0x21)->subfile_filename + 2) = "./MiiStatueM1.brres";
    *((config + 0x21)->subfile_filename + 3) = "./MiiStatueP1.brres";
    *((config + 0x21)->subfile_filename + 4) = "./bblock1.brres";
    *((config + 0x21)->subfile_filename + 5) = "./bblock1.kcl";
    *((config + 0x21)->subfile_filename + 6) = "./bblock1b.kcl";
    *((config + 0x21)->subfile_filename + 7) = "./bblock2.kcl";
    *((config + 0x21)->subfile_filename + 8) = "./bblock2b.kcl";
    *((config + 0x21)->subfile_filename + 9) = "./bblock3.kcl";
    *((config + 0x21)->subfile_filename + 10) = "./bblock3b.kcl";
    *((config + 0x21)->subfile_filename + 11) = "./bblock4.kcl";
    *((config + 0x21)->subfile_filename + 12) = "./bblock4b.kcl";
    *((config + 0x21)->subfile_filename + 13) = "./bblock5.kcl";
    *((config + 0x21)->subfile_filename + 14) = "./bblock5b.kcl";
    *((config + 0x21)->subfile_filename + 15) = "./bblock6.kcl";
    *((config + 0x21)->subfile_filename + 16) = "./bblock6b.kcl";
    *((config + 0x21)->subfile_filename + 17) = "./bblock7.kcl";
    *((config + 0x21)->subfile_filename + 18) = "./bblock7b.kcl";
    *((config + 0x21)->subfile_filename + 19) = "./bblock8.kcl";
    *((config + 0x21)->subfile_filename + 20) = "./bblock8b.kcl";
    *((config + 0x21)->subfile_filename + 21) = "./bblock9.kcl";
    *((config + 0x21)->subfile_filename + 22) = "./bblock9b.kcl";
    *((config + 0x21)->subfile_filename + 23) = "./coin.brres";
    *((config + 0x21)->subfile_filename + 24) = "./effect/coin/rk_coin.breff";
    *((config + 0x21)->subfile_filename + 25) = "./effect/coin/rk_coin.breft";
    *((config + 0x21)->subfile_filename + 26) = NULL;
    (config + 0x22)->szs_filename = "casino_battle.szs";
    (config + 0x22)->subfile_filename = (const char**)calloc(5, sizeof(const char*));
    *((config + 0x22)->subfile_filename + 0) = "./Twanwan.brres";
    *((config + 0x22)->subfile_filename + 1) = "./casino_roulette.brres";
    *((config + 0x22)->subfile_filename + 2) = "./casino_roulette.kcl";
    *((config + 0x22)->subfile_filename + 3) = "./spot.brres";
    *((config + 0x22)->subfile_filename + 4) = NULL;
    (config + 0x23)->szs_filename = "skate_battle.szs";
    (config + 0x23)->subfile_filename = (const char**)calloc(4, sizeof(const char*));
    *((config + 0x23)->subfile_filename + 0) = "./effect/Flash_S/rk_flashSkate.breff";
    *((config + 0x23)->subfile_filename + 1) = "./effect/Flash_S/rk_flashSkate.breft";
    *((config + 0x23)->subfile_filename + 2) = "./fks_screen_wii.brres";
    *((config + 0x23)->subfile_filename + 3) = NULL;
    (config + 0x24)->szs_filename = "sand_battle.szs";
    (config + 0x24)->subfile_filename = (const char**)calloc(4, sizeof(const char*));
    *((config + 0x24)->subfile_filename + 0) = "./effect/quicksand/rk_qsandDossun.breff";
    *((config + 0x24)->subfile_filename + 1) = "./effect/quicksand/rk_qsandDossun.breft";
    *((config + 0x24)->subfile_filename + 2) = "./quicksand.brres";
    *((config + 0x24)->subfile_filename + 3) = NULL;
    (config + 0x25)->szs_filename = "";//empty
    (config + 0x25)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x25)->subfile_filename + 0) = NULL;
    (config + 0x26)->szs_filename = "";//empty
    (config + 0x26)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x26)->subfile_filename + 0) = NULL;
    (config + 0x27)->szs_filename = "";//empty
    (config + 0x27)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x27)->subfile_filename + 0) = NULL;
    (config + 0x28)->szs_filename = "";//empty
    (config + 0x28)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x28)->subfile_filename + 0) = NULL;
    (config + 0x29)->szs_filename = "old_matenro_64.szs";
    (config + 0x29)->subfile_filename = (const char**)calloc(3, sizeof(const char*));
    *((config + 0x29)->subfile_filename + 0) = "./Crescent64.brres";
    *((config + 0x29)->subfile_filename + 1) = "./Spot64.brres";
    *((config + 0x29)->subfile_filename + 2) = NULL;
    (config + 0x2A)->szs_filename = "";//empty
    (config + 0x2A)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2A)->subfile_filename + 0) = NULL;
    (config + 0x2B)->szs_filename = "";//empty
    (config + 0x2B)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2B)->subfile_filename + 0) = NULL;
    (config + 0x2C)->szs_filename = "";//empty
    (config + 0x2C)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2C)->subfile_filename + 0) = NULL;
    (config + 0x2D)->szs_filename = "";//empty
    (config + 0x2D)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2D)->subfile_filename + 0) = NULL;
    (config + 0x2E)->szs_filename = "";//empty
    (config + 0x2E)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2E)->subfile_filename + 0) = NULL;
    (config + 0x2F)->szs_filename = "";//empty
    (config + 0x2F)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x2F)->subfile_filename + 0) = NULL;
    (config + 0x30)->szs_filename = "";//empty
    (config + 0x30)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x30)->subfile_filename + 0) = NULL;
    (config + 0x31)->szs_filename = "";//empty
    (config + 0x31)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x31)->subfile_filename + 0) = NULL;
    (config + 0x32)->szs_filename = "";//empty
    (config + 0x32)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x32)->subfile_filename + 0) = NULL;
    (config + 0x33)->szs_filename = "";//empty
    (config + 0x33)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x33)->subfile_filename + 0) = NULL;
    (config + 0x34)->szs_filename = "";//empty
    (config + 0x34)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x34)->subfile_filename + 0) = NULL;
    (config + 0x35)->szs_filename = "";//empty
    (config + 0x35)->subfile_filename = (const char**)calloc(1, sizeof(const char*));
    *((config + 0x35)->subfile_filename + 0) = NULL;
    (config + 0x36)->szs_filename = "ring_mission.szs";
    (config + 0x36)->subfile_filename = (const char**)calloc(14, sizeof(const char*));
    *((config + 0x36)->subfile_filename + 0) = "./RM_ring1.brres";
    *((config + 0x36)->subfile_filename + 1) = "./RM_ring1.kcl";
    *((config + 0x36)->subfile_filename + 2) = "./RM_ring1b.kcl";
    *((config + 0x36)->subfile_filename + 3) = "./RM_ring2.kcl";
    *((config + 0x36)->subfile_filename + 4) = "./RM_ring2b.kcl";
    *((config + 0x36)->subfile_filename + 5) = "./RM_ring2c.kcl";
    *((config + 0x36)->subfile_filename + 6) = "./RM_ring3.kcl";
    *((config + 0x36)->subfile_filename + 7) = "./RM_ring3b.kcl";
    *((config + 0x36)->subfile_filename + 8) = "./RM_ring3c.kcl";
    *((config + 0x36)->subfile_filename + 9) = "./begoman_spike.brres";
    *((config + 0x36)->subfile_filename + 10) = "./effect/begoman_spike/rk_begoman_spike.breff";
    *((config + 0x36)->subfile_filename + 11) = "./effect/begoman_spike/rk_begoman_spike.breft";
    *((config + 0x36)->subfile_filename + 12) = "./ring_mission_b.kcl";
    *((config + 0x36)->subfile_filename + 13) = NULL;
    (config + 0x37)->szs_filename = "winningrun_demo.szs";
    (config + 0x37)->subfile_filename = (const char**)calloc(13, sizeof(const char*));
    *((config + 0x37)->subfile_filename + 0) = "./DemoJugemu.brres";
    *((config + 0x37)->subfile_filename + 1) = "./effect/DemoEf/rk_demo.breff";
    *((config + 0x37)->subfile_filename + 2) = "./effect/DemoEf/rk_demo.breft";
    *((config + 0x37)->subfile_filename + 3) = "./effect/DemoJugemu/rk_DemoJugemu.breff";
    *((config + 0x37)->subfile_filename + 4) = "./effect/DemoJugemu/rk_DemoJugemu.breft";
    *((config + 0x37)->subfile_filename + 5) = "./effect/Kamifubuki/rk_kamifubuki.breff";
    *((config + 0x37)->subfile_filename + 6) = "./effect/Kamifubuki/rk_kamifubuki.breft";
    *((config + 0x37)->subfile_filename + 7) = "./group_enemy_a_demo.brres";
    *((config + 0x37)->subfile_filename + 8) = "./group_monte_a_demo.brres";
    *((config + 0x37)->subfile_filename + 9) = "./posteffect/posteffect.blobj";
    *((config + 0x37)->subfile_filename + 10) = "./posteffect/posteffect.bltex";
    *((config + 0x37)->subfile_filename + 11) = "./posteffect/posteffect2.blobj";
    *((config + 0x37)->subfile_filename + 12) = NULL;
    (config + 0x38)->szs_filename = "loser_demo.szs";
    (config + 0x38)->subfile_filename = (const char**)calloc(3, sizeof(const char*));
    *((config + 0x38)->subfile_filename + 0) = "./DemoCol.brres";
    *((config + 0x38)->subfile_filename + 1) = "./DemoCol.kcl";
    *((config + 0x38)->subfile_filename + 2) = NULL;
    (config + 0x39)->szs_filename = NULL;
    return config;
}

bool dumpFromOneConfig(szs_subfile_dump_config* oneConfig){
    char tmpPath[128];
    if(oneConfig->szs_filename != NULL && oneConfig->subfile_filename[0] == NULL)return true;
    //skip for empty slot
    printf("Reading %s...\n", oneConfig->szs_filename);
    sprintf(tmpPath, "/Race/Course/%s", oneConfig->szs_filename);
    unsigned int srcSzsSize;
    unsigned char *srcSzs = loadFileToMemory(tmpPath, &srcSzsSize);
    if(!srcSzs){
        printf("Error: cannot read %s!\n", oneConfig->szs_filename);
        return false;
    }
    unsigned int u8ArchiveRawSize;
    unsigned char *u8ArchiveRaw = decompSzsMalloc(srcSzs, &u8ArchiveRawSize);
    free(srcSzs);
    u8_archive u8a;
    u8_archive_ro_init(&u8a, u8ArchiveRaw);
    unsigned int subFileIndex = 0;
    while(1){
        if(oneConfig->subfile_filename[subFileIndex] == NULL)break;
        unsigned char *subFile = NULL;
        unsigned int subFileSize = 0;
        subFile = u8_archive_ro_get_file(&u8a, oneConfig->subfile_filename[subFileIndex], &subFileSize);
        printf("Dumping %s...\n", oneConfig->subfile_filename[subFileIndex]);
        if(!subFile){
            free(u8ArchiveRaw);
            printf("Error: cannot find %s!\n", oneConfig->subfile_filename[subFileIndex]);
            return false;
        }
        sprintf(tmpPath, "sd:/rk_dumper/auto-add/%s", (oneConfig->subfile_filename[subFileIndex]) + 2);
        if(!bytesToFile(subFile, subFileSize, tmpPath)){
            printf("Error: cannot write %s!\n", oneConfig->subfile_filename[subFileIndex]);
            free(u8ArchiveRaw);
            return false;
        }
        subFileIndex++;
    }
    free(u8ArchiveRaw);
    return true;
}

bool dumpByConfig(szs_subfile_dump_config* src){
    szs_subfile_dump_config *config = src;
    while(1){
        if(config->szs_filename == NULL)return true;
        if(!dumpFromOneConfig(config))return false;
        config++;
    }
    return true;
}